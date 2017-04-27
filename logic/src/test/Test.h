#ifndef __TEST_H__
#define __TEST_H__
#include "util.h"
#include "ITest.h"

class IMapReduce;
class Test : public ITest {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	static Test * Self() { return s_self; }

private:
	static Test * s_self;
    static IKernel * s_kernel;
	IMapReduce * _mapReduce;
};

#endif //__TEST_H__

