#include "Redis.h"
#include "XmlReader.h"
#include <string>
#include "RedisSession.h"
#include "IScriptEngine.h"

Redis * Redis::s_self = nullptr;
IKernel * Redis::s_kernel = nullptr;
IScriptEngine * Redis::s_scriptEngine = nullptr;

IScriptModule * Redis::s_module = nullptr;
s32 Redis::s_sendBufferSize = 0;
s32 Redis::s_recvBufferSize = 0;
s32 Redis::s_reconnectTick = 0;

std::vector<RedisSession*> Redis::s_sessions;

bool Redis::Initialize(IKernel * kernel) {
    s_self = this;
    s_kernel = kernel;

	olib::XmlReader reader;
	std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/server_conf.xml";
	if (!reader.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
		return false;
	}

	s_sendBufferSize = reader.Root()["redis"][0].GetAttributeInt32("send");
	s_recvBufferSize = reader.Root()["redis"][0].GetAttributeInt32("recv");
	s_reconnectTick = reader.Root()["redis"][0].GetAttributeInt32("reconnect");

	const olib::IXmlObject& units = reader.Root()["redis"][0]["unit"];
	for (s32 i = 0; i < units.Count(); ++i) {
		const char * ip = units[i].GetAttributeString("ip");
		s32 port = units[i].GetAttributeInt32("port");

		RedisSession * session = NEW RedisSession;
		session->SetConnect(i, ip, port);
		s_kernel->Connect(ip, port, s_sendBufferSize, s_recvBufferSize, session);
		s_sessions.push_back(session);
	}

	s_scriptEngine = (IScriptEngine*)kernel->FindModule("ScriptEngine");
	OASSERT(s_scriptEngine, "where is scriptEngine");

    return true;
}

bool Redis::Launched(IKernel * kernel) {
	s_module = s_scriptEngine->CreateModule("redis");
	s_scriptEngine->AddModuleFunction(s_module, "call", Redis::Call);
    return true;
}

bool Redis::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void Redis::OnRecv(IKernel * kernel, const s64 sequenceId, const void * context, const s32 size) {
	s_scriptEngine->Call(s_module, "onRecv", nullptr, "lS", sequenceId, context, size);
}

void Redis::OnFailed(IKernel * kernel, const s64 sequenceId) {
	s_scriptEngine->Call(s_module, "onFail", nullptr, "l", sequenceId);
}

void Redis::Call(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer) {
	s64 sequenceId;
	s32 threadId;
	s32 size = 0;
	const void * context = nullptr;

	if (!reader->Read("liS", &sequenceId, &threadId, &context, &size)) {
		writer->Write("b", false);
		return;
	}

	if (s_sessions.empty() && s_sessions[threadId % s_sessions.size()]->IsConnected())
		writer->Write("b", false);
	else {
		auto session = s_sessions[threadId % s_sessions.size()];
		session->Send(context, size);
		session->AddSequence(sequenceId);
		writer->Write("b", true);
	}
}

void Redis::Reconnect(RedisSession * session) {
	s_kernel->Connect(session->GetConnectIp(), session->GetConnectPort(), s_sendBufferSize, s_recvBufferSize, session);
}

