#ifndef __CONNECTION_H__
#define __CONNECTION_H__
#include "IKernel.h"
#include "ORingBuffer.h"

#define MAX_IP_SIZE 32
class NetWorker;
class Connection : public core::IPipe {
	enum {
		ST_NORMAL = 0,
		ST_CLOSING,
		ST_ERROR,
		ST_CLOSED,
	};
	enum {
		ERROR = -1,
		EMPTY,
		PENDING,
	};
	
public:
    static Connection * Create(const s32 fd, const s32 sendSize, const s32 recvSize) {
        return NEW Connection(fd, sendSize, recvSize);
    }

	void OnRelease() { DEL this; }

    inline void SetSession(core::ISession * session) {
        _session = session;
        session->SetPipe(this);
    }
	
	inline void SetWorker(NetWorker * worker) { _worker = worker; }
	inline s32 GetFd() const { return _fd; }

    inline void SetRemoteIp(const char * ip) { SafeSprintf(_remoteIp, sizeof(_remoteIp) - 1, ip); }
    virtual const char * GetRemoteIp() const { return _remoteIp; }

    inline void SetRemotePort(const s32 port) { _remotePort = port; }
    virtual s32 GetRemotePort() const { return _remotePort; }
	
	virtual void Send(const void * context, const s32 size);
    virtual void Close();

	void OnRecv();
	void OnDone();
	
	bool ThreadRecv();
	void ThreadSend(bool inThread);
	void ThreadClose(bool force);

private:
    Connection(const s32 fd, const s32 sendSize, const s32 recvSize);
    virtual ~Connection();

private:
	s32 _fd;
    core::ISession * _session;
	NetWorker * _worker;
    char _remoteIp[MAX_IP_SIZE];
    s32 _remotePort;
	
	RingBuffer * _sendBuff;
	RingBuffer * _recvBuff;

	bool _closing;
	s8 _threadStatus;
	bool _pending;
};

#endif //__CONNECTION_H__
