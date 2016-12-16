#ifndef __OBJECTFSM_H__
#define __OBJECTFSM_H__
#include <unordered_map>
#include "IFSM.h"
#include "OCallback.h"

class ObjectFsm {
	typedef olib::CallbackType<s32, StatusCallback>::type STATUS_CB_POOL;
	typedef olib::CallbackType<s64, StatusCallback>::type STATUS_CHANGECB_POOL;
	typedef olib::CallbackType<s32, StatusJudgeCallback>::type STATUS_JUDEGCB_POOL;
	typedef olib::CallbackType<s64, StatusJudgeCallback>::type STATUS_JUDEGCHANGECB_POOL;
public:
	ObjectFsm();
	~ObjectFsm();

	void RgsEntryJudgeCB(s32 status, const StatusJudgeCallback& f, const char * debug) { _entryJudge.Register(status, f, debug); }
	void RgsChangeJudgeCB(s32 from, s32 to, const StatusJudgeCallback& f, const char * debug) { _changeJudge.Register(((((s64)from) << 32) | (s64)to), f, debug); }
	void RgsLeaveJudgeCB(s32 status, const StatusJudgeCallback& f, const char * debug) { _leaveJudge.Register(status, f, debug); }
	
	void RgsEntryCB(s32 status, const StatusCallback& f, const char * debug) { _entryStatus.Register(status, f, debug); }
	void RgsChageCB(s32 from, s32 to, const StatusCallback& f, const char * debug) { _changeStatus.Register(((((s64)from) << 32) | (s64)to), f, debug); }
	void RgsLeaveCB(s32 status, const StatusCallback& f, const char * debug) { _leaveStatus.Register(status, f, debug); }
	
	bool EntryStatus(IObject * object, s32 status, const void * context, const s32 size);

private:
	s32 _status;

	STATUS_JUDEGCB_POOL _entryJudge;
	STATUS_JUDEGCHANGECB_POOL _changeJudge;
	STATUS_JUDEGCB_POOL _leaveJudge;

	STATUS_CB_POOL _entryStatus;
	STATUS_CHANGECB_POOL _changeStatus;
	STATUS_CB_POOL _leaveStatus;
};

#endif //__OBJECTFSM_H__