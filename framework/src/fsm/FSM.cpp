#include "FSM.h"
#include "ObjectFsm.h"
#include "IObjectMgr.h"
#include "XmlReader.h"

bool FSM::Initialize(IKernel * kernel) {
    _kernel = kernel;
    return true;
}

bool FSM::Launched(IKernel * kernel) {
	olib::XmlReader reader;
	std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/server_conf.xml";
	if (!reader.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
		return false;
	}

	const olib::IXmlObject& units = reader.Root()["fsm"][0]["object"];
	for (s32 i = 0; i < units.Count(); ++i) {
		OMODULE(ObjectMgr)->ExtendInt8(units[i].GetAttributeString("name"), "fsm", "state");
		OMODULE(ObjectMgr)->ExtendStruct(units[i].GetAttributeString("name"), "fsm", "fsm", sizeof(ObjectFsm), [](void * p, s32 size) {
			new(p) ObjectFsm();
		}, [](void * p, s32 size) {
			ObjectFsm * fsm = (ObjectFsm *)p;
			fsm->~ObjectFsm();
		});
	}
    return true;
}

bool FSM::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void FSM::RgsEntryJudgeCB(IObject * object, s32 status, const StatusJudgeCallback& f, const char * debug) {
	ObjectFsm& fsm = object->GetPropT<ObjectFsm>(OMPROP("fsm", "fsm"));
	fsm.RgsEntryJudgeCB(status, f, debug);
}

void FSM::RgsChangeJudgeCB(IObject * object, s32 from, s32 to, const StatusJudgeCallback& f, const char * debug) {
	ObjectFsm& fsm = object->GetPropT<ObjectFsm>(OMPROP("fsm", "fsm"));
	fsm.RgsChangeJudgeCB(from, to, f, debug);
}

void FSM::RgsLeaveJudgeCB(IObject * object, s32 status, const StatusJudgeCallback& f, const char * debug) {
	ObjectFsm& fsm = object->GetPropT<ObjectFsm>(OMPROP("fsm", "fsm"));
	fsm.RgsLeaveJudgeCB(status, f, debug);
}

void FSM::RgsEntryCB(IObject * object, s32 status, const StatusCallback& f, const char * debug) {
	ObjectFsm& fsm = object->GetPropT<ObjectFsm>(OMPROP("fsm", "fsm"));
	fsm.RgsEntryCB(status, f, debug);
}

void FSM::RgsChageCB(IObject * object, s32 from, s32 to, const StatusCallback& f, const char * debug) {
	ObjectFsm& fsm = object->GetPropT<ObjectFsm>(OMPROP("fsm", "fsm"));
	fsm.RgsChageCB(from, to, f, debug);
}

void FSM::RgsLeaveCB(IObject * object, s32 status, const StatusCallback& f, const char * debug) {
	ObjectFsm& fsm = object->GetPropT<ObjectFsm>(OMPROP("fsm", "fsm"));
	fsm.RgsLeaveCB(status, f, debug);
}


bool FSM::EntryStatus(IObject * object, s32 status, const void * context, const s32 size) {
	ObjectFsm& fsm = object->GetPropT<ObjectFsm>(OMPROP("fsm", "fsm"));
	return fsm.EntryStatus(object, status, context, size);
}
