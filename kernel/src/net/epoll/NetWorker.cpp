#include "NetWorker.h"
#include "Connection.h"
#include "kernel.h"
#include "ConfigMgr.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include "tools.h"

#define QUEUE_SIZE 8196

NetWorker::NetWorker() 
	: _fd(-1)
	, _count(0)
	, _runQueue(QUEUE_SIZE)
	, _terminate(false) {
	
	_waitTime = ConfigMgr::Instance()->GetNetFrameWaitTick();
}
	
bool NetWorker::Start() {
	_fd = epoll_create(ConfigMgr::Instance()->GetNetSupportSize());
	_thread = std::thread(&NetWorker::ThreadProc, this);
	return true;
}

void NetWorker::Terminate() {
	_terminate = true;
	_thread.join();
	close(_fd);
}

void NetWorker::Process(s64 overtime) {
	s64 tick = tools::GetTimeMillisecond();
	
	bool swaped = false;
	while (true) {
		if (!swaped && _completeQueue.empty()) {
			swaped = true;
			std::unique_lock<spin_mutex> guard(_lock);
			_completeQueue.swap(_waitCompleteQueue);
		}
		
		if (_completeQueue.empty())
			break;
		
		NetEvent evt = *_completeQueue.begin();
		_completeQueue.pop_front();
		
		switch (evt.type) {
		case NWET_RECV: evt.connection->OnRecv(); break;
		case NWET_ERROR: evt.connection->Close(); break;
		case NWET_DONE: evt.connection->OnDone(); break;
		}
		
		if (tools::GetTimeMillisecond() - tick >= overtime)
			break;
	}
}

void NetWorker::ThreadProc() {
	while (!_terminate) {
		NetEvent evt;
		while (_runQueue.Read(evt)) {
			switch (evt.type) {
			case NWET_SEND: evt.connection->ThreadSend(false); break;
			case NWET_CLOSING: evt.connection->ThreadClose(false); break;
			}	
		}
		
		ProcessRS(_waitTime);
	}
}

void NetWorker::ProcessRS(s64 waitTime) {
	if (_count <= 0) {
		CSLEEP(1);
		return;
	}
	
	epoll_event events[_count];
	memset(events, 0, sizeof(events));
	s32 retCount = epoll_wait(_fd, events, _count, waitTime);
	if (retCount == -1) {
		return;
	}

	if (retCount == 0)
		return;

	for (s32 i = 0; i < retCount; i++) {
		Connection * connection = (Connection*)events[i].data.ptr;
		if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
			s32 error = 0;
			socklen_t len = sizeof(error);
			getsockopt(connection->GetFd(), SOL_SOCKET, SO_ERROR, (char*)&error, &len);
			KERNEL_LOG("connection %s:%d error:%d", connection->GetRemoteIp(), connection->GetRemotePort(), error);
			connection->ThreadClose(true);
			continue;
		}
		
		if (events[i].events & EPOLLIN) {
			if (!connection->ThreadRecv())
				continue;
		}
		
		if (events[i].events & EPOLLOUT)
			connection->ThreadSend(true);
	}
}

bool NetWorker::Add(Connection * connection) {
	struct epoll_event ev;
	ev.data.ptr = connection;
	ev.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLERR | EPOLLHUP;
	
	if (epoll_ctl(_fd, EPOLL_CTL_ADD, connection->GetFd(), &ev) != 0) {
		KERNEL_LOG("net work add connection error %d\n", errno);
		return false;
	}
	++_count;
	connection->SetWorker(this);
	return true;
}

void NetWorker::Remove(Connection * connection) { //in thread
	struct epoll_event ev;
	ev.data.ptr = connection;
	ev.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLERR | EPOLLHUP;
	
	epoll_ctl(_fd, EPOLL_CTL_DEL, connection->GetFd(), &ev);
}
