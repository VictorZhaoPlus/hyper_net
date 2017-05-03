#include "NetEngine.h"
#include "Marco.h"
#include "Connection.h"
#include "ConfigMgr.h"
#include "IKernel.h"
#include "tools.h"
#include "kernel.h"
#include <signal.h>
#include "NetWorker.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>


s32 SetNonBlocking(const s32 fd) {
	if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK) == -1) {
		printf("setnonblocking error %s\n", strerror(errno));
		return -1;
	}
	return 0;
}
	
s32 SetNonNegal(const s32 fd) {
	long val = 1l;
	return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&val, sizeof(val));
}

s32 SetSendBuf(const s32 fd, const s32 size) {
	return setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
}

s32 SetReuse(const s32 fd) {
	s32 val = 1;
	return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&val, sizeof(val));
}

NetEngine::NetEngine() {
    //ctor
}

NetEngine::~NetEngine() {
    //dtor
}

bool NetEngine::Ready() {
	signal(SIGPIPE, SIG_IGN);
    return true;
}

bool NetEngine::Initialize() {
	_acFd = epoll_create(ConfigMgr::Instance()->GetNetSupportSize());
	_acSize = 0;
	
	for (s32 i = 0; i < ConfigMgr::Instance()->GetNetThreadCount(); ++i) {
		NetWorker * worker = NEW NetWorker;
		if (!worker->Start())
			return false;
		
		_workers.push_back(worker);
	}
	
    return true;
}

s32 NetEngine::Loop(s64 overtime) {
    s64 tick = tools::GetTimeMillisecond();
	ProcessAC(ConfigMgr::Instance()->GetNetFrameWaitTick());
	for (auto * worker : _workers)
		worker->Process(overtime / _workers.size());

	return tools::GetTimeMillisecond() - tick;
}

void NetEngine::Destroy() {
	for (auto * worker : _workers) {
		worker->Terminate();
		DEL worker;
	}
	_workers.clear();
	close(_acFd);
}

bool NetEngine::Listen(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, core::ISessionFactory * factory) {
	s32 fd;
	struct sockaddr_in addr;

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		KERNEL_LOG("listen[%s:%d] socket create failed, error %d\n", ip, port, errno);
		return false;
	}

	if (0 != SetNonBlocking(fd) || 0 != SetReuse(fd)) {
		KERNEL_LOG("listen[%s:%d] set opt failed, error %d\n", ip, port, errno);
		return false;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &addr.sin_addr);

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		KERNEL_LOG("listen[%s:%d] bind failed, error %d\n", ip, port, errno);
		close(fd);
		return false;
	}

	if (listen(fd, 128) == -1) {
		KERNEL_LOG("listen[%s:%d] listen failed, error %d\n", ip, port, errno);
		close(fd);
		return false;
	}

	struct epoll_event ev;
	ev.data.ptr = NEW ACDealer({ACDealerType::ACDT_ACCEPT, fd, sendSize, recvSize, factory});
	ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
	if (epoll_ctl(_acFd, EPOLL_CTL_ADD, fd, &ev) != 0) {
		KERNEL_LOG("listen[%s:%d] epoll add failed, error %d\n", ip, port, errno);
		close(fd);
		return false;
	}

	KERNEL_LOG("listen[%s:%d] socket create ok\n", ip, port);
	++_acSize;
	return true;
}

bool NetEngine::Connect(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, core::ISession * session) {
	s32 fd;
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
		KERNEL_LOG("inet_pton error %s\n", ip);
		return false;
	}

	if (-1 == (fd = socket(AF_INET, SOCK_STREAM, 0))) {
		KERNEL_LOG("connect[%s:%d] create failed, error %d\n", ip, port, errno);
		return false;
	}

	if (0 != SetNonBlocking(fd) || 0 != SetSendBuf(fd, 0) || 0 != SetNonNegal(fd)) {
		KERNEL_LOG("connect[%s:%d] set opt failed, error %d\n", ip, port, errno);
		close(fd);
		return false;
	}

	s32 ret = connect(fd, (struct sockaddr *) &addr, sizeof(addr));
	if (ret < 0 && errno != EINPROGRESS) {
		KERNEL_LOG("connect[%s:%d] connect failed, error %d\n", ip, port, errno);
		close(fd);
		return false;
	}
	
	struct epoll_event ev;
	ev.data.ptr = NEW ACDealer({ACDealerType::ACDT_CONNECT, fd, sendSize, recvSize, session});
	ev.events = EPOLLOUT | EPOLLET | EPOLLERR | EPOLLHUP;
	if (epoll_ctl(_acFd, EPOLL_CTL_ADD, fd, &ev) != 0) {
		KERNEL_LOG("connect[%s:%d] epoll add failed, error %d\n", ip, port, errno);
		close(fd);
		return false;
	}
	++_acSize;
	return true;
}

