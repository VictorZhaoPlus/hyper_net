#ifndef __NODESESSION_H__
#define __NODESESSION_H__
#include "util.h"
#include "IKernel.h"
using namespace core;

#define MAX_IP_SIZE 64

class NodeSession : public ISession, public ITimer {
public:
    NodeSession();
    virtual ~NodeSession();

    inline void SetConnect(const char * ip, const s32 port) {
        _connect = true;
		SafeSprintf(_ip, sizeof(_ip), ip);
        _port = port;
    }
	inline const char * GetConnectIp() const { return _ip; }
	inline s32 GetConnectPort() const { return _port; }

    virtual void OnConnected(IKernel * kernel);
    virtual s32 OnRecv(IKernel * kernel, const void * context, const s32 size);
    virtual void OnError(IKernel * kernel, const s32 error);
    virtual void OnDisconnected(IKernel * kernel);
    virtual void OnConnectFailed(IKernel * kernel);

	virtual void OnStart(IKernel * kernel, s64 tick) {}
	virtual void OnTimer(IKernel * kernel, s32 beatCount, s64 tick);
	virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {}

	virtual void OnPause(IKernel * kernel, s64 tick) {}
	virtual void OnResume(IKernel * kernel, s64 tick) {}

    bool PrepareSendNodeMessage(const s32 size);
    bool SendNodeMessage(const void * context, const s32 size);

	void DealPacket(IKernel * kernel, const void * context, const s32 size);

private:
    bool _ready;
    s32 _nodeType;
    s32 _nodeId;

    bool _connect;
    char _ip[MAX_IP_SIZE];
    s32 _port;
};

#endif //__NODESESSION_H__
