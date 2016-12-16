#include "Connection.h"
#include "NetEngine.h"
#include "kernel.h"
#include "tools.h"
#include "ORingBuffer.h"

#define ERROR_PARSE_FAILED -1
#define SINGLE_RECV_SIZE 32768

Connection::Connection(NetBase * base, const s32 fd, const s32 sendSize, const s32 recvSize)
    : _base(base)
	, _fd(fd)
    , _session(nullptr)
    , _remotePort(0) 
	, _closeing(false) 
	, _prev(false)
	, _next(false)
	, _needSend(false) {
	_base->sendBuff = RingBufferAlloc(sendSize);
	_base->recvBuff = RingBufferAlloc(recvSize);
}

Connection::~Connection() {
	RingBufferDestroy(_base->sendBuff);
	RingBufferDestroy(_base->recvBuff);
}

void Connection::Send(const void * context, const s32 size) {
	if (_closeing)
		return;

	bool pending = (RingBufferLength(_base->sendBuff) > 0);
	if (!RingBufferWriteBlock(_base->sendBuff, context, size)) {
		Close();
		return;
	}
	
	if (pending) {
		if (NetEngine::Instance()->IsDirectSend()) {
			if (SendBack() == ERROR)
				InnerClose();
		}
		else 
			NetEngine::Instance()->InsertIntoChain(this);
	}
}

void Connection::Close() {
	if (!_closeing) {
		if (RingBufferLength(_base->sendBuff) == 0)
			shutdown(_fd, SHUT_RDWR);
		_closeing = true;
	}
}

void Connection::InnerClose() {
	if (!_closeing) {
		shutdown(_fd, SHUT_RDWR);
		_closeing = true;
	}
}

void Connection::OnRecv(const s32 size) {
	if (!_closeing) {
		RingBufferIn(_base->recvBuff, size);
		Recv();
	}
}

void Connection::Recv(const void * buff, const s32 size) {
	char temp[SINGLE_RECV_SIZE];
	while (true) {
		if (_closeing)
			break;
		
		u32 dataLen = 0;
		char * data = RingBufferReadTemp(_base->recvBuff, (char*)temp, sizeof(temp), &dataLen);
		if (data == nullptr || dataLen == 0)
			break;
		
		s32 used = 0;
		s32 totalUsed = 0;
		do {
			used = _session->OnRecv(Kernel::Instance(), data + totalUsed, (s32)dataLen - totalUsed);
			if (used > 0) {
				OASSERT(totalUsed + used < (s32)dataLen, "wtf");
				totalUsed += used;
			}

		} while (used > 0 && totalUsed < dataLen);
		
		if (used >= 0) {
			if (totalUsed > 0)
				RingBufferOut(_base->recvBuff, totalUsed);
			else
				break;
		}
		else
			Close();
	}
}

void Connection::OnSendBack() {
	if (RingBufferLength(_base->sendBuff) > 0) {
		s32 ret = SendBack();
		if (_closeing) {
			if (ret != PENDING) {
				shutdown(_fd, SHUT_RDWR);
				return;
			}
		}
		
		if (ret == ERROR)
			InnerClose();
	}
}

s32 Connection::SendBack() {
	while (RingBufferLength(_base->sendBuff) > 0) {
		s32 dataLen;
		const void * data = RingBufferRead(_base->sendBuff, &dataLen);
		if (data == NULL)
			break;

		int ret = send(_fd, data, dataLen, 0);
		if (ret > 0) {
			RingBufferOut(_base->sendBuff, ret);
		}
		else if (ret == -1 && EAGAIN == errno) 
			return PENDING;
		else 
			return ERROR;
	}
	return EMPTY;
}

