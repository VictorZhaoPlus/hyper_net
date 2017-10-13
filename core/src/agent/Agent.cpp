#include "Agent.h"
#include "AgentSession.h"
#include "IScriptEngine.h"
#include "XmlReader.h"

bool Agent::Initialize(IKernel * kernel) {
	_kernel = kernel;

    return true;
}

bool Agent::Launched(IKernel * kernel) {
	olib::XmlReader reader;
	std::string coreConfigPath = std::string(tools::GetWorkPath()) + "/config/server_conf.xml";
	if (!reader.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
		return false;
	}

	const olib::IXmlObject& agent = reader.Root()["agent"][0];
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

	_listener->OnAgentOpen(_kernel, ret);
	return ret;
}

s32 Agent::OnRecv(const s64 id, const void * context, const s32 size) {
	OASSERT(s_sessions.find(id) != s_sessions.end(), "where is agent %lld", id);

	return _listener->OnAgentRecvPacket(_kernel, id, context, size);;
}

void Agent::OnClose(const s64 id) {
	OASSERT(s_sessions.find(id) != s_sessions.end(), "where is agent %lld", id);
	s_sessions.erase(id);

	_listener->OnAgentOpen(_kernel, id);
}

void Agent::Send(const s64 id, const void * context, const s32 size) {
	auto itr = s_sessions.find(id);
	if (itr == s_sessions.end()) {
		OASSERT(false, "where is agent %lld", id);
		return;
	}

	itr->second->Send(context, size);
}

void Agent::Kick(const s64 id) {
	auto itr = s_sessions.find(id);
	if (itr == s_sessions.end()) {
		OASSERT(false, "where is agent %lld", id);
		return;
	}

	itr->second->Close();
}
