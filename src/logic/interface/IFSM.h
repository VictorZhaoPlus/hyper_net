/*
 * File: IFSM.h
 * Author: ooeyusea
 *
 * Created On December 01, 2016, 11:38 AM
 */

#ifndef __IFSM_H__
#define __IFSM_H__
#include "IModule.h"

class IObject;
typedef std::function<void(IKernel *, IObject *, const void * context, const s32 size)> StatusCallback;
typedef std::function<bool(IKernel *, IObject *, const void * context, const s32 size)> StatusJudgeCallback;

class IFSM : public IModule {
public:
	virtual ~IFSM() {}

	virtual void RgsEntryJudgeCB(IObject * object, s32 status, const StatusJudgeCallback& f, const char * debug) = 0;
	virtual void RgsChangeJudgeCB(IObject * object, s32 from, s32 to, const StatusJudgeCallback& f, const char * debug) = 0;
	virtual void RgsLeaveJudgeCB(IObject * object, s32 status, const StatusJudgeCallback& f, const char * debug) = 0;

	virtual void RgsEntryCB(IObject * object, s32 status, const StatusCallback& f, const char * debug) = 0;
	virtual void RgsChageCB(IObject * object, s32 from, s32 to, const StatusCallback& f, const char * debug) = 0;
	virtual void RgsLeaveCB(IObject * object, s32 status, const StatusCallback& f, const char * debug) = 0;

	virtual bool EntryStatus(IObject * object, s32 status, const void * context, const s32 size) = 0;
};

#define RGS_ENTRY_JUDEG(fsm, obj, status, cb) fsm->RgsEntryJudegCB(obj, status, std::bind(&cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), #cb)
#define RGS_CHANGE_JUDEG(fsm, obj, from, to, cb) fsm->RgsChangeJudgeCB(obj, from, to, std::bind(&cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), #cb)
#define RGS_LEAVE_JUDEG(fsm, obj, status, cb) fsm->RgsLeaveJudegCB(obj, status, std::bind(&cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), #cb)

#define RGS_ENTRY_STATUS(fsm, obj, status, cb) fsm->RgsEntryCB(obj, status, std::bind(&cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), #cb)
#define RGS_CHANGE_STATUS(fsm, obj, from, to, cb) fsm->RgsChageCB(obj, from, to, std::bind(&cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), #cb)
#define RGS_LEAVE_STATUS(fsm, obj, status, cb) fsm->RgsLeaveCB(obj, status, std::bind(&cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), #cb)

#endif /*__IFSM_H__ */
