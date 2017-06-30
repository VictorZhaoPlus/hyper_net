#include "ObjectFsm.h"
#include "FSM.h"
#include "IObjectMgr.h"

ObjectFsm::ObjectFsm() {

}

ObjectFsm::~ObjectFsm() {

}

bool ObjectFsm::EntryStatus(IObject * object, s32 status, const void * context, const s32 size) {
	s8 oldStatus = object->GetPropInt8(OMPROP("fsm", "state"));
	if (oldStatus != status) {
		if (!_leaveJudge.Call(oldStatus, false, FSM::Instance()->GetKernel(), object, context, size)
			|| !_changeJudge.Call(((((s64)oldStatus) << 32) | (s64)status), false, FSM::Instance()->GetKernel(), object, context, size)
			|| !_entryJudge.Call(status, false, FSM::Instance()->GetKernel(), object, context, size)) {
			return false;
		}
		_leaveStatus.Call(oldStatus, FSM::Instance()->GetKernel(), object, context, size);
		object->SetPropInt8(OMPROP("fsm", "state"), status);
		_entryStatus.Call(((((s64)oldStatus) << 32) | (s64)status), FSM::Instance()->GetKernel(), object, context, size);
		_entryStatus.Call(oldStatus, FSM::Instance()->GetKernel(), object, context, size);
	}
	else {
		if (!_changeJudge.Call(((((s64)oldStatus) << 32) | (s64)status), false, FSM::Instance()->GetKernel(), object, context, size)
			|| !_entryJudge.Call(status, false, FSM::Instance()->GetKernel(), object, context, size)) {
			return false;
		}
		_entryStatus.Call(((((s64)oldStatus) << 32) | (s64)status), FSM::Instance()->GetKernel(), object, context, size);
		_entryStatus.Call(oldStatus, FSM::Instance()->GetKernel(), object, context, size);
	}

	return true;
}
