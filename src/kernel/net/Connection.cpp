#include "Connection.h"
#include "NetEngine.h"
#include "ConfigMgr.h"
#include "kernel.h"
#include "tools.h"
#include <mutex>

#define ERROR_PARSE_FAILED -1

Connection::Connection(NetBase * base, const s32 fd, const s32 sendSize, const s32 recvSize)
    : _base(base)
	, _fd(fd)
	, _threadId(-1)
    , _session(nullptr)
    , _remotePort(0) 
	, _recvBuff(recvSize)
	, _sendBuff(sendSize)
	, _totalRecvSize(0)
	, _totalSendSize(0)
	, _closeing(false) 
	, _pushing(false)
	, _broken(false)
	, _pipeBroked(false)
	, _sending(false) {
}

Connection::~Connection() {

}

void Connection::Send(const void * context, const s32 size) {
	if (_closeing)
		return;

	if (!_sendBuff.Write(context, size)) {
		Close();
		return;
	}

	if (!_pushing) {
		_pushing = true;
		NetEngine::AddSendEvent(_threadId, _base);
	}
}

void Connection::Close() {
	OASSERT(!_closeing, "wtf");
	if (!_closeing) {
		if (!_pushing)
			shutdown(_fd, SHUT_RDWR);
		_closeing = true;
	}
}

bool Connection::Recv(const void * buff, const s32 size) {
	if (_recvBuff.Write(buff, size)) {
		_totalRecvSize += size;
		return true;
	}
	else
		return false;
}

s32 Connection::SendBack() {
	OASSERT(!_pipeBroked, "wtf");
	char temp[_sendBuff.MaxSize()];
	while (_sendBuff.GetLength() > 0) {
		s32 dataLen;
		const void * data = _sendBuff.Read(temp, dataLen);
		if (data == NULL)
			break;

		int ret = send(_fd, data, dataLen, 0);
		if (ret > 0) {
			_sendBuff.Out(ret);
			_totalSendSize += ret;
		}
		else if (ret == -1 && EAGAIN == errno) 
			return PENDING;
		else 
			return ERROR;
	}
	return EMPTY;
}

void Connection::OnRecv() {
	if (_closeing)
		return;

	char temp[_recvBuff.MaxSize()];
	while (true) {
		if (_closeing)
			break;

		s32 dataLen;
		const void * data = _recvBuff.Read(temp, dataLen);
		if (data == nullptr || dataLen == 0)
			break;

		s32 used = _session->OnRecv(Kernel::Instance(), data, dataLen);
		if (used >= 0) {
			if (used > 0)
				_recvBuff.Out(used);
			else
				break;
		}
		else
			Close();
	}
}

bool Connection::OnSendBack() {
	OASSERT(_pushing, "wtf");
	if (_broken)
		return false;

	if(_sendBuff.GetLength() != 0)
		NetEngine::AddSendEvent(_threadId, _base);
	else {
		if (_closeing)
			shutdown(_fd, SHUT_RDWR);
		_pushing = false;
	}

	return true;
}

bool Connection::CheckPipeBroke() {
	OASSERT(!_broken, "wtf");
	_broken = true;
	if (!_closeing)
		_closeing = true;
	return _pushing;
}
