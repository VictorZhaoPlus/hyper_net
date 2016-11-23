#include "Connection.h"
#include "NetEngine.h"
#include "ConfigMgr.h"
#include "kernel.h"
#include "tools.h"
#include <mutex>

#define ERROR_PARSE_FAILED -1
#define SINGLE_RECV_SIZE 8192

extern LPFN_CONNECTEX g_connect;

Connection::Connection(const SOCKET fd, const s32 sendSize, const s32 recvSize)
    : _fd(fd)
    , _session(nullptr)
    , _remotePort(0) 
#ifdef _DEBUG
	, _establish(false)
#endif
	, _closing(false)
	, _sending(false)
	, _recving(false)
	, _isSocketClosed(false) {
	OASSERT(recvSize > SINGLE_RECV_SIZE, "wtf");
	_sendBuf = RingBufferAlloc(sendSize);
	_recvBuf = RingBufferAlloc(recvSize);

	SafeMemset(&_addr, sizeof(_addr), 0, sizeof(_addr));

	_connect.opt = IOCP_OPT_CONNECT;
	_connect.socket = _fd;
	_connect.bytes = 0;
	_connect.context = this;

	_recv.opt = IOCP_OPT_RECV;
	_recv.socket = _fd;
	_recv.bytes = 0;
	_recv.context = this;
	_recv.buf.buf = (char*)MALLOC(SINGLE_RECV_SIZE);
	_recv.buf.len = SINGLE_RECV_SIZE;

	_send.opt = IOCP_OPT_SEND;
	_recv.socket = _fd;
	_recv.bytes = 0;
	_recv.context = this;
}

Connection::~Connection() {
	RingBufferDestroy(_sendBuf);
	RingBufferDestroy(_recvBuf);
	FREE(_recv.buf.buf);
}

void Connection::Send(const void * context, const s32 size) {
	if (_closing)
		return;

	if (!RingBufferWriteBlock(_sendBuf, context, size)) {
		Close();
		return;
	}
	
	if (!_sending) {
		if (!DoSend())
			CloseForce();
		else {
			_sending = true;
		}
	}
}

void Connection::Close() {
	_closing = true;
}

void Connection::CloseForce() {
	if (!_isSocketClosed) {
		shutdown(_fd, SD_BOTH);
		closesocket(_fd);

		_isSocketClosed = true;
		_closing = true;
	}
}

void Connection::CloseThread() {
	if (!_isSocketClosed) {
		shutdown(_fd, SD_BOTH);
		closesocket(_fd);

		_isSocketClosed = true;
		_closing = true;
	}
}

bool Connection::OnConnected() {
#ifdef _DEBUG
	_establish = true;
#endif
	if (DoRecv()) {
		_session->OnConnected(Kernel::Instance());
		_recving = true;
		return true;
	}
	return false;
}

void Connection::OnConnectFailed() {
	::closesocket(_fd);
#ifdef _DEBUG
	_establish = false;
#endif
	_session->OnConnectFailed(Kernel::Instance());
	_session->SetPipe(nullptr);
}

bool Connection::DoConnect(const char * ip, const s32 port) {
	_addr.sin_family = AF_INET;
	if (SOCKET_ERROR == bind(_fd, (sockaddr *)&_addr, sizeof(sockaddr_in)))
		return false;

	_addr.sin_port = htons(port);
	if ((_addr.sin_addr.s_addr = inet_addr(ip)) == INADDR_NONE)
		return false;

	BOOL res = g_connect(_fd, (sockaddr *)&_addr, sizeof(sockaddr_in), nullptr, 0, nullptr, (LPOVERLAPPED)&_connect);
	s32 errCode = WSAGetLastError();
	if (!res && errCode != WSA_IO_PENDING) {
		return false;
	}

	return true;
}

bool Connection::In(const char * buf, const s32 size) {
	return RingBufferWriteBlock(_recvBuf, buf, size) != 0;
}

void Connection::OnRecv() {
	if (_closing)
		return;

	char temp[SINGLE_RECV_SIZE];
	u32 size = 0;
	char * buf = RingBufferReadTemp(_recvBuf, (char*)temp, sizeof(temp), &size);
	if (buf && size > 0) {
		s32 used = 0;
		do {
			used = _session->OnRecv(Kernel::Instance(), buf + used, size);
			if (used > 0) {
				OASSERT(used <= (s32)size, "wtf");
				RingBufferOut(_sendBuf, used);
				size -= used;
			}
		} while (used > 0 && size > 0);

		if (used < 0)
			Close();
	}
}

bool Connection::OnRecvDone() {
	OASSERT(_closing, "wtf");
	_recving = false;

	if (_sending)
		return false;
#ifdef _DEBUG
	_establish = false;
#endif
	_session->OnDisconnected(Kernel::Instance());
	_session->SetPipe(nullptr);
	return true;
}

bool Connection::DoRecv() {
#ifdef _DEBUG
	OASSERT(_establish, "wtf");
#endif
	DWORD flags = 0;
	_recv.code = 0;
	_recv.bytes = 0;
	if (SOCKET_ERROR == WSARecv(_fd, &_recv.buf, 1, nullptr, &flags, (LPWSAOVERLAPPED)&_recv, nullptr)) {
		_recv.code = WSAGetLastError();
		if (WSA_IO_PENDING != _recv.code)
			return false;
	}
	return true;
}

void Connection::OnSend() {
	_sending = false;

	u32 size = 0;
	char * buf = RingBufferRead(_sendBuf, &size);
	if (buf && size > 0) {
		if (!DoSend()) {
			CloseForce();
			return;
		}
		_sending = true;
	}
	else {
		if (_closing)
			CloseForce();
	}
}

bool Connection::OnSendDone() {
	OASSERT(_closing, "wtf");
	_sending = false;

	if (_recving)
		return false;
#ifdef _DEBUG
	_establish = false;
#endif
	_session->OnDisconnected(Kernel::Instance());
	_session->SetPipe(nullptr);
	return true;
}

bool Connection::Out(const s32 size) {
	RingBufferOut(_sendBuf, size);

	u32 left = 0;
	return RingBufferRead(_sendBuf, &left) != nullptr;
}

bool Connection::DoSend() {
#ifdef _DEBUG
	OASSERT(_establish, "wtf");
#endif
	u32 size = 0;
	char * buf = RingBufferRead(_sendBuf, &size);
	OASSERT(buf && size > 0, "wtf");
	
	_send.buf.buf = buf;
	_send.buf.len = size;
	_send.code = 0;
	_send.bytes = 0;
	if (SOCKET_ERROR == WSASend(_fd, &_send.buf, 1, nullptr, 0, (LPWSAOVERLAPPED)&_send, nullptr)) {
		_send.code = WSAGetLastError();
		if (WSA_IO_PENDING != _send.code)
			return false;
	}
	return true;
}

