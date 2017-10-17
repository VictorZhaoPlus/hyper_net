#include "Id.h"
#include "XmlReader.h"
#include "kernel.h"
#include "tools.h"

bool Id::Ready() {
    return true;
}

bool Id::Initialize() {
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

bool Id::Destroy() {
    DEL this;
    return true;
}
