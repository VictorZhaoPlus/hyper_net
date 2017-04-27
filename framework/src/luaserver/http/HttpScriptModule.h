#ifndef __HTTPSCRIPTMODULE_H__
#define __HTTPSCRIPTMODULE_H__
#include "IModule.h"
#include <unordered_map>

class IHttpMgr;
class IHttpHandler;
class IScriptEngine;
class IScriptModule;
class IScriptArgumentReader;
class IScriptResultWriter;
class HttpScriptModule : public IModule {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	static HttpScriptModule * Self() { return s_self; }

	static void Get(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer);
	static void Post(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer);
	static void Stop(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer);

	static void OnSuccess(const s64 sequenceId, const void * context, const s32 size);
	static void OnFailed(const s64 sequenceId, const s32 errCode);

private:
	static HttpScriptModule * s_self;
    static IKernel * s_kernel;
	static IScriptEngine * s_scriptEngine;
	static IHttpMgr * s_httpMgr;

	static IScriptModule * s_module;

	static std::unordered_map<s64, IHttpHandler*> s_requests;
};

#endif //__HTTPSCRIPTMODULE_H__

