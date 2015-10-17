#include "Http.h"
#include "IScriptEngine.h"
#include "XmlReader.h"

void Request::OnSuccess(IKernel * kernel, const void * context, const s32 size) {
	Http::OnSuccess(_sequenceId, context, size);
}

void Request::OnFail(IKernel * kernel, const s32 errCode) {
	Http::OnFailed(_sequenceId, errCode);
}

Http * Http::s_self = nullptr;
IKernel * Http::s_kernel = nullptr;
IScriptEngine * Http::s_scriptEngine = nullptr;

IScriptModule * Http::s_module = nullptr;
std::unordered_map<s64, IHttpHandler*> Http::s_requests;

bool Http::Initialize(IKernel * kernel) {
    s_self = this;
    s_kernel = kernel;

	s_scriptEngine = (IScriptEngine*)kernel->FindModule("ScriptEngine");
	OASSERT(s_scriptEngine, "where is scriptEngine");

    return true;
}

bool Http::Launched(IKernel * kernel) {
	s_module = s_scriptEngine->CreateModule("http");
	OASSERT(s_module, "create module failed");
	s_scriptEngine->AddModuleFunction(s_module, "get", Http::Get);
	s_scriptEngine->AddModuleFunction(s_module, "post", Http::Post);
	s_scriptEngine->AddModuleFunction(s_module, "stop", Http::Stop);

    return true;
}

bool Http::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void Http::Get(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer) {
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
	kernel->Get(threadId, request, uri);

	s_requests[sequenceId] = request;
}

void Http::Post(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer) {
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
	kernel->Post(threadId, request, url, param);

	s_requests[sequenceId] = request;
}

void Http::Stop(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer) {
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

	kernel->Stop(itr->second);
}

void Http::OnSuccess(const s64 sequenceId, const void * context, const s32 size) {
	s_scriptEngine->Call(s_module, "onSuccess", nullptr, "lS", sequenceId, context, size);

	OASSERT(s_requests.find(sequenceId) != s_requests.end(), "where is request???");
	s_requests.erase(sequenceId);
}

void Http::OnFailed(const s64 sequenceId, const s32 errCode) {
	s_scriptEngine->Call(s_module, "OnFailed", nullptr, "li", sequenceId, errCode);

	OASSERT(s_requests.find(sequenceId) != s_requests.end(), "where is request???");
	s_requests.erase(sequenceId);
}
