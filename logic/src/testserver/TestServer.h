#ifndef __TESTSERVER_H__
#define __TESTSERVER_H__
#include "util.h"
#include "ITestServer.h"
#include "singleton.h"
#include <unordered_set>

class TestServer : public ITestServer, public ISessionFactory, public ITimer, public OHolder<TestServer> {
	struct Buffer {
		s16 size;
		void * data;
	};
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual ISession * Create();
	virtual void Recover(ISession *);

	virtual void OnStart(IKernel * kernel, s64 tick) {}
	virtual void OnTimer(IKernel * kernel, s32 beatCount, s64 tick);
	virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {}

	inline const void * GetBuffer(s16 & size) const {
		const Buffer& buf = _buf[rand() % 1024];

		size = buf.size;
		return buf.data;
	}

	inline void Active(ISession * session) { _actives.insert(session); }

private:
    IKernel * _kernel;

	Buffer _buf[1024];

	s32 _count;
	std::unordered_set<ISession *> _actives;
};

#endif //__TESTSERVER_H__

