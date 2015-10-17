#include "Connection.h"
#include "NetHeader.h"
#include "NetEngine.h"
#include "ConfigMgr.h"
#include "kernel.h"

#define ERROR_PARSE_FAILED -1

Connection::Connection(const s32 fd)
    : INetHandler(fd)
    , _session(nullptr)
    , _localPort(0)
    , _remotePort(0)
	, _canSend(false)
	, _valid(true) 
    , _recving(false) {
	memset(&_sendBuff, 0, sizeof(_sendBuff));
	memset(&_recvBuff, 0, sizeof(_recvBuff));
}

Connection::~Connection() {
	if (_sendBuff.data)
		FREE(_sendBuff.data);

	if (_recvBuff.data)
		FREE(_recvBuff.data);
}

void Connection::SetBufferSize(const s32 sendSize, const s32 recvSize) {
	_sendBuff.data = (char *)MALLOC(sendSize);
	_sendBuff.size = sendSize;

	_recvBuff.data = (char *)MALLOC(recvSize);
	_recvBuff.size = recvSize;
}

void Connection::OnConnected() {
    OASSERT(_session, "session is null");
    _session->OnConnected(Kernel::Instance());
}

void Connection::OnIn() {
	_recving = true;
	while (true) {
		if (_recvBuff.offset == _recvBuff.size)
			Expand(_recvBuff);

		s32 res = recv(_fd, _recvBuff.data + _recvBuff.offset, _recvBuff.size - _recvBuff.offset, 0);
		if (res < 0) {
			if (errno != EAGAIN)
				OnError(errno);
			break;
		}
		else if (res == 0) {
			OnClose();
			break;
		}
		else {
			_recvBuff.offset += res;

			bool parseOk = true;
			s32 pos = 0;
			while (pos < _recvBuff.offset) {
				s32 used = _session->OnRecv(Kernel::Instance(), _recvBuff.data + pos, _recvBuff.offset - pos);
				if (used == ON_RECV_FAILED) {
					parseOk = false;
					break;
				}
				else if (used == 0)
					break;
				else
					pos += used;

				if (!_valid)
					break;
			}

			if (_valid) {
				if (!parseOk) {
					OnError(ERROR_PARSE_FAILED);
					break;
				}
				else {
					if (pos < _recvBuff.offset)
						memmove(_recvBuff.data, _recvBuff.data + pos, _recvBuff.offset - pos);
					_recvBuff.offset -= pos;
				}
			}
			else
				break;
		}
	}
	_recving = false;
	if (!_valid)
		OnClose();
}

void Connection::OnOut() {
	_canSend = true;
	OnSend();
}

void Connection::OnError() {
	OnError(errno);
}

void Connection::Send(const void * context, const s32 size) {
	while (_sendBuff.offset + size > _sendBuff.size) {
		Expand(_sendBuff);
	}

	memcpy(_sendBuff.data + _sendBuff.offset, context, size);
	_sendBuff.offset += size;

	if (_canSend)
		OnSend();
}

void Connection::Close() {
    OASSERT(_session, "session is null");
	if (_recving)
		_valid = false;
	else
		OnClose();
}

void Connection::Expand(Buffer& buff) {
	buff.data = (char*)REALLOC(buff.data, buff.size * 2);
	buff.size *= 2;
}

void Connection::OnSend() {
	while (_sendBuff.offset > 0) {
		s32 res = send(_fd, _sendBuff.data, _sendBuff.offset, 0);
		if (res < 0) {
			if (errno != EAGAIN)
				OnError(errno);
			break;
		}
		else {
			if (res < _sendBuff.offset)
				memmove(_sendBuff.data, _sendBuff.data + res, _sendBuff.size - res);
			_sendBuff.offset -= res;
		}
	}
}

void Connection::OnError(s32 errCode) {
	_session->OnError(Kernel::Instance(), errCode);
	if (_recving)
		_valid = false;
	else
		OnClose();
}

void Connection::OnClose() {
	OASSERT(_session, "session is null");

	_session->SetPipe(nullptr);
	NetEngine::Instance()->AddToWaitRelease(_session);

	NetEngine::Instance()->Del(this, EPOLLIN | EPOLLOUT);
	close(_fd);
	NetEngine::Instance()->Remove(this);
	DEL this;
}
