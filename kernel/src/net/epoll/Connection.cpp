#include "Connection.h"
#include "NetEngine.h"
#include "kernel.h"
#include "tools.h"
#include "NetWorker.h"
#include "ConfigMgr.h"
#include <sys/socket.h>

#define ERROR_PARSE_FAILED -1
#define SINGLE_RECV_SIZE 32768

Connection::Connection(const s32 fd, const s32 sendSize, const s32 recvSize)
    : _fd(fd)
    , _session(nullptr)
	, _worker(nullptr)
    , _remotePort(0) 
	, _closing(false) 
	, _threadStatus(ST_NORMAL) {
	_sendBuff = RingBufferAlloc(sendSize);
	_recvBuff = RingBufferAlloc(recvSize);
}

Connection::~Connection() {
	RingBufferDestroy(_sendBuff);
	RingBufferDestroy(_recvBuff);
}

void Connection::Send(const void * context, const s32 size) {
	if (_closing)
		return;

	if (!RingBufferWriteBlock(_sendBuff, context, size)) {
		Close();
		return;
	}
	
	_worker->PostSend(this);
}

void Connection::Close() {
	if (!_closing) {
		_closing = true;
		_worker->PostClosing(this);
	}
}

void Connection::OnRecv() {
	if (_closing)
		return;
	
	char temp[ConfigMgr::Instance()->GetNetMaxPacketSize()];
	u32 dataLen = 0;
	char * data = RingBufferReadTemp(_recvBuff, (char*)temp, sizeof(temp), &dataLen);
	if (data == nullptr || dataLen == 0)
		return;
	
	s32 used = 0;
	s32 totalUsed = 0;
	do {
		used = _session->OnRecv(Kernel::Instance(), data + totalUsed, (s32)dataLen - totalUsed);
		if (used > 0) {
			OASSERT(totalUsed + used < (s32)dataLen, "wtf");
			totalUsed += used;
		}

	} while (used > 0 && totalUsed < (s32)dataLen);
	
	if (used >= 0) {
		if (totalUsed > 0)
			RingBufferOut(_recvBuff, totalUsed);
	}
	else
		Close();
}

void Connection::OnDone() {
	OASSERT(_closing, "wtf");
	
	close(_fd);
	_session->OnDisconnected(Kernel::Instance());
	_session->SetPipe(nullptr);
	OnRelease();
}

bool Connection::ThreadRecv() {
	if (_threadStatus == ST_NORMAL) {
		s32 totalRead = 0;
		while (true) {
			u32 size = 0;
			s32 len = 0;
			char * buf = RingBufferWrite(_recvBuff, &size);
			if (buf && size > 0) {
				len = recv(_fd, buf, size, 0);
				if (len > 0)
					totalRead += len;
				else if (len < 0 && errno == EAGAIN) {
					if (totalRead > 0) {
						RingBufferIn(_recvBuff, totalRead);
						_worker->PostRecv(this);
					}
					break;
				}
			}
			else
				len = -1;

			if (len <= 0) {
				if (totalRead > 0) {
					RingBufferIn(_recvBuff, totalRead);
					_worker->PostRecv(this);
				}
				ThreadClose(true);
				return false;
			}
		}
	}
	return true;
}

void Connection::ThreadSend(bool reset) {
	if (_pending && !reset)
		return;
	
	_pending = false;
	if (_threadStatus == ST_NORMAL || _threadStatus == ST_CLOSING) {
		while (RingBufferLength(_sendBuff) > 0) {
			u32 dataLen;
			const void * data = RingBufferRead(_sendBuff, &dataLen);
			if (data == NULL)
				break;

			int ret = send(_fd, data, dataLen, 0);
			if (ret > 0) {
				RingBufferOut(_sendBuff, ret);
			}
			else if (ret == -1 && EAGAIN == errno) {
				_pending = true;
				return;
			}
			else  {
				ThreadClose(true);
				return;
			}
		}
		
		if (_threadStatus == ST_CLOSING)
			ThreadClose(true);
	}
}

void Connection::ThreadClose(bool force) {
	if (_threadStatus == ST_NORMAL) {
		if (force) {
			_threadStatus = ST_ERROR;
			_worker->PostError(this);
		}
		else
			_threadStatus = ST_CLOSING;
	}
	else if (_threadStatus == ST_ERROR) {
		if (!force) {
			_threadStatus = ST_CLOSED;
			_worker->Remove(this);
			_worker->PostDone(this);
		}
	}
	else if (_threadStatus == ST_CLOSING) {
		if (force) {
			_threadStatus = ST_CLOSED;
			_worker->Remove(this);
			_worker->PostDone(this);
		}
	}
	else {
		OASSERT(false, "wtf");
	}
}
