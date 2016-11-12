#ifndef __DISTRIBUTION_H__
#define __DISTRIBUTION_H__
#include "util.h"
#include "IModule.h"
#include "singleton.h"

class Distribution : public IModule, public OHolder<Distribution> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

private:
    IKernel * _kernel;
};

#endif //__DISTRIBUTION_H__

