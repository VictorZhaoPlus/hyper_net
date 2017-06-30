#ifndef __OBJECTTIMER_H__
#define __OBJECTTIMER_H__
#include "util.h"
#include "IObjectTimer.h"
#include "singleton.h"

using namespace object_timer;

class ObjectTimer : public IObjectTimer, public OHolder<ObjectTimer> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void Start(IObject * object, const IProp * prop, s64 delay, s32 count, s64 interval, const char * file, const s32 line, const StartFunc & start, const TickFunc & tick, const EndFunc & end);
	virtual void Stop(IObject * object, const IProp * prop);

private:
    IKernel * _kernel;
};

#endif //__OBJECTTIMER_H__

