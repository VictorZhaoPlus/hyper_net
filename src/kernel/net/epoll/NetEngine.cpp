#include "NetEngine.h"
#include "Marco.h"
#include "Connection.h"
#include "ConfigMgr.h"
#include "IKernel.h"
#include "tools.h"
#include "kernel.h"
#include <signal.h>

enum netio_event_type {
	NETIO_EVENT_WAITING_SEND,
	NETIO_EVENT_DATA_RECVED,
	NETIO_EVENT_CONNECT_BREAK,
};

enum thread_event_type {
	TET_NEW,
	TET_SEND,
};

NetEngine::Thread * NetEngine::s_threads = nullptr;

NetEngine::NetEngine()
	: _looper(nullptr) {
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
	SetMaxOpenFile(ConfigMgr::Instance()->GetMaxOpenFile());
	SetStackSize(ConfigMgr::Instance()->GetStackSize());
    s32 size = ConfigMgr::Instance()->GetNetSupportSize();
	_looper = MallocLooper(size, OnAccept, OnConnect, nullptr, nullptr);

	s_threads = NEW Thread[ConfigMgr::Instance()->GetNetThreadCount()];
	for (s32 i = 0; i < ConfigMgr::Instance()->GetNetThreadCount(); ++i) {
		s_threads[i].looper = MallocLooper(size, nullptr, nullptr, OnRecv, OnSend);
		s_threads[i].terminated = false;
		s_threads[i].handler = std::thread(NetEngine::ThreadLoop, &s_threads[i]);
	}

    return true;
}

s32 NetEngine::Loop(s64 overtime) {
    s64 tick = tools::GetTimeMillisecond();

	DispatchLooper(_looper, ConfigMgr::Instance()->GetNetFrameWaitTick());

	for (s32 i = 0; i < ConfigMgr::Instance()->GetNetThreadCount(); ++i) {
		ProcessingOneThread(&s_threads[i], overtime / ConfigMgr::Instance()->GetNetThreadCount());
	}

	return tools::GetTimeMillisecond() - tick;
}

void NetEngine::Destroy() {
	if (s_threads) {
		for (s32 i = 0; i < ConfigMgr::Instance()->GetNetThreadCount(); ++i) {
			s_threads[i].terminated = true;
			s_threads[i].handler.join();
			FreeLooper(s_threads[i].looper);
		}
		DEL[] s_threads;
	}

	if (_looper) {
		FreeLooper(_looper);
		_looper = nullptr;
	}
}

void NetEngine::ProcessingOneThread(Thread * thread, s64 overtime) {
	s64 tick = tools::GetTimeMillisecond();
	NetEvent ev;
	while (thread->events.Read(ev)) {
		core::ISession * session = (core::ISession *)ev.base->context;
		Connection * connection = (Connection*)session->GetPipe();

		switch (ev.event) {
			case netio_event_type::NETIO_EVENT_DATA_RECVED: connection->OnRecv(); break;
			case netio_event_type::NETIO_EVENT_WAITING_SEND: {
				if (!connection->OnSendBack()) {
					DecThreadConnectionCount(connection->GetThreadId());
					session->SetPipe(nullptr);
					session->OnDisconnected(Kernel::Instance());
					connection->OnRelease();
					FreeBase(ev.base);
				}
				break;
			}
			case netio_event_type::NETIO_EVENT_CONNECT_BREAK: {
				if (!connection->CheckPipeBroke()) {
					DecThreadConnectionCount(connection->GetThreadId());
					session->SetPipe(nullptr);
					session->OnDisconnected(Kernel::Instance());
					connection->OnRelease();
					FreeBase(ev.base);
				}
				break;
			}
		}

		s64 use = tools::GetTimeMillisecond() - tick;
		if (use > overtime)
			break;
	}
}

bool NetEngine::Listen(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, core::ISessionFactory * factory) {
	OASSERT(factory, "wtf");
	NetBase * acceptor = MallocAcceptor(ip, port, 128);
	acceptor->context = factory;
	acceptor->maxRecvSize = recvSize;
	acceptor->maxSendSize = sendSize;

	return 0 == BindLooper(_looper, acceptor);
}

