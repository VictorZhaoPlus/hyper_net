#include "StartNodeTimer.h"
#include "Starter.h"

void StartNodeTimer::OnStart(IKernel * kernel, s64 tick) {
	Starter::Instance()->OnNodeTimerStart(kernel, _type, tick);
}

void StartNodeTimer::OnTimer(IKernel * kernel, s32 beatCount, s64 tick) {
	Starter::Instance()->OnNodeTimer(kernel, _type, tick);
}

void StartNodeTimer::OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {
	Starter::Instance()->OnNodeTimerEnd(kernel, _type, tick);
}
