#ifndef __CONNECTION_H__
#define __CONNECTION_H__
#include "IKernel.h"
#include "NetLoop.h"

#define MAX_IP_SIZE 32
class Connection : public core::IPipe {
	enum {
		ERROR = -1,
		EMPTY,
		PENDING,
	};
	
public:
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

	inline Connection * GetPrev() const { return _prev; }
	inline void SetPrev(Connection * prev) { _prev = prev; }
	
	inline Connection * GetNext() const { return next; }
	inline void SetNext(Connection * next) { _next = next; }

	inline bool IsNeedSend() const { return _needSend; }
	inline void SetNeedSend(bool val) { _needSend = val; }
	
	virtual void Send(const void * context, const s32 size);
    virtual void Close();
	void InnerClose();

	void OnRecv(const s32 size);
	void Recv();

	void OnSendBack();
	s32 SendBack();

private:
    Connection(NetBase * base, const s32 fd, const s32 sendSize, const s32 recvSize);
    virtual ~Connection();

private:
	NetBase * _base;
	s32 _fd;
    core::ISession * _session;
    char _remoteIp[MAX_IP_SIZE];
    s32 _remotePort;

	bool _closeing;

	Connection * _prev;
	Connection * _next;
	bool _needSend;
};

#endif //__CONNECTION_H__