bool NetEngine::Connect(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, core::ISession * session) {
	OASSERT(session, "wtf");
	NetBase * connecter = MallocConnector(ip, port);
	if (connecter) {
		connecter->maxRecvSize = recvSize;
		connecter->maxSendSize = sendSize;
		connecter->context = session;
		if (0 != BindLooper(_looper, connecter)) {
			FreeBase(connecter);
			session->OnConnectFailed(Kernel::Instance());
		}
	}
	else
		session->OnConnectFailed(Kernel::Instance());
    return true;
}

s32 NetEngine::OnAccept(NetBase * accepter, struct NetBase * base) {
	core::ISessionFactory * factory = (core::ISessionFactory *)accepter->context;
	OASSERT(factory, "wtf");

	core::ISession * session = factory->Create();
	OASSERT(session != nullptr, "wtf");
	session->SetFactory(factory);
	base->context = session;

	Connection * connection = Connection::Create(base, base->fd, accepter->maxSendSize, accepter->maxRecvSize);
	OASSERT(connection != nullptr, "wtf");
	connection->SetSession(session);

	sockaddr_in remote;
	socklen_t len = sizeof(remote);
	getpeername(base->fd, (sockaddr*)&remote, &len);
	connection->SetRemoteIp(inet_ntoa(remote.sin_addr));
	connection->SetRemotePort(ntohs(remote.sin_port));

	s32 threadId = SelectThread();
	connection->SetThreadId(threadId);
	OASSERT(threadId < ConfigMgr::Instance()->GetNetThreadCount(), "wtf");
	s_threads[threadId].threadEvents.Push({ base, thread_event_type::TET_NEW });
	++s_threads[threadId].count;

	session->OnConnected(Kernel::Instance());
	return 0;
}

s32 NetEngine::OnConnect(NetBase * connecter, const int code) {
	core::ISession * session = (core::ISession *)connecter->context;
	if (0 == code) {
		Connection * connection = Connection::Create(connecter, connecter->fd, connecter->maxSendSize, connecter->maxRecvSize);
		OASSERT(connection != nullptr, "wtf");
		connection->SetSession(session);

		sockaddr_in remote;
		socklen_t len = sizeof(remote);
		getpeername(connecter->fd, (sockaddr*)&remote, &len);
		connection->SetRemoteIp(inet_ntoa(remote.sin_addr));
		connection->SetRemotePort(ntohs(remote.sin_port));

		s32 threadId = SelectThread();
		connection->SetThreadId(threadId);
		OASSERT(threadId < ConfigMgr::Instance()->GetNetThreadCount(), "wtf");
		s_threads[threadId].threadEvents.Push({ connecter, thread_event_type::TET_NEW });
		++s_threads[threadId].count;

		session->OnConnected(Kernel::Instance());
	}
	else
		session->OnConnectFailed(Kernel::Instance());
	return 0;
}

s32 NetEngine::OnRecv(NetBase * base, const s32 code, const char * buff, const s32 size) {
	core::ISession * session = (core::ISession *)base->context;
	Connection * connection = (Connection *)session->GetPipe();
	s32 threadId = connection->GetThreadId();
	if (code != 0 || size == 0 || !connection->Recv(buff, size)) {
		connection->SetPipeBroken();

		//close
		NetEvent evt;
		evt.base = base;
		if (connection->IsSending()) {
			evt.event = netio_event_type::NETIO_EVENT_WAITING_SEND;
			AddEvent(threadId, evt);
		}
		evt.event = netio_event_type::NETIO_EVENT_CONNECT_BREAK;
		AddEvent(threadId, evt);
		return -1;
	}
	else {
		NetEvent evt;
		evt.base = base;
		evt.event = netio_event_type::NETIO_EVENT_DATA_RECVED;
		AddEvent(threadId, evt);
		return 0;
	}
}

