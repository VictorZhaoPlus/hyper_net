#include "AgentModule.h"
#include "IScriptEngine.h"
#include "FrameworkProtocol.h"

AgentModule * AgentModule::s_self = nullptr;
IKernel * AgentModule::s_kernel = nullptr;
IScriptEngine * AgentModule::s_scriptEngine = nullptr;
IAgent * AgentModule::s_agent = nullptr;

IScriptModule * AgentModule::s_module = nullptr;

bool AgentModule::Initialize(IKernel * kernel) {
    s_self = this;
	s_kernel = kernel;

	s_scriptEngine = (IScriptEngine*)kernel->FindModule("ScriptEngine");
	OASSERT(s_scriptEngine, "where is scriptEngine");

	s_agent = (IAgent*)kernel->FindModule("Agent");
	OASSERT(s_agent, "where is Agent");

	RGS_AGENT_LISTENER(s_agent, this);
    return true;
}

bool AgentModule::Launched(IKernel * kernel) {
	s_module = s_scriptEngine->CreateModule("agent");
	s_scriptEngine->AddModuleFunction(s_module, "send", AgentModule::Send);
	s_scriptEngine->AddModuleFunction(s_module, "kick", AgentModule::Kick);
    return true;
}

bool AgentModule::Destroy(IKernel * kernel) {
	if (s_module)
		s_module->Release();
    DEL this;
    return true;
}

void AgentModule::OnAgentOpen(IKernel * kernel, const s64 id) {
	s_scriptEngine->Call(s_module, "onOpen", nullptr, "l", id);
}

void AgentModule::OnAgentClose(IKernel * kernel, const s64 id) {
	s_scriptEngine->Call(s_module, "onClose", nullptr, "l", id);
}

s32 AgentModule::OnAgentRecvPacket(IKernel * kernel, const s64 id, const void * context, const s32 size) {
	s32 used = ON_RECV_FAILED;
	bool res = s_scriptEngine->Call(s_module, "onRecv", [&used](IKernel * kernel, IScriptCallResult * result) {
		result->Read("i", &used);
	}, "lPi", id, (const char*)context, size);

	OASSERT(res, "wtf");
	return used;
}


void AgentModule::Send(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer) {
	s64 id = 0;
	s32 size = 0;
	const void * context = nullptr;

	if (!reader->Read("lS", &id, &context, &size))
		return;

	s_agent->Send(id, context, size);
}

void AgentModule::Kick(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer) {
	s64 id;
	if (!reader->Read("l", &id))
		return;

	s_agent->Kick(id);
}