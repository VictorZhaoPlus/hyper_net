#ifndef __@_H__
#define __@_H__
#include "util.h"
#include "I&.h"

class & : public I& {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	static & * Self() { return s_self; }

private:
	static & * s_self;
    static IKernel * s_kernel;
};

#endif //__@_H__

