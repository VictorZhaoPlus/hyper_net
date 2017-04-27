#ifndef __CONNECTION_H__
#define __CONNECTION_H__
#include "IKernel.h"
#include "NetLoop.h"
#include "CircularBuffer.h"

#define MAX_IP_SIZE 32
class Connection : public core::IPipe {
public:
	enum {
		ERROR = -1,
		EMPTY,
		PENDING,
	};

    static Connection * Create(NetBase * base, const s32 fd, const s32 sendSize, const s32 recvSize) {
        return NEW Connection(base, fd, sendSize, recvSize);
    }

	void OnRelease() { DEL this; }

    inline void SetSession(core::ISession * session) {
        _session = session;
        session->SetPipe(this);
    }

    inline void SetRemoteIp(const char * ip) { SafeSprintf(_remoteIp, sizeof(_remoteIp) - 1, ip); }
    virtual const char * GetRemoteIp() const { return _remoteIp; }

    inline void SetRemotePort(const s32 port) { _remotePort = port; }
    virtual s32 GetRemotePort() const { return _remotePort; }

    virtual void Send(const void * context, const s32 size);
    virtual void Close();

	inline bool IsClosing() const { return _closeing; }

	inline bool IsPipeBroken() const { return _pipeBroked; }
	inline void SetPipeBroken() { _pipeBroked = true; }

	inline bool IsSending() const { return _sending; }
	inline void SetSending() { _sending = true; }
	inline void ResetSending() { _sending = false; }

	inline void SetThreadId(s32 id) { _threadId = id; }
	inline s32 GetThreadId() const { return _threadId; }

	bool Recv(const void * buff, const s32 size);
	s32 SendBack();

	void OnRecv();
	bool OnSendBack();
	bool CheckPipeBroke();

private:
    Connection(NetBase * base, const s32 fd, const s32 sendSize, const s32 recvSize);
    virtual ~Connection();

private:
	NetBase * _base;
	s32 _fd;
    core::ISession * _session;
    char _remoteIp[MAX_IP_SIZE];
    s32 _remotePort;
	s32 _threadId;

	olib::CircularBuffer _recvBuff;
	olib::CircularBuffer _sendBuff;
	s32 _totalRecvSize;
	s32 _totalSendSize;

	bool _closeing;
	bool _pushing;
	bool _broken;

	bool _pipeBroked;
	bool _sending;
};

#endif //__CONNECTION_H__
