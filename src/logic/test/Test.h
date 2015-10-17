#ifndef __TEST_H__
#define __TEST_H__
#include "util.h"
#include "ITest.h"

class Test : public ITest, public ISession {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	static Test * Self() { return s_self; }

	virtual void OnConnected(IKernel * kernel);
	virtual s32 OnRecv(IKernel * kernel, const void * context, const s32 size);
	virtual void OnError(IKernel * kernel, const s32 error);
	virtual void OnDisconnected(IKernel * kernel);
	virtual void OnConnectFailed(IKernel * kernel);

private:
	static Test * s_self;
    static IKernel * s_kernel;
};

#endif //__TEST_H__

