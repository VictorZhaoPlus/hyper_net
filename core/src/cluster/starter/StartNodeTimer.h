#ifndef __STARTNODETIMER_H__
#define __STARTNODETIMER_H__
#include "IKernel.h"
using namespace core;

class StartNodeTimer : public core::ITimer {
public:
	static StartNodeTimer * Create(s32 type) { return NEW StartNodeTimer(type); }
	void Release() { DEL this; }

	virtual void OnStart(IKernel * kernel, s64 tick);
	virtual void OnTimer(IKernel * kernel, s32 beatCount, s64 tick);
	virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick);

	virtual void OnPause(IKernel * kernel, s64 tick) {}
	virtual void OnResume(IKernel * kernel, s64 tick) {}

private:
	StartNodeTimer(s32 type) : _type(type) {}
	virtual ~StartNodeTimer() {}

private:
	s32 _type;
};

#endif //__STARTNODETIMER_H__
