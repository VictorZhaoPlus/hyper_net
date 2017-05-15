#ifndef __CONNECTION_H__
#define __CONNECTION_H__
#include "IKernel.h"
#include "ORingBuffer.h"
#include "Iocp.h"

#define MAX_IP_SIZE 32
class Connection : public core::IPipe {
public:
    static Connection * Create(const SOCKET fd, const s32 sendSize, const s32 recvSize) {
        return NEW Connection(fd, sendSize, recvSize);
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

	virtual s32 GetLocalPort() const { return 0; }

	virtual void AdjustSendBuffSize(u32 size) {}
	virtual void AdjustRecvBuffSize(u32 size) {}

    virtual void Send(const void * context, const s32 size);
    virtual void Close();
	void CloseForce();
	void CloseThread();

	bool OnConnected();
	void OnConnectFailed();
	bool DoConnect(const char * ip, const s32 port);
	
	bool In(const char * buf, const s32 size);
	void OnRecv();
	bool OnRecvDone();
	bool DoRecv();

	void OnSend();
	bool OnSendDone();
	bool Out(const s32 size);
	bool DoSend();

	bool IsClosing() { return _closing; }

private:
    Connection(const SOCKET fd, const s32 sendSize, const s32 recvSize);
    virtual ~Connection();

private:
	SOCKET _fd;
    core::ISession * _session;
    char _remoteIp[MAX_IP_SIZE];
    s32 _remotePort;

	RingBuffer * _sendBuf;
	RingBuffer * _recvBuf;

#ifdef _DEBUG
	bool _establish;
#endif
	bool _closing;
	bool _sending;
	bool _recving;
	bool _isSocketClosed;

	IocpEvent _connect;
	IocpEvent _recv;
	IocpEvent _send;

	sockaddr_in _addr;
};

#endif //__CONNECTION_H__
