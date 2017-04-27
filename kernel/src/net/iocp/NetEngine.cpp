#include "NetEngine.h"
#include "Marco.h"
#include "ConfigMgr.h"
#include "IKernel.h"
#include "tools.h"
#include "kernel.h"
#include "Iocp.h"
#include "Connection.h"

LPFN_ACCEPTEX g_accept = nullptr;
LPFN_CONNECTEX g_connect = nullptr;

NetEngine::NetEngine() {
    //ctor
}

NetEngine::~NetEngine() {
    //dtor
}

bool NetEngine::Ready() {
    return true;
}

bool NetEngine::Initialize() {
	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
		return false;
	SetLastError(0);
	if (nullptr == (g_accept = GetAcceptExFunc())) {
		OASSERT(false, "GetAcceptExFun error %d", (s32)::GetLastError());
		return false;
	}

	if (nullptr == (g_connect = GetConnectExFunc())) {
		OASSERT(false, "GetConnectExFun error %d", (s32)::GetLastError());
		return false;
	}

	if (nullptr == (_completionPortAC = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))) {
		OASSERT(false, "CreateIoCompletionPort error %d", (s32)::GetLastError());
		return false;
	}

	if (nullptr == (_completionPortRS = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))) {
		OASSERT(false, "CreateIoCompletionPort error %d", (s32)::GetLastError());
		return false;
	}

	OASSERT(ConfigMgr::Instance()->GetNetThreadCount() > 0, "thread must > 0");
	if (ConfigMgr::Instance()->GetNetThreadCount() <= 0)
		return false;

	_threads = NEW WorkerThread[ConfigMgr::Instance()->GetNetThreadCount()];
	for (s32 i = 0; i < ConfigMgr::Instance()->GetNetThreadCount(); ++i) {
		_threads[i].terminated = false;
		_threads[i].completionPort = _completionPortRS;
		_threads[i].handler = std::thread(NetEngine::ThreadLoop, &_threads[i]);
	}

    return true;
}

s32 NetEngine::Loop(s64 overtime) {
	s64 tick = tools::GetTimeMillisecond();
	ProcessAccept();
	for (s32 i = 0; i < ConfigMgr::Instance()->GetNetThreadCount(); ++i) {
		ProcessThreadEvent(&_threads[i].events, overtime / ConfigMgr::Instance()->GetNetThreadCount());
	}
	return (s32)(tools::GetTimeMillisecond() - tick);
}

void NetEngine::Destroy() {
	CloseHandle(_completionPortRS);
	CloseHandle(_completionPortAC);

	if (_threads) {
		for (s32 i = 0; i < ConfigMgr::Instance()->GetNetThreadCount(); ++i) {
			_threads[i].terminated = true;
			_threads[i].handler.join();
		}
		DEL[] _threads;
	}
	WSACleanup();
	DEL this;
}

bool NetEngine::Listen(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, core::ISessionFactory * factory) {
	SOCKET socket = INVALID_SOCKET;
	if (INVALID_SOCKET == (socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED))) {
		OASSERT(false, "AddClient error %d", ::WSAGetLastError());
		return false;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if ((addr.sin_addr.s_addr = inet_addr(ip)) == INADDR_NONE) {
		closesocket(socket);
		return false;
	}

	if (SOCKET_ERROR == bind(socket, (sockaddr*)&addr, sizeof(sockaddr_in))) {
		closesocket(socket);
		return false;
	}

	if (listen(socket, 2048) == SOCKET_ERROR) {
		closesocket(socket);
		return false;
	}

	if (_completionPortAC != CreateIoCompletionPort((HANDLE)socket, _completionPortAC, socket, 0)) {
		closesocket(socket);
		return false;
	}

	IocpAcceptor * acceptor = NEW IocpAcceptor;
	acceptor->factory = factory;
	acceptor->recvSize = recvSize;
	acceptor->sendSize = sendSize;
	acceptor->socket = socket;

	SafeMemset(&acceptor->accept, sizeof(acceptor->accept), 0, sizeof(acceptor->accept));
	acceptor->accept.opt = IOCP_OPT_ACCEPT;
	acceptor->accept.code = 0;
	acceptor->accept.context = acceptor;

	if (!DoAccept(&acceptor->accept)) {
		closesocket(socket);
		DEL acceptor;
		return false;
	}
	return true;
}

bool NetEngine::Connect(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, core::ISession * session) {
	SOCKET socket = INVALID_SOCKET;
	if (INVALID_SOCKET == (socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED))) {
		OASSERT(false, "AddClient error %d", ::WSAGetLastError());

		session->OnConnectFailed(Kernel::Instance());
		return false;
	}

	DWORD value = 0;
	if (SOCKET_ERROR == ioctlsocket(socket, FIONBIO, &value)) {
		session->OnConnectFailed(Kernel::Instance());
		closesocket(socket);
		return false;
	}

	const s8 nodelay = 1;
	setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

	if (_completionPortRS != CreateIoCompletionPort((HANDLE)socket, _completionPortRS, socket, 0)) {
		session->OnConnectFailed(Kernel::Instance());
		closesocket(socket);
		return false;
	}

	Connection * connection = Connection::Create(socket, sendSize, recvSize);
	OASSERT(connection, "wtf");
	if (!connection->DoConnect(ip, port)) {
		OASSERT(false, "AddClient error %u", (s32)::GetLastError());

		session->OnConnectFailed(Kernel::Instance());
		closesocket(socket);
		connection->OnRelease();
		return false;
	}

	connection->SetSession(session);
	connection->SetRemoteIp(ip);
	connection->SetRemotePort(port);
	return true;
}

