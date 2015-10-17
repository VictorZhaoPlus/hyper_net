#ifndef __HTTP_H__
#define __HTTP_H__
#include "util.h"
#include "IModule.h"
using namespace core;
#include <unordered_map>

class Request : public IHttpHandler {
public:
	static IHttpHandler * Create(const s64 sequenceId) {
		return NEW Request(sequenceId);
	}

	virtual void OnSuccess(IKernel * kernel, const void * context, const s32 size);
	virtual void OnFail(IKernel * kernel, const s32 errCode);
	virtual void OnRelease() {
		DEL this;
	}

private:
	Request(const s64 sequenceId) : _sequenceId(sequenceId) {}
	virtual ~Request() {}

	s64 _sequenceId;
};

class IScriptEngine;
class IScriptModule;
class IScriptArgumentReader;
class IScriptResultWriter;
class Http : public IModule {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	static Http * Self() { return s_self; }

	static void Get(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer);
	static void Post(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer);
	static void Stop(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer);

	static void OnSuccess(const s64 sequenceId, const void * context, const s32 size);
	static void OnFailed(const s64 sequenceId, const s32 errCode);

private:
	static Http * s_self;
    static IKernel * s_kernel;
	static IScriptEngine * s_scriptEngine;

	static IScriptModule * s_module;

	static std::unordered_map<s64, IHttpHandler*> s_requests;
};

#endif //__HTTP_H__

