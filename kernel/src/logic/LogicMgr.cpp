#include "LogicMgr.h"
#include "IModule.h"
#include "kernel.h"
#include "tinyxml/tinyxml.h"
#include "ConfigMgr.h"
#include "tools.h"

bool LogicMgr::Ready() {
    return true;
}

bool LogicMgr::Initialize() {
    const char * name = ConfigMgr::Instance()->GetCmdArg("name");
    OASSERT(name, "invalid command args, there is no name");

    TiXmlDocument doc;
    std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/extend/" + name + ".xml";
    if (!doc.LoadFile(coreConfigPath.c_str())) {
        OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
        return false;
    }

    const TiXmlElement * root = doc.RootElement();
    OASSERT(root, "module.xml format error");
    const char * path = root->Attribute("path");
    OASSERT(path, "module.xml format error, can't find module path");

    const TiXmlElement * module = root->FirstChildElement("module");
    while (module) {
        const char * name = module->Attribute("name");
        OASSERT(name, "module.xml form error, can't find module's name");

        char fullPath[512];
#ifdef WIN32
		SafeSprintf(fullPath, sizeof(fullPath), "%s/%s%s.dll", tools::GetAppPath(), path, name);
#else
        SafeSprintf(fullPath, sizeof(fullPath), "%s/%slib%s.so", tools::GetAppPath(), path, name);
#endif
        if (!LoadModule(fullPath)) {
            OASSERT(false, "load module failed");
            return false;
        }

        module = module->NextSiblingElement("module");
    }

	for (auto * logic : _moduleList) {
		bool res = logic->Initialize(Kernel::Instance());
		OASSERT(res, "wtf");
		if (!res)
			return false;
	}

	for (auto * logic : _moduleList) {
		bool res = logic->Launched(Kernel::Instance());
		OASSERT(res, "wtf");
		if (!res)
			return false;
	}

    return true;
}

void LogicMgr::Loop() {
}

void LogicMgr::Destroy() {
    for (auto itr = _modules.begin(); itr != _modules.end(); ++itr)
        itr->second->Destroy(Kernel::Instance());
    DEL this;
}

bool LogicMgr::LoadModule(const char * path) {
#ifdef WIN32
	HINSTANCE inst = ::LoadLibrary(path);
	GetModuleFun fun = (GetModuleFun)::GetProcAddress(inst, NAME_OF_GET_LOGIC_FUN);
#else
    void * handle = dlopen(path, RTLD_LAZY);
    OASSERT(handle, "open %s error %s", path, dlerror());

    GetModuleFun fun = (GetModuleFun) dlsym(handle, NAME_OF_GET_LOGIC_FUN);
#endif
	OASSERT(fun, "get function:GetLogicModule error");

    IModule * logic = fun();
    OASSERT(logic, "can't get module from %s", path);

    while (logic) {
        const char * name = logic->GetName();
        auto itr = _modules.find(name);
        if (itr != _modules.end()) {
            OASSERT(false, "duplicate logic name %s", name);
            return false;
        }

		if (logic->GetKernelVersion() != KERNEL_VERSION) {
			OASSERT(false, "kernel version not match %s", name);
			return false;
		}

        _modules.insert(std::make_pair(name, logic));
		_moduleList.push_back(logic);

        logic = logic->GetNext();
    }
    return true;
}
