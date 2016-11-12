#ifndef __PLAYERMGR_H__
#define __PLAYERMGR_H__
#include "util.h"
#include "ILogin.h"
#include "singleton.h"

class PlayerMgr : public IModule, public OHolder<PlayerMgr> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

private:
    IKernel * _kernel;
};

#endif //__PLAYERMGR_H__

