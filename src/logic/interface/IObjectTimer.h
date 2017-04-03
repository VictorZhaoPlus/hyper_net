/*
 * File: IObjectTimer.h
 * Author: ooeyusea
 *
 * Created On November 15, 2016, 12:51 PM
 */

#ifndef __IOBJECTTIMER_H__
#define __IOBJECTTIMER_H__
 
#include "IModule.h"

class IObject;
class IProp;

namespace object_timer {
	typedef std::function<void(IKernel * kernel, IObject * object, s64 tick)> StartFunc;
	typedef std::function<void(IKernel * kernel, IObject * object, s32 beatCount, s64 tick)> TickFunc;
	typedef std::function<void(IKernel * kernel, IObject * object, bool nonviolent, s64 tick)> EndFunc;
}

class IObjectTimer : public IModule {
public:
	virtual ~IObjectTimer() {}

	virtual void Start(IObject * object, const IProp * prop, s64 delay, s32 count, s64 interval, const char * file, const s32 line, const object_timer::StartFunc & start, const object_timer::TickFunc & tick, const object_timer::EndFunc & end) = 0;
	virtual void Stop(IObject * object, const IProp * prop) = 0;
};

#endif /*__IOBJECTTIMER_H__ */
