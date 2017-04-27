#ifndef __AGENTSESSION_H__
#define __AGENTSESSION_H__
#include "util.h"
#include "IKernel.h"
using namespace core;

class AgentSession : public ISession {
public:
	AgentSession();
	virtual ~AgentSession();

    virtual void OnConnected(IKernel * kernel);
    virtual s32 OnRecv(IKernel * kernel, const void * context, const s32 size);
    virtual void OnError(IKernel * kernel, const s32 error);
    virtual void OnDisconnected(IKernel * kernel);
	virtual void OnConnectFailed(IKernel * kernel) {}

private:
	s64 _id;
};

#endif //__AGENTSESSION_H__
