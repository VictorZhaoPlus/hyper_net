#include "Connection.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include "tools.h"
#include "kernel.h"
#include "ConfigMgr.h"
#include "NetEngine.h"
#include "NetWorker.h"

#define ERROR_PARSE_FAILED -1
#define SINGLE_RECV_SIZE 32768

s32 SetQuickAck(const s32 fd) {
	s32 val = 1;
	return setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, (const char*)&val, sizeof(val));
}

Connection::Connection(const s32 fd, const s32 sendSize, const s32 recvSize)
    : _fd(fd)
    , _session(nullptr)
	, _worker(nullptr)
    , _remotePort(0) 
	, _localPort(0)
	, _profile(false)
	, _closing(false) 
	, _threadStatus(ST_NORMAL)
	, _pending(false)
	, _changing(false)
	, _adjustSendSize(sendSize)
	, _adjustRecvSize(recvSize) {
	_sendBuff = RingBufferAlloc(sendSize);
	_recvBuff = RingBufferAlloc(recvSize);
}

Connection::~Connection() {
	RingBufferDestroy(_sendBuff);
	RingBufferDestroy(_recvBuff);
}

void Connection::AdjustSendBuffSize(u32 size) {
	if (_closing)
		return;

	_adjustSendSize = RingBufferCalcSize(size);
	
	if (!_changing) {
		_changing = true;
		_worker->PostChange(this, tools::GetTimeMillisecond());
	}
}

void Connection::AdjustRecvBuffSize(u32 size) {
	if (_closing)
		return;
	
	_adjustRecvSize = RingBufferCalcSize(size);
	
	if (!_changing) {
		_changing = true;
		_worker->PostChange(this, tools::GetTimeMillisecond());
	}
}

void Connection::Send(const void * context, const s32 size) {
	if (_closing)
		return;

	if (!RingBufferWriteBlock(_sendBuff, context, size)) {
		KERNEL_LOG("connection %s:%d send over flow", _remoteIp, _remotePort);
		Close();
		return;
	}
	
	if (!_changing)
		_worker->PostSend(this, tools::GetTimeMillisecond());
	if (_profile)
		KERNEL_LOG("%lld:%d write %d", tools::GetTimeMillisecond(), _localPort, size);
}

void Connection::Close() {
	if (!_closing) {
		_closing = true;
		_worker->PostClosing(this, tools::GetTimeMillisecond());
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
			OASSERT(totalUsed + used <= (s32)dataLen, "wtf");
			RingBufferOut(_recvBuff, used);
			totalUsed += used;
			
			if (_profile)
				KERNEL_LOG("%lld:%d read %d", tools::GetTimeMillisecond(), _localPort, used);
		}

	} while (used > 0 && totalUsed < (s32)dataLen);
	
	if (used < 0)
		Close();
}

void Connection::OnDone() {
	OASSERT(_closing, "wtf");
	
	KERNEL_LOG("connection %s:%d close", _remoteIp, _remotePort);
	close(_fd);
	_session->OnDisconnected(Kernel::Instance());
	_session->SetPipe(nullptr);
	_session->OnRelease();
	_worker->DelCounter(this);
	OnRelease();
}

void Connection::OnChange() {
	if (_closing)
		return;
	
	OASSERT(_changing, "wtf");
	
	_changing = false;
	
	if (_sendBuff->size != _adjustSendSize)
		RingBufferRealloc(_sendBuff, _adjustSendSize);

	if (_recvBuff->size != _adjustRecvSize)
		RingBufferRealloc(_recvBuff, _adjustRecvSize);

	_worker->DelCounter(this);
	_worker->Add(this);
	
	if (RingBufferLength(_sendBuff) > 0)
		_worker->PostSend(this, tools::GetTimeMillisecond());
}

bool Connection::ThreadRecv() {
	if (_threadStatus == ST_NORMAL) {
		while (true) {
			u32 size = 0;
			s32 len = 0;
			char * buf = RingBufferWrite(_recvBuff, &size);
			if (buf && size > 0) {
				SetQuickAck(_fd);
				len = recv(_fd, buf, size, 0);
				if (len > 0) {
					RingBufferIn(_recvBuff, len);
					_worker->PostRecv(this, tools::GetTimeMillisecond());
					
					if (_profile)
						KERNEL_LOG("%lld:%d recv %d", tools::GetTimeMillisecond(), _localPort, len);
				}
				else if (len < 0 && errno == EAGAIN) {
					break;
				}
			}
			else
				len = -1;

			if (len <= 0) {
				KERNEL_LOG("connection %s:%d close %d %d", _remoteIp, _remotePort, len, errno);
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
				if (_profile)
					KERNEL_LOG("%lld:%d send %d/%d", tools::GetTimeMillisecond(), _localPort, ret, dataLen);
			}
			else if (ret == -1 && EAGAIN == errno) {
				_pending = true;
				if (_profile)
					KERNEL_LOG("%lld:%d pending", tools::GetTimeMillisecond(), _localPort);
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
			_worker->Remove(this);
			_worker->PostError(this, tools::GetTimeMillisecond());
		}
		else {
			if (RingBufferLength(_sendBuff) == 0 || _changing) {
				_threadStatus = ST_CLOSED;
				_worker->Remove(this);
				_worker->PostDone(this, tools::GetTimeMillisecond());
			}
			else
				_threadStatus = ST_CLOSING;
		}
	}
	else if (_threadStatus == ST_ERROR) {
		if (!force) {
			_threadStatus = ST_CLOSED;
			_worker->PostDone(this, tools::GetTimeMillisecond());
		}
	}
	else if (_threadStatus == ST_CLOSING) {
		if (force) {
			_threadStatus = ST_CLOSED;
			_worker->Remove(this);
			_worker->PostDone(this, tools::GetTimeMillisecond());
		}
	}
	else {
		OASSERT(false, "wtf");
	}
}

void Connection::ThreadChange() {
	if (_threadStatus == ST_NORMAL) {
		_worker->Remove(this);
		_worker->PostChanged(this, tools::GetTimeMillisecond());
	}
}
