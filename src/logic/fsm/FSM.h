#ifndef __FSM_H__
#define __FSM_H__
#include "util.h"
#include "IFSM.h"
#include "singleton.h"

class IObjectMgrExt;
class IObjectExt;
class FSM : public IFSM, public OHolder<FSM> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void RgsEntryJudgeCB(IObject * object, s32 status, const StatusJudgeCallback& f, const char * debug);
	virtual void RgsChangeJudgeCB(IObject * object, s32 from, s32 to, const StatusJudgeCallback& f, const char * debug);
	virtual void RgsLeaveJudgeCB(IObject * object, s32 status, const StatusJudgeCallback& f, const char * debug);

	virtual void RgsEntryCB(IObject * object, s32 status, const StatusCallback& f, const char * debug);
	virtual void RgsChageCB(IObject * object, s32 from, s32 to, const StatusCallback& f, const char * debug);
	virtual void RgsLeaveCB(IObject * object, s32 status, const StatusCallback& f, const char * debug);

	virtual bool EntryStatus(IObject * object, s32 status, const void * context, const s32 size);

	void CreateFsm(IKernel * kernel, IObjectExt * object, void * data, const s32 size);
	void RecoverFsm(IKernel * kernel, IObjectExt * object, void * data, const s32 size);

	IKernel * GetKernel() { return _kernel; }
	s32 GetPropStatus() { return _status; }

private:
    IKernel * _kernel;
	IObjectMgrExt * _objectMgr;

	s32 _ext;
	s32 _status;
};

#endif //__FSM_H__

