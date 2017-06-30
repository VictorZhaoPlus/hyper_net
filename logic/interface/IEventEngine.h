/*
 * File: IEventEngine.h
 * Author: ooeyusea
 *
 * Created On November 15, 2016, 12:55 PM
 */

#ifndef __IEVENTENGINE_H__
#define __IEVENTENGINE_H__
 
#include "IModule.h"

typedef std::function<void(IKernel *, const void *, const s32)> EventCB;
typedef std::function<bool (IKernel *, const void *, const s32)> JudgeCB;

class IEventEngine : public IModule {
public:
	virtual ~IEventEngine() {}

	virtual void RgsEvent(const s32 eventId, const EventCB& cb, const char * debug) = 0;
	virtual void Exec(const s32 eventId, const void * context, const s32 size) = 0;

	virtual void RgsJudge(const s32 eventId, const JudgeCB& cb, const char * debug) = 0;
	virtual bool Judge(const s32 eventId, const void * context, const s32 size) = 0;
};

#define RGS_EVENT_HANDLER(eventId, cb) OMODULE(EventEngine)->RgsEvent(eventId, std::bind(&cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), #cb)
#define RGS_JUDGE_HANDLER(eventId, cb) OMODULE(EventEngine)->RgsJudge(eventId, std::bind(&cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), #cb)

#endif /*__IEVENTENGINE_H__ */
