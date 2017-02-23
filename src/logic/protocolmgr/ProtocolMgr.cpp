#include "ProtocolMgr.h"
#include "XmlReader.h"

bool ProtocolMgr::Initialize(IKernel * kernel) {
    _kernel = kernel;

	olib::XmlReader reader;
	std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/protos.xml";
	if (!reader.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
		return false;
	}

	reader.Root().ForEach([this](const char * name, const olib::IXmlObject& objects) {
		for (s32 i = 0; i < objects.Count(); ++i) {
			_protos[name][objects[i].GetAttributeString("name")] = objects[i].GetAttributeInt32("id");
			_descs[name][objects[i].GetAttributeInt32("id")] = objects[i].GetAttributeString("desc");
		}
	});
    return true;
}

bool ProtocolMgr::Launched(IKernel * kernel) {
    return true;
}

bool ProtocolMgr::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

s32 ProtocolMgr::GetId(const char * group, const char * name) {
	auto itr = _protos[group].find(name);
	OASSERT(itr != _protos[group].end(), "wtf");
	if (itr != _protos[group].end())
		return itr->second;
	return 0;
}

const char * ProtocolMgr::GetDesc(const char * group, const s32 id) {
	auto itr = _descs[group].find(id);
	OASSERT(itr != _descs[group].end(), "wtf");
	if (itr != _descs[group].end())
		return itr->second.c_str();
	return "";
}
