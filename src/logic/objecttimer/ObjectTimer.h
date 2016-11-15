#ifndef __OBJECTTIMER_H__
#define __OBJECTTIMER_H__
#include "util.h"
#include "IObjectTimer.h"
#include "singleton.h"

class ObjectTimer : public IObjectTimer, public OHolder<ObjectTimer> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

private:
    IKernel * _kernel;
};

#endif //__OBJECTTIMER_H__

