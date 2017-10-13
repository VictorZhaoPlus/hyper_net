#include "ProtocolMgr.h"
#include "XmlReader.h"
#include "tools.h"

bool ProtocolMgr::Initialize(IKernel * kernel) {
    _kernel = kernel;

	std::string coreConfigPath = std::string(tools::GetWorkPath()) + "/config/proto";
	tools::ListFileInDirection(coreConfigPath.c_str(), ".xml", [this](const char * name, const char * path) {
		olib::XmlReader reader;
		if (!reader.LoadXml(path)) {
			OASSERT(false, "can't find core file : %s", path);
			return false;
		}

		reader.Root().ForEach([this](const char * name, const olib::IXmlObject& objects) {
			for (s32 i = 0; i < objects.Count(); ++i) {
				if (objects[i].HasAttribute("id"))
					_protos[name][objects[i].GetAttributeString("name")] = objects[i].GetAttributeInt32("id");
				else
					_protos[name][objects[i].GetAttributeString("name")] = (++_nextProtos[objects[i].GetAttributeString("type")]);
			}
		});
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
	OASSERT(itr != _protos[group].end(), "where is %s/%s", group, name);
	if (itr != _protos[group].end())
		return itr->second;
	return 0;
}