void NetEngine::ProcessAccept() {
	IocpEvent * evt = GetQueueState(_completionPortAC);
	if (evt) {
		OASSERT(evt->opt == IOCP_OPT_ACCEPT, "wtf");
		Accept(evt);
	}
}

void NetEngine::ProcessThreadEvent(olib::CycleQueue<NetEvent> * events, s64 overtime) {
	NetEvent evt;
	while (events->Read(evt)) {
		switch (evt.type) {
		case CONNECTED: {
				if (!evt.connection->OnConnected()) {
					evt.connection->OnConnectFailed();
					evt.connection->OnRelease();
				}
			}
			break;
		case CONNECTFAILED: {
				evt.connection->OnConnectFailed();
				evt.connection->OnRelease();
			}
			break;
		case RECV: evt.connection->OnRecv(); break;
		case RECV_BREAK: {
				if (evt.connection->OnRecvDone())
					evt.connection->OnRelease();
			}
			break;
		case SEND_DONE: evt.connection->OnSend(); break;
		case SEND_BREAK: {
				if (evt.connection->OnSendDone())
					evt.connection->OnRelease();
			}
			break;
		}
	}
}

void NetEngine::ThreadLoop(WorkerThread * t) {
	while (!t->terminated) {
		IocpEvent * evt = GetQueueState(t->completionPort);
		if (evt) {
			switch (evt->opt) {
			case IOCP_OPT_CONNECT: Connect(t, evt); break;
			case IOCP_OPT_RECV: Recv(t, evt); break;
			case IOCP_OPT_SEND: Send(t, evt); break;
			}
		}
		else {
			CSLEEP(1);
		}
	}
}

IocpEvent * NetEngine::GetQueueState(HANDLE completionPort) {
	DWORD bytes = 0;
	s64 socket = INVALID_SOCKET;
	IocpEvent * evt = nullptr;

	SetLastError(0);
	BOOL ret = GetQueuedCompletionStatus(completionPort, &bytes, (PULONG_PTR)&socket, (LPOVERLAPPED *)&evt, 10);

	if (nullptr == evt)
		return nullptr;

	evt->code = WSAGetLastError();
	evt->bytes = bytes;
	if (!ret) {
		if (WAIT_TIMEOUT == evt->code)
			return nullptr;
	}
	return evt;
}

void NetEngine::Accept(IocpEvent * evt) {
	IocpAcceptor * acceptor = (IocpAcceptor *)evt->context;
	BOOL res = setsockopt(evt->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (const char *)&acceptor->socket, sizeof(acceptor->socket));

	sockaddr_in remote;
	s32 size = sizeof(sockaddr_in);
	if (res != 0 || 0 != getpeername(evt->socket, (sockaddr*)&remote, &size)) {
		OASSERT(false, "complete accept error %d", GetLastError());
		::closesocket(evt->socket);
	}
	else {
		HANDLE ret = CreateIoCompletionPort((HANDLE)evt->socket, _completionPortRS, evt->socket, 0);
		if (_completionPortRS != ret)
			closesocket(evt->socket);
		else {
			ISession * session = acceptor->factory->Create();
			if (!session) {
				::closesocket(evt->socket);
			}
			else {
				const s8 nodelay = 1;
				setsockopt(evt->socket, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

				Connection * connection = Connection::Create(evt->socket, acceptor->sendSize, acceptor->recvSize);
				OASSERT(connection, "wtf");

				connection->SetSession(session);
				connection->SetRemoteIp(inet_ntoa(remote.sin_addr));
				connection->SetRemotePort(ntohs(remote.sin_port));

				if (!connection->OnConnected()) {
					closesocket(evt->socket);
					acceptor->factory->Recover(session);
					connection->OnRelease();
				}
			}
		}
	}

	DoAccept(evt);
}

bool NetEngine::DoAccept(IocpEvent * evt) {
	evt->socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (evt->socket == INVALID_SOCKET) {
		OASSERT(false, "do accept failed %d", WSAGetLastError());
		return false;
	}

	DWORD bytes = 0;
	IocpAcceptor * acceptor = (IocpAcceptor *)evt->context;
	s32 res = g_accept(acceptor->socket,
		evt->socket,
		acceptor->buf,
		0,
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		&bytes,
		(LPOVERLAPPED)evt
	);

	s32 errCode = WSAGetLastError();
	if (!res && errCode != WSA_IO_PENDING) {
		OASSERT(false, "do accept failed %d", errCode);
		return false;
	}
	return true;
}

void NetEngine::Connect(WorkerThread * t, IocpEvent * evt) {
	NetEvent netEvt = { evt->code == ERROR_SUCCESS ? CONNECTED : CONNECTFAILED , (Connection*)evt->context };
	t->events.Push(netEvt);
}

void NetEngine::Recv(WorkerThread * t, IocpEvent * evt) {
	Connection * connection = (Connection*)evt->context;
	if (evt->code == ERROR_SUCCESS || evt->bytes > 0) {
		if (!connection->IsClosing() && connection->In(evt->buf.buf, evt->bytes) && connection->DoRecv()) {
			NetEvent netEvt = { RECV , connection };
			t->events.Push(netEvt);
			return;
		}
	}

	connection->CloseThread();
	NetEvent netEvt = { RECV_BREAK , connection };
	t->events.Push(netEvt);
}

void NetEngine::Send(WorkerThread * t, IocpEvent * evt) {
	Connection * connection = (Connection*)evt->context;
	if (evt->code == ERROR_SUCCESS) {
		if (connection->Out(evt->bytes) && connection->DoSend())
			return;
	}
	if (evt->code != ERROR_SUCCESS)
		connection->CloseThread();

	NetEvent netEvt = { evt->code == ERROR_SUCCESS ? SEND_DONE : SEND_BREAK , connection };
	t->events.Push(netEvt);
}
