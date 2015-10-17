#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "INetHandler.h"
#include "IKernel.h"
#include <unordered_map>

class Connection : public INetHandler, public core::IPipe {
	struct Buffer {
		char * data;
		s32 size;
		s32 offset;
	};

public:
    static Connection * Create(const s32 fd) {
        return NEW Connection(fd);
    }

    inline void SetSession(core::ISession * session) {
        _session = session;
        session->SetPipe(this);
    }
    void SetBufferSize(const s32 sendSize, const s32 recvSize);

    inline void SetLocalIp(const char * ip) { SafeSprintf(_localIp, sizeof(_localIp) - 1, ip); }
    virtual const char * GetLocalIp() const { return _localIp; }

    inline void SetLocalPort(const s32 port) { _localPort = port; }
    virtual s32 GetLocalPort() const { return _localPort; }

    inline void SetRemoteIp(const char * ip) { SafeSprintf(_remoteIp, sizeof(_remoteIp) - 1, ip); }
    virtual const char * GetRemoteIp() const { return _remoteIp; }

    inline void SetRemotePort(const s32 port) { _remotePort = port; }
    virtual s32 GetRemotePort() const { return _remotePort; }

    void OnConnected();

    virtual void OnIn();
    virtual void OnOut();
    virtual void OnError();

    virtual void Send(const void * context, const s32 size);
    virtual void Close();

private:
    Connection(const s32 fd);
    virtual ~Connection();

	void Expand(Buffer& buff);

	void OnSend();
	void OnError(s32 errCode);
	void OnClose();

private:
    core::ISession * _session;
    char _localIp[MAX_IP_SIZE];
    s32 _localPort;
    char _remoteIp[MAX_IP_SIZE];
    s32 _remotePort;

	bool _valid;
	bool _recving;

	Buffer _sendBuff;
	Buffer _recvBuff;
	bool _canSend;
};

#endif //__CONNECTION_H__
