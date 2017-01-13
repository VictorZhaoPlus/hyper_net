#ifndef __SHADOWMGR_H__
#define __SHADOWMGR_H__
#include "util.h"
#include "IShadowMgr.h"
#include "singleton.h"

class ShadowMgr : public IShadowMgr, public OHolder<ShadowMgr> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

private:
    IKernel * _kernel;
};

#endif //__SHADOWMGR_H__

