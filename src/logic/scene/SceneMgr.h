#ifndef __SCENEMGR_H__
#define __SCENEMGR_H__
#include "util.h"
#include "IModule.h"
#include "singleton.h"

class SceneMgr : public IModule, public OHolder<SceneMgr> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

private:
    IKernel * _kernel;
};

#endif //__SCENEMGR_H__

