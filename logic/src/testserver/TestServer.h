#ifndef __TESTSERVER_H__
#define __TESTSERVER_H__
#include "util.h"
#include "ITestServer.h"
#include "singleton.h"

class TestServer : public ITestServer, public ISessionFactory, public OHolder<TestServer> {
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

	inline const void * GetBuffer(s16 & size) const {
		const Buffer& buf = _buf[rand() % 1024];

		size = buf.size;
		return buf.data;
	}

private:
    IKernel * _kernel;

	Buffer _buf[1024];
};

#endif //__TESTSERVER_H__

