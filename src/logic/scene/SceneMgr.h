#ifndef __SCENEMGR_H__
#define __SCENEMGR_H__
#include "util.h"
#include "IScene.h"
#include "singleton.h"

class SceneMgr : public IScene, public OHolder<SceneMgr> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

private:
    IKernel * _kernel;
};

#endif //__SCENEMGR_H__

