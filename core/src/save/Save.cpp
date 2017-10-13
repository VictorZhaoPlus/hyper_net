#include "Save.h"
#include "XmlReader.h"
#include "IObjectMgr.h"
#include "ICacheDB.h"

void Save::ObjectSavor::OnCreate(IKernel * kernel, IObject * object) {
	static s32 save = OMODULE(ObjectMgr)->CalcPropSetting("save");
	for (auto * prop : object->GetPropsInfo()) {
		if (prop->GetSetting(object) & save) {
			RGS_PROP_CHANGER(object, prop, ObjectSavor::Save);
		}
	}
}

void Save::ObjectSavor::OnRecover(IKernel * kernel, IObject * object) {
	SaveTime * timer = (SaveTime *)object->GetPropInt64(OMPROP("save", "saveTimer"));
	if (timer) {
		kernel->KillTimer(timer);

		OnSave(kernel, object);
	}
}

void Save::ObjectSavor::Save(IKernel * kernel, IObject * object, const char * name, const IProp * prop, const bool sync) {
	static s32 significant = OMODULE(ObjectMgr)->CalcPropSetting("significant");
	if (prop->GetSetting(object) & significant) {
		SaveTime * timer = (SaveTime *)object->GetPropInt64(OMPROP("save", "saveTimer"));
		if (timer)
			kernel->KillTimer(timer);

		OnSave(kernel, object);
	}
	else {
		SaveTime * timer = (SaveTime *)object->GetPropInt64(OMPROP("save", "saveTimer"));
		if (!timer) {
			timer = NEW SaveTime(object, this);
			START_TIMER(timer, 0, 1, _delay);

			object->SetPropInt64(OMPROP("save", "saveTimer"), (s64)timer);
		}
	}
}

void Save::ObjectSavor::OnSave(IKernel * kernel, IObject * object) {

}

void Save::SaveTime::OnTimer(IKernel * kernel, s32 beatCount, s64 tick) {
	_savor->OnSave(kernel, _object);
}

void Save::SaveTime::OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {
	_object->SetPropInt64(OMPROP("save", "saveTimer"), 0);

	DEL this;
}

bool Save::Initialize(IKernel * kernel) {
    _kernel = kernel;
    return true;
}

bool Save::Launched(IKernel * kernel) {
	olib::XmlReader reader;
	std::string coreConfigPath = std::string(tools::GetWorkPath()) + "/config/server_conf.xml";
	if (!reader.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
		return false;
	}

	if (reader.Root()["save"][0].IsExist("object")) {
		const olib::IXmlObject & saves = reader.Root()["save"][0]["object"];
		for (s32 i = 0; i < saves.Count(); ++i) {
			const char * type = saves[i].GetAttributeString("type");
			auto itr = _savors.find(type);
			if (itr == _savors.end()) {
				_savors[type] = NEW ObjectSavor(saves[i].GetAttributeInt32("interval"));
				OMODULE(ObjectMgr)->ExtendInt64(type, "saveTimer", "save");
			}
			else {
				OASSERT(false, "wtf");
				return false;
			}

			OMODULE(ObjectMgr)->RgsObjectCRCB(type,
				std::bind(&ObjectSavor::OnCreate, _savors[type], std::placeholders::_1, std::placeholders::_2),
				std::bind(&ObjectSavor::OnRecover, _savors[type], std::placeholders::_1, std::placeholders::_2)
			);
		}
	}
    return true;
}

bool Save::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

