#include "StartNodeTimer.h"
#include "Starter.h"

void StartNodeTimer::OnStart(IKernel * kernel, s64 tick) {
	Starter::OnNodeTimerStart(kernel, _type, tick);
}

void StartNodeTimer::OnTimer(IKernel * kernel, s64 tick) {
	Starter::OnNodeTimer(kernel, _type, tick);
}

void StartNodeTimer::OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {
	Starter::OnNodeTimerEnd(kernel, _type, tick);
}
