#include "ObjectFsm.h"
#include "FSM.h"
#include "IObjectMgr.h"

ObjectFsm::ObjectFsm() : _status(0) {

}

ObjectFsm::~ObjectFsm() {

}

bool ObjectFsm::EntryStatus(IObject * object, s32 status, const void * context, const s32 size) {
	if (_status != status) {
		if (!_leaveJudge.Call(_status, false, FSM::Instance()->GetKernel(), object, context, size)
			|| !_changeJudge.Call(((((s64)_status) << 32) | (s64)status), false, FSM::Instance()->GetKernel(), object, context, size)
			|| !_entryJudge.Call(status, false, FSM::Instance()->GetKernel(), object, context, size)) {
			return false;
		}
		_leaveStatus.Call(_status, FSM::Instance()->GetKernel(), object, context, size);
		_status = status;
		object->SetPropInt32(FSM::Instance()->GetPropStatus(), status);
		_entryStatus.Call(((((s64)_status) << 32) | (s64)status), FSM::Instance()->GetKernel(), object, context, size);
		_entryStatus.Call(_status, FSM::Instance()->GetKernel(), object, context, size);
	}
	else {
		if (!_changeJudge.Call(((((s64)_status) << 32) | (s64)status), false, FSM::Instance()->GetKernel(), object, context, size) 
			|| !_entryJudge.Call(status, false, FSM::Instance()->GetKernel(), object, context, size)) {
			return false;
		}
		_entryStatus.Call(((((s64)_status) << 32) | (s64)status), FSM::Instance()->GetKernel(), object, context, size);
		_entryStatus.Call(_status, FSM::Instance()->GetKernel(), object, context, size);
	}

	return true;
}
