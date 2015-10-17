#include "Agent.h"
#include "tinyxml.h"
#include "AgentSession.h"
#include "IScriptEngine.h"
#include "XmlReader.h"

Agent * Agent::s_self = nullptr;
IKernel * Agent::s_kernel = nullptr;
IScriptEngine * Agent::s_scriptEngine = nullptr;

IScriptModule * Agent::s_module = nullptr;
s64 Agent::s_nextSessionId = 1;
std::unordered_map<s64, AgentSession*> Agent::s_sessions;

bool Agent::Initialize(IKernel * kernel) {
    s_self = this;
	s_kernel = kernel;

	s_scriptEngine = (IScriptEngine*)kernel->FindModule("ScriptEngine");
	OASSERT(s_scriptEngine, "where is scriptEngine");
    return true;
}

bool Agent::Launched(IKernel * kernel) {
	s_module = s_scriptEngine->CreateModule("agent");
	s_scriptEngine->AddModuleFunction(s_module, "send", Agent::Send);
	s_scriptEngine->AddModuleFunction(s_module, "kick", Agent::Kick);

	olib::XmlReader reader;
	std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/server_conf.xml";
	if (!reader.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
		return false;
	}

	olib::IXmlObject& agent = reader.Root()["agent"][0];
	s32 sendBuffSize = agent.GetAttributeInt32("send");
	s32 recvBuffSize = agent.GetAttributeInt32("recv");
	s32 port = tools::StringAsInt(kernel->GetCmdArg("agent"));

	if (!kernel->Listen("0.0.0.0", port, sendBuffSize, recvBuffSize, this)) {
		OASSERT(false, "listen failed");
		return false;
	}

    return true;
}

bool Agent::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

ISession * Agent::Create() {
	return NEW AgentSession;
}

void Agent::Recover(ISession * session) {
	DEL session;
}

s64 Agent::OnOpen(AgentSession * session) {
	if (s_nextSessionId < 0)
		s_nextSessionId = 1;

	s64 ret = s_nextSessionId++;
	s_sessions[ret] = session;

	s_scriptEngine->Call(s_module, "onOpen", nullptr, "l", ret);
	return ret;
}

s32 Agent::OnRecv(const s64 id, const void * context, const s32 size) {
	OASSERT(s_sessions.find(id) != s_sessions.end(), "where is agent %d", id);

	s32 used = ON_RECV_FAILED;
	bool res = s_scriptEngine->Call(s_module, "onRecv", [&used](IKernel * kernel, IScriptCallResult * result) {
		result->Read("i", &used);
	}, "lPi", id, context, size);

	OASSERT(res, "wtf");
	return used;
}

void Agent::OnClose(const s64 id) {
	OASSERT(s_sessions.find(id) != s_sessions.end(), "where is agent %d", id);
	s_sessions.erase(id);

	s_scriptEngine->Call(s_module, "onClose", nullptr, "l", id);
}

void Agent::Send(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer) {
	s64 id = 0;
	s32 size = 0;
	const void * context = nullptr;

	if (!reader->Read("lS", &id, &context, &size))
		return;

	auto itr = s_sessions.find(id);
	if (itr == s_sessions.end()) {
		OASSERT(false, "where is agent %d", id);
		return;
	}

	itr->second->Send(context, size);
}

void Agent::Kick(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer) {
	s64 id;
	if (!reader->Read("l", &id))
		return;

	auto itr = s_sessions.find(id);
	if (itr == s_sessions.end()) {
		OASSERT(false, "where is agent %d", id);
		return;
	}

	itr->second->Close();
}
