#ifndef __TESTCLIENT_H__
#define __TESTCLIENT_H__
#include "util.h"
#include "ITestClient.h"
#include "singleton.h"

class TestClient : public ITestClient, public OHolder<TestClient> {
	struct Buffer {
		s16 size;
		void * data;
	};
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	inline const void * GetBuffer(s16 & size) const {
		const Buffer& buf = _buf[rand() % 1024];

		size = buf.size;
		return buf.data;
	}

private:
    IKernel * _kernel;

	Buffer _buf[1024];
};

#endif //__TESTCLIENT_H__

