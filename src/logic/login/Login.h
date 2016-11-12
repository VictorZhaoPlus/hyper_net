#ifndef __LOGIN_H__
#define __LOGIN_H__
#include "util.h"
#include "IModule.h"
#include "singleton.h"

class Login : public IModule, public OHolder<Login> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

private:
    IKernel * _kernel;
};

#endif //__LOGIN_H__

