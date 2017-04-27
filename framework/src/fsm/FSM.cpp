#include "FSM.h"
#include "IObjectMgrExt.h"
#include "ObjectFsm.h"

bool FSM::Initialize(IKernel * kernel) {
    _kernel = kernel;

	FIND_MODULE_EXT(_objectMgr, IObjectMgrExt, ObjectMgr);
	_ext = _objectMgr->CalcProp("fsm");
	_status = _objectMgr->CalcProp("fsm");

	_objectMgr->RgsExt(_ext, sizeof(ObjectFsm));
	RGS_EXT_CREATOR(_objectMgr, _ext, FSM::CreateFsm);
	RGS_EXT_RECOVER(_objectMgr, _ext, FSM::RecoverFsm);

    return true;
}

bool FSM::Launched(IKernel * kernel) {
    return true;
}

bool FSM::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void FSM::RgsEntryJudgeCB(IObject * object, s32 status, const StatusJudgeCallback& f, const char * debug) {
	ObjectFsm * fsm = (ObjectFsm *)((IObjectExt*)object)->GetExtData(_ext, sizeof(ObjectFsm));
	OASSERT(fsm, "wtf");
	fsm->RgsEntryJudgeCB(status, f, debug);
}

void FSM::RgsChangeJudgeCB(IObject * object, s32 from, s32 to, const StatusJudgeCallback& f, const char * debug) {
	ObjectFsm * fsm = (ObjectFsm *)((IObjectExt*)object)->GetExtData(_ext, sizeof(ObjectFsm));
	OASSERT(fsm, "wtf");
	fsm->RgsChangeJudgeCB(from, to, f, debug);
}

void FSM::RgsLeaveJudgeCB(IObject * object, s32 status, const StatusJudgeCallback& f, const char * debug) {
	ObjectFsm * fsm = (ObjectFsm *)((IObjectExt*)object)->GetExtData(_ext, sizeof(ObjectFsm));
	OASSERT(fsm, "wtf");
	fsm->RgsLeaveJudgeCB(status, f, debug);
}


void FSM::RgsEntryCB(IObject * object, s32 status, const StatusCallback& f, const char * debug) {
	ObjectFsm * fsm = (ObjectFsm *)((IObjectExt*)object)->GetExtData(_ext, sizeof(ObjectFsm));
	OASSERT(fsm, "wtf");
	fsm->RgsEntryCB(status, f, debug);
}

void FSM::RgsChageCB(IObject * object, s32 from, s32 to, const StatusCallback& f, const char * debug) {
	ObjectFsm * fsm = (ObjectFsm *)((IObjectExt*)object)->GetExtData(_ext, sizeof(ObjectFsm));
	OASSERT(fsm, "wtf");
	fsm->RgsChageCB(from, to, f, debug);
}

void FSM::RgsLeaveCB(IObject * object, s32 status, const StatusCallback& f, const char * debug) {
	ObjectFsm * fsm = (ObjectFsm *)((IObjectExt*)object)->GetExtData(_ext, sizeof(ObjectFsm));
	OASSERT(fsm, "wtf");
	fsm->RgsLeaveCB(status, f, debug);
}


bool FSM::EntryStatus(IObject * object, s32 status, const void * context, const s32 size) {
	ObjectFsm * fsm = (ObjectFsm *)((IObjectExt*)object)->GetExtData(_ext, sizeof(ObjectFsm));
	OASSERT(fsm, "wtf");
	return fsm->EntryStatus(object, status, context, size);
}

void FSM::CreateFsm(IKernel * kernel, IObjectExt * object, void * data, const s32 size) {
	OASSERT(size == sizeof(ObjectFsm), "wtf");
	ObjectFsm * fsm = new(data) ObjectFsm();
}

void FSM::RecoverFsm(IKernel * kernel, IObjectExt * object, void * data, const s32 size) {
	OASSERT(size == sizeof(ObjectFsm), "wtf");
	ObjectFsm * fsm = (ObjectFsm *)data;
	fsm->~ObjectFsm();
}

