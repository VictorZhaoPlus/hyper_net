#ifndef __EVENTENGINE_H__
#define __EVENTENGINE_H__
#include "util.h"
#include "IEventEngine.h"
#include "singleton.h"
#include "OCallback.h"

class EventEngine : public IEventEngine, public OHolder<EventEngine> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void RgsEvent(const s32 eventId, const EventCB& cb, const char * debug);
	virtual void Exec(const s32 eventId, const void * context, const s32 size);

	virtual void RgsJudge(const s32 eventId, const JudgeCB& cb, const char * debug);
	virtual bool Judge(const s32 eventId, const void * context, const s32 size);

private:
    IKernel * _kernel;

	olib::CallbackType<s32, EventCB>::type _events;
	olib::CallbackType<s32, JudgeCB>::type _judges;
};

#endif //__EVENTENGINE_H__