s32 NetEngine::DoSend(NetBase * base) {
	core::ISession * session = (core::ISession *)base->context;
	Connection * connection = (Connection *)session->GetPipe();
	s32 threadId = connection->GetThreadId();
	s32 ret = connection->SendBack();
	if (ret == Connection::ERROR) {
		connection->SetPipeBroken();
		connection->ResetSending();

		NetEvent evt;
		evt.base = base;
		if (connection->IsSending()) {
			evt.event = netio_event_type::NETIO_EVENT_WAITING_SEND;
			AddEvent(threadId, evt);
		}
		evt.event = netio_event_type::NETIO_EVENT_CONNECT_BREAK;
		AddEvent(threadId, evt);
		return -1;
	}
	else {
		if (ret == Connection::EMPTY) {
			NetEvent evt;
			evt.base = base;
			evt.event = netio_event_type::NETIO_EVENT_WAITING_SEND;
			AddEvent(threadId, evt);
			connection->ResetSending();
		}
		return 0;
	}
}

s32 NetEngine::OnSend(NetBase * base) {
	core::ISession * session = (core::ISession *)base->context;
	Connection * connection = (Connection *)session->GetPipe();
	if (connection->IsSending())
		return DoSend(base);
	return 0;
}

s32 NetEngine::StartSend(NetBase * base) {
	core::ISession * session = (core::ISession *)base->context;
	Connection * connection = (Connection *)session->GetPipe();
	if (connection->IsPipeBroken()) {
		NetEvent evt;
		evt.base = base;
		evt.event = netio_event_type::NETIO_EVENT_WAITING_SEND;
		s32 threadId = connection->GetThreadId();
		AddEvent(threadId, evt);
		return 0;
	}
	connection->SetSending();
	return DoSend(base);
}

void NetEngine::ThreadLoop(Thread * t) {
	while (!t->terminated) {
		ThreadEvent ev;
		s64 tick = tools::GetTimeMillisecond();
		while (t->threadEvents.Read(ev)) {
			if (ev.event == thread_event_type::TET_NEW) {
				if (0 != BindLooper(t->looper, ev.base)) {
					OASSERT(false, "bind looper failed");
				}
			}
			else if (ev.event == thread_event_type::TET_SEND) {
				if (0 != StartSend(ev.base))
					UnbindLooper(ev.base);
			}

			s64 use = tools::GetTimeMillisecond() - tick;
			if (use > ConfigMgr::Instance()->GetNetFrameWaitTick() / 2)
				break;
		}

		s32 overtime = ConfigMgr::Instance()->GetNetFrameWaitTick() - (tools::GetTimeMillisecond() - tick);
		if (overtime < ConfigMgr::Instance()->GetNetFrameWaitTick() / 2)
			overtime = ConfigMgr::Instance()->GetNetFrameWaitTick() / 2;

		DispatchLooper(t->looper, overtime);
		CSLEEP(1);
	}
}

void NetEngine::AddEvent(s32 threadId, const NetEvent& evt) {
	OASSERT(threadId >= 0 && ConfigMgr::Instance()->GetNetThreadCount(), "wtf");
	s_threads[threadId].events.Push(evt);
}

void NetEngine::AddSendEvent(s32 threadId, NetBase * base) {
	OASSERT(threadId >= 0 && ConfigMgr::Instance()->GetNetThreadCount(), "wtf");
	s_threads[threadId].threadEvents.Push({ base, thread_event_type::TET_SEND });
}

s32 NetEngine::SelectThread() {
	s32 count = -1;
	s32 threadId = -1;
	for (s32 i = 0; i < ConfigMgr::Instance()->GetNetThreadCount(); ++i) {
		if (count == -1 || s_threads[threadId].count < count) {
			threadId = i;
			count = s_threads[threadId].count;
		}
	}
	OASSERT(threadId != -1, "wtf");
	return threadId;
}

void NetEngine::DecThreadConnectionCount(s32 threadId) {
	OASSERT(threadId >= 0 && ConfigMgr::Instance()->GetNetThreadCount(), "wtf");
	--s_threads[threadId].count;
}
