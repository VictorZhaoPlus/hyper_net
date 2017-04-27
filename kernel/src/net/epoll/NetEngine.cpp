#include "NetEngine.h"
#include "Marco.h"
#include "Connection.h"
#include "ConfigMgr.h"
#include "IKernel.h"
#include "tools.h"
#include "kernel.h"
#include <signal.h>

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
	_directSend = ConfigMgr::Instance()->GetNetDirectSend();
	_looper = MallocLooper(size, OnAccept, OnConnect, OnRecv, OnSend);
	
    return true;
}

s32 NetEngine::Loop(s64 overtime) {
    s64 tick = tools::GetTimeMillisecond();

	DispatchLooper(_looper, ConfigMgr::Instance()->GetNetFrameWaitTick());
	Flush();
	return tools::GetTimeMillisecond() - tick;
}

void NetEngine::Destroy() {
	if (_looper) {
		FreeLooper(_looper);
		_looper = nullptr;
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
	
	if (0 != BindLooper(_looper, base))
		return -1;

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

	session->OnConnected(Kernel::Instance());
	return 0;
}

s32 NetEngine::OnConnect(NetBase * connecter, const int code) {
	core::ISession * session = (core::ISession *)connecter->context;
	if (0 == code) {
		if (0 != BindLooper(_looper, base)) {
			session->OnConnectFailed(Kernel::Instance());
			return -1;
		}
	
		Connection * connection = Connection::Create(connecter, connecter->fd, connecter->maxSendSize, connecter->maxRecvSize);
		OASSERT(connection != nullptr, "wtf");
		connection->SetSession(session);

		sockaddr_in remote;
		socklen_t len = sizeof(remote);
		getpeername(connecter->fd, (sockaddr*)&remote, &len);
		connection->SetRemoteIp(inet_ntoa(remote.sin_addr));
		connection->SetRemotePort(ntohs(remote.sin_port));

		session->OnConnected(Kernel::Instance());
	}
	else
		session->OnConnectFailed(Kernel::Instance());
	return 0;
}

s32 NetEngine::OnRecv(NetBase * base, const s32 code, const char * buff, const s32 size) {
	core::ISession * session = (core::ISession *)base->context;
	Connection * connection = (Connection *)session->GetPipe();
	if (code != 0 || size == 0) {
		session->SetPipe(nullptr);
		session->OnDisconnected(Kernel::Instance());
		NetEngine::Instance()->RemoveFromChain(connection);
		connection->OnRelease();
		return -1;
	}

	connection->OnRecv(size);
	return 0;
}


s32 NetEngine::OnSend(NetBase * base) {
	core::ISession * session = (core::ISession *)base->context;
	Connection * connection = (Connection *)session->GetPipe();
	connection->OnSendBack();
	return 0;
}

void NetEngine::Flush() {
	if (_directSend)
		return;
	
	Connection * connection = _head;
	while (connection) {
		connection->OnSendBack();
		 
		Connection * next = connection->GetNext();
		connection->SetNext(nullptr);
		connection->SetPrev(nullptr);
		connection->SetNeedSend(false);
		
		connection = next;
	}
	_head = nullptr;
}

void NetEngine::InsertIntoChain(Connection * connection) {
	if (_directSend)
		return;
	
	if (!connection->IsNeedSend()) {
		if (_head)
			_head->SetPrev(connection);
		connection->SetNext(_head);
		connection->SetPrev(nullptr);
		connection->SetNeedSend(true);
		_head = connection;
	}
}

void NetEngine::RemoveFromChain(Connection * connection) {
	if (_directSend)
		return;
	
	if (connection->IsNeedSend()) {
		Connection * prev = connection->GetPrev();
		Connection * next = connection->GetNext();
		
		if (prev)
			prev->SetNext(next);
		
		if (next)
			next->SetPrev(prev);
		
		if (_head == connection)
			_head = next;
		
		connection->SetNext(nullptr);
		connection->SetPrev(nullptr);
		connection->SetNeedSend(false);
	}
}
