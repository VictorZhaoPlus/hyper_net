#ifndef __LUACORE_H__
#define __LUACORE_H__
#include "util.h"
#include "IScriptEngine.h"
#include "singleton.h"
#ifndef LUAJIT
extern "C" {
    #include "lua/lua.h"
    #include "lua/lualib.h"
    #include "lua/lauxlib.h"
};
#else
#include "luajit/lua.hpp"
#endif
#include "IHarbor.h"
#include <functional>
#include <unordered_map>

typedef std::function<void (IKernel * kernel, lua_State * state)> ResultReader;
class ScriptEngine : public IScriptEngine, public OHolder<ScriptEngine>{
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual IScriptModule * CreateModule(const char * name);
	virtual bool AddModuleFunction(const IScriptModule * module, const char * func, const ScriptFuncType& f);

	virtual bool Call(const IScriptModule * module, const char * func, const ScriptResultReadFuncType& f, const char * format, ...);

	IKernel * GetKernel() const { return _kernel; }

private:
    bool LoadLua(const char * path, const char * logic);
    void AddSearchPath(const char* path);

	static s32 Callback(lua_State * state);

	bool PrepareCall(const IScriptModule * module, const char * func);
    bool ExecuteFunction(IKernel * kernel, int argc, const ResultReader& reader);
    bool ExecuteGlobalFunction(IKernel * kernel, const char * functionName, const ResultReader& reader);
    bool ExecuteString(IKernel * kernel, const char *codes, const ResultReader& reader);
    bool ExecuteScriptFile(IKernel * kernel, const char * module);

private:
    IKernel * _kernel;

    lua_State * _state;
};

#endif //__STARTER_H__