void NetEngine::ProcessAC(s64 waitTime) {
	if (_acSize <= 0)
		return;
	epoll_event events[_acSize];
	memset(events, 0, sizeof(events));
	s32 retCount = epoll_wait(_acFd, events, _acSize, waitTime);
	if (retCount == -1) {
		if (errno != EINTR) {
			KERNEL_LOG("epoll_wait error %d\n", errno);
		}
		return;
	}

	if (retCount == 0)
		return;

	for (s32 i = 0; i < retCount; i++) {
		ACDealer * dealer = (ACDealer*)events[i].data.ptr;
		switch (dealer->type) {
		case ACDealerType::ACDT_ACCEPT: {
			if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
				OASSERT(0, "wtf");
			} 
			else if (events[i].events & EPOLLIN) {
				s32 fd;
				struct sockaddr_in addr;
				socklen_t len = sizeof(addr);
				memset(&addr, 0, sizeof(addr));
				
				s32 i = 0;
				while (i++ < 30 && (fd = accept(dealer->fd, (struct sockaddr *)&addr, &len)) >= 0) {
					if (0 == SetNonBlocking(fd) && 0 == SetSendBuf(fd, 0) && 0 == SetNonNegal(fd))
						OnAccept(dealer, fd);
					else
						close(fd);
				}
			}
			break;
		}
		case ACDealerType::ACDT_CONNECT: {
			epoll_ctl(_acFd, EPOLL_CTL_DEL, dealer->fd, &events[i]);
			if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
				OnConnect(dealer, false);
				close(dealer->fd);
			}
			else if (events[i].events & EPOLLOUT)
				OnConnect(dealer, true);
			DEL dealer;
			--_acSize;
			break;
		}
		default:
			OASSERT(0, "wtf");
			break;
		}
	}
}

void NetEngine::OnAccept(ACDealer * accepter, s32 fd) {
	core::ISessionFactory * factory = (core::ISessionFactory *)accepter->context;
	OASSERT(factory, "wtf");

	core::ISession * session = factory->Create();
	if (session) {
		OASSERT(session != nullptr, "wtf");
		session->SetFactory(factory);

		Connection * connection = Connection::Create(fd, accepter->sendSize, accepter->recvSize);
		OASSERT(connection != nullptr, "wtf");
		connection->SetSession(session);

		sockaddr_in remote;
		socklen_t len = sizeof(remote);
		getpeername(fd, (sockaddr*)&remote, &len);
		connection->SetRemoteIp(inet_ntoa(remote.sin_addr));
		connection->SetRemotePort(ntohs(remote.sin_port));
		
		if (!AddToWorker(connection)) {
			session->OnRelease();
			connection->OnRelease();
			close(fd);
			return;
		}

		session->OnConnected(Kernel::Instance());
	}
	else
		close(fd);
}

void NetEngine::OnConnect(ACDealer * connecter, bool connectSuccess) {
	core::ISession * session = (core::ISession *)connecter->context;
	OASSERT(session, "wtf");
	
	if (connectSuccess) {
		Connection * connection = Connection::Create(connecter->fd, connecter->sendSize, connecter->recvSize);
		OASSERT(connection != nullptr, "wtf");
		connection->SetSession(session);

		sockaddr_in remote;
		socklen_t len = sizeof(remote);
		getpeername(connecter->fd, (sockaddr*)&remote, &len);
		connection->SetRemoteIp(inet_ntoa(remote.sin_addr));
		connection->SetRemotePort(ntohs(remote.sin_port));
		
		if (!AddToWorker(connection)) {
			session->SetPipe(nullptr);
			connection->OnRelease();
			close(connecter->fd);
			session->OnConnectFailed(Kernel::Instance());
			return;
		}

		session->OnConnected(Kernel::Instance());
	}
	else
		session->OnConnectFailed(Kernel::Instance());
}

bool NetEngine::AddToWorker(Connection * connection) {
	NetWorker * sel = nullptr;
	for (auto * worker : _workers) {
		if (sel == nullptr || sel->Count() > worker->Count())
			sel = worker;
	}
	
	OASSERT(sel, "wtf");
	connection->SetWorker(sel);
	return sel->Add(connection);
}
