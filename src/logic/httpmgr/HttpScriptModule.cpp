#include "HttpScriptModule.h"
#include "tools.h"
#include <string>
#include "IScriptEngine.h"
#include "IHttpMgr.h"

class Request : public IHttpHandler {
public:
	static IHttpHandler * Create(const s64 sequenceId) {
		return NEW Request(sequenceId);
	}

	virtual void OnSuccess(IKernel * kernel, const void * context, const s32 size) {
		HttpScriptModule::OnSuccess(_sequenceId, context, size);
	}
	virtual void OnFail(IKernel * kernel, const s32 errCode) {
		HttpScriptModule::OnFailed(_sequenceId, errCode);
	}

	virtual void OnRelease() {
		DEL this;
	}

private:
	Request(const s64 sequenceId) : _sequenceId(sequenceId) {}
	virtual ~Request() {}

	s64 _sequenceId;
};

HttpScriptModule * HttpScriptModule::s_self = nullptr;
IKernel * HttpScriptModule::s_kernel = nullptr;
IScriptEngine * HttpScriptModule::s_scriptEngine = nullptr;
IHttpMgr * HttpScriptModule::s_httpMgr = nullptr;

IScriptModule * HttpScriptModule::s_module = nullptr;
std::unordered_map<s64, IHttpHandler*> HttpScriptModule::s_requests;

bool HttpScriptModule::Initialize(IKernel * kernel) {
    s_self = this;
    s_kernel = kernel;

	s_scriptEngine = (IScriptEngine*)kernel->FindModule("ScriptEngine");
	OASSERT(s_scriptEngine, "where is ScriptEngine");

	s_httpMgr = (IHttpMgr*)kernel->FindModule("HttpMgr");
	OASSERT(s_httpMgr, "where is HttpMgr");

    return true;
}

bool HttpScriptModule::Launched(IKernel * kernel) {
	s_module = s_scriptEngine->CreateModule("http");
	OASSERT(s_module, "create module failed");

	s_scriptEngine->AddModuleFunction(s_module, "get", HttpScriptModule::Get);
	s_scriptEngine->AddModuleFunction(s_module, "post", HttpScriptModule::Post);
	s_scriptEngine->AddModuleFunction(s_module, "stop", HttpScriptModule::Stop);

    return true;
}

bool HttpScriptModule::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void HttpScriptModule::Get(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer) {
	s64 threadId = 0;
	s64 sequenceId = 0;
	const char * uri = nullptr;
	if (!reader->Read("lls", &threadId, &sequenceId, &uri)) {
		OASSERT(false, "read uri failed");
		return;
	}
	OASSERT(s_requests.find(sequenceId) == s_requests.end(), "already has a request");

	IHttpHandler * request = Request::Create(sequenceId);
	OASSERT(request, "wtf");
	s_httpMgr->Get(threadId, request, uri);

	s_requests[sequenceId] = request;
}

void HttpScriptModule::Post(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer) {
	s64 threadId = 0;
	s64 sequenceId = 0;
	const char * url = nullptr;
	const char * param = nullptr;
	if (!reader->Read("llss", &threadId, &sequenceId, &url, &param)) {
		OASSERT(false, "read uri failed");
		return;
	}
	OASSERT(s_requests.find(sequenceId) == s_requests.end(), "already has a request");

	IHttpHandler * request = Request::Create(sequenceId);
	OASSERT(request, "wtf");
	s_httpMgr->Post(threadId, request, url, param);

	s_requests[sequenceId] = request;
}

void HttpScriptModule::Stop(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer) {
	s64 sequenceId = 0;
	if (!reader->Read("l", &sequenceId)) {
		OASSERT(false, "read uri failed");
		return;
	}

	auto itr = s_requests.find(sequenceId);
	if (itr == s_requests.end()) {
		OASSERT(false, "where is request???");
		return;
	}

	s_httpMgr->Stop(itr->second);
}

void HttpScriptModule::OnSuccess(const s64 sequenceId, const void * context, const s32 size) {
	s_scriptEngine->Call(s_module, "onSuccess", nullptr, "lS", sequenceId, context, size);

	OASSERT(s_requests.find(sequenceId) != s_requests.end(), "where is request???");
	s_requests.erase(sequenceId);
}

void HttpScriptModule::OnFailed(const s64 sequenceId, const s32 errCode) {
	s_scriptEngine->Call(s_module, "OnFailed", nullptr, "li", sequenceId, errCode);

	OASSERT(s_requests.find(sequenceId) != s_requests.end(), "where is request???");
	s_requests.erase(sequenceId);
}

