#ifndef __@_H__
#define __@_H__
#include "util.h"
#include "I&.h"
#include "singleton.h"

class & : public I&, public OHolder<&> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

private:
    IKernel * _kernel;
};

#endif //__@_H__

