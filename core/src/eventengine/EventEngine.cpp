#include "EventEngine.h"

bool EventEngine::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool EventEngine::Launched(IKernel * kernel) {
    return true;
}

bool EventEngine::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void EventEngine::RgsEvent(const s32 eventId, const EventCB& cb, const char * debug) {
	_events.Register(eventId, cb, debug);
}

void EventEngine::Exec(const s32 eventId, const void * context, const s32 size) {
	_events.Call(eventId, _kernel, context, size);
}


void EventEngine::RgsJudge(const s32 eventId, const JudgeCB& cb, const char * debug) {
	_judges.Register(eventId, cb, debug);
}

bool EventEngine::Judge(const s32 eventId, const void * context, const s32 size) {
	return _judges.Call(eventId, false, _kernel, context, size);
}
