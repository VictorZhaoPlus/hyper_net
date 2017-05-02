#include "Profile.h"
#include "kernel.h"
#include "ConfigMgr.h"
#include "XmlReader.h"
#include "tools.h"
#include "Logger.h"

void WirteDump(const char * buf) {
	printf("%s\n", buf);
}

bool Profile::Ready() {
    return true;
}

bool Profile::Initialize() {
	_tick = tools::GetTimeMillisecond();

	const char * name = ConfigMgr::Instance()->GetCmdArg("name");
	OASSERT(name, "invalid command args, there is no name");

	std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/server_conf.xml";
	olib::XmlReader conf;
	if (!conf.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
		return false;
	}

	if (conf.Root()["extend"][0][name][0].IsExist("mem_profile")) {
		_open = conf.Root()["extend"][0][name][0]["mem_profile"][0].GetAttributeBoolean("enable");
		_interval = conf.Root()["extend"][0][name][0]["mem_profile"][0].GetAttributeInt32("interval");
	}
	else {
		_open = false;
	}

    return true;
}

void Profile::Loop() {
	if (_open) {
		s64 now = tools::GetTimeMillisecond();
		if (now - _tick >= _interval) {
			_tick = now;
		}
	}
}

void Profile::Destroy() {
    DEL this;
}
