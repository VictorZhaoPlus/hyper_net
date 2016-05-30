#include "ConfigMgr.h"
#include "tools.h"
#include "tinyxml.h"
#include "XmlReader.h"

ConfigMgr::ConfigMgr() {
    //ctor
}

ConfigMgr::~ConfigMgr() {
    //dtor
}


bool ConfigMgr::Ready() {
    return true;
}

bool ConfigMgr::Initialize(int argc, char ** argv) {
    if (!parse(argc, argv))
        return false;

    const char * name = GetCmdArg("name");
    OASSERT(name, "invalid command args, there is no name");

    std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/server_conf.xml";
	olib::XmlReader conf;
	if (!conf.LoadXml(coreConfigPath.c_str())) {
        OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
        return false;
    }

	_maxOpenFile = conf.Root()["limit"][0].GetAttributeInt32("open_file");
	_stackSize = conf.Root()["limit"][0].GetAttributeInt32("stack");

	_frameDuration = conf.Root()["extend"][0][name][0]["loop"][0].GetAttributeInt32("tick");

    _netFrameTick = conf.Root()["extend"][0][name][0]["net"][0].GetAttributeInt32("tick");
    _netFrameWaitTick = conf.Root()["extend"][0][name][0]["net"][0].GetAttributeInt32("wait");
    _netSupportSize = conf.Root()["extend"][0][name][0]["net"][0].GetAttributeInt32("support");
	_netThreadCount = conf.Root()["extend"][0][name][0]["net"][0].GetAttributeInt32("thread");

	SafeSprintf(_loggerPath, sizeof(_loggerPath), conf.Root()["logger"][0].GetAttributeString("path"));
	_logToConsole = conf.Root()["logger"][0].GetAttributeInt32("console");

	_asyncThreadCount = conf.Root()["extend"][0][name][0]["async"][0].GetAttributeInt32("thread");
	_asyncTick = conf.Root()["extend"][0][name][0]["async"][0].GetAttributeInt32("tick");
    return true;
}

void ConfigMgr::Destroy() {
    DEL this;
}

const char * ConfigMgr::GetCmdArg(const char * name) {
    auto itr = _args.find(name);
    if (itr != _args.end()) {
        return itr->second.c_str();
    }
    return nullptr;
}

bool ConfigMgr::parse(int argc, char ** argv) {
    for (s32 i = 1; i < argc; ++i) {
        OASSERT(strncmp(argv[i], "--", 2) == 0, "invalid argv %s", argv[i]);

        char * start = argv[i] + 2;
        char * equal = strstr(start, "=");
        OASSERT(equal, "invalid argv %s", argv[i]);
        std::string name(start, equal);
        std::string val(equal + 1);
        _args[name] = val;
    }
    return true;
}
