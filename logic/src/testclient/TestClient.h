#ifndef __TESTCLIENT_H__
#define __TESTCLIENT_H__
#include "util.h"
#include "ITestClient.h"
#include "singleton.h"
#include <string>

class TestClient : public ITestClient, public ITimer, public OHolder<TestClient> {
	struct Buffer {
		s16 size;
		void * data;
	};
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void OnStart(IKernel * kernel, s64 tick) {}
	virtual void OnTimer(IKernel * kernel, s32 beatCount, s64 tick);
	virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {}

	inline const void * GetBuffer(s16 & size) const {
		const Buffer& buf = _buf[rand() % 1024];

		size = buf.size;
		return buf.data;
	}

private:
    IKernel * _kernel;
	std::string _ip;
	s32 _port;

	Buffer _buf[1024];
	s32 _count;
};

#endif //__TESTCLIENT_H__

