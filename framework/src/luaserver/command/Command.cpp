#include "Command.h"
#include "IScriptEngine.h"
#include "IHarbor.h"
#include "IProtocolMgr.h"
#include "OBuffer.h"
#include "UserNodeType.h"
#include "IObjectMgr.h"

bool Command::Initialize(IKernel * kernel) {
	_kernel = kernel;
    return true;
}

bool Command::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);

	FIND_MODULE(_scriptEngine, ScriptEngine);
	_module = _scriptEngine->CreateModule("command");

	FIND_MODULE(_protocolMgr, ProtocolMgr);
	_protoCall = _protocolMgr->GetId("proto_command", "call");
	_protoCallNoRet = _protocolMgr->GetId("proto_command", "call_no_ret");
	_protoRespone = _protocolMgr->GetId("proto_command", "respone");
	_protoCallForward = _protocolMgr->GetId("proto_command", "call_forward");

	if (_harbor->GetNodeType() == user_node_type::LOGIC) {
		RGS_HABOR_HANDLER(_protoCall, Command::OnCall);
		RGS_HABOR_HANDLER(_protoCallNoRet, Command::OnCallNoRet);
		RGS_HABOR_HANDLER(_protoRespone, Command::OnRespone);

		RGS_SCRIPT_FUNC(_module, "call_wait", Command::Call);
		RGS_SCRIPT_FUNC(_module, "call", Command::CallNoRet);
		RGS_SCRIPT_FUNC(_module, "respone", Command::Respone);
	}
	else if (_harbor->GetNodeType() == user_node_type::LOGIC) {
		RGS_HABOR_HANDLER(_protoCallForward, Command::OnCallForward);
	}
	else {
		RGS_SCRIPT_FUNC(_module, "call", Command::CallNoRet);
	}
    return true;
}

bool Command::Destroy(IKernel * kernel) {
	if (_module)
		_module->Release();
    DEL this;
    return true;
}

void Command::Call(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer) {
	IObject * sender = nullptr;
	s64 reciever = 0;
	s32 size = 0;
	const void * buffer = nullptr;

	if (!reader->Read("PlS", &sender, &reciever, &buffer, &size))
		return;

	s64 id = sender->GetID();
	s32 sequenceId = _sequenceId++;
	_harbor->PrepareSend(user_node_type::RELATION, 1, _protoCallForward, sizeof(s64) * 2 + sizeof(s32) + size);
	_harbor->Send(user_node_type::RELATION, 1, &id, sizeof(id));
	_harbor->Send(user_node_type::RELATION, 1, &reciever, sizeof(reciever));
	_harbor->Send(user_node_type::RELATION, 1, &sequenceId, sizeof(sequenceId));
	_harbor->Send(user_node_type::RELATION, 1, buffer, size);

	writer->Write("i", sequenceId);
}

void Command::CallNoRet(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer) {
	s64 reciever = 0;
	s32 size = 0;
	const void * buffer = nullptr;

	if (!reader->Read("lS", &reciever, &buffer, &size))
		return;

	_harbor->PrepareSend(user_node_type::RELATION, 1, _protoCallNoRet, sizeof(s64) + size);
	_harbor->Send(user_node_type::RELATION, 1, &reciever, sizeof(reciever));
	_harbor->Send(user_node_type::RELATION, 1, buffer, size);
}

void Command::Respone(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer) {
	s64 sequenceId = 0;
	s32 size = 0;
	const void * buffer = nullptr;

	if (!reader->Read("lS", &sequenceId, &buffer, &size))
		return;

	auto itr = _respones.find(sequenceId);
	OASSERT(itr != _respones.end(), "wtf");
	if (itr != _respones.end()) {

		_harbor->PrepareSend(itr->second.nodeType, itr->second.nodeId, _protoRespone, sizeof(s64) + sizeof(s32) + size);
		_harbor->Send(itr->second.nodeType, itr->second.nodeId, &itr->second.sender, sizeof(s64));
		_harbor->Send(itr->second.nodeType, itr->second.nodeId, &itr->second.sequenceId, sizeof(s32));
		_harbor->Send(itr->second.nodeType, itr->second.nodeId, buffer, size);

		_respones.erase(itr);
	}
}

void Command::OnCall(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& stream) {
}

void Command::OnCallNoRet(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& stream) {
}

void Command::OnRespone(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& stream) {
}

void Command::OnCallForward(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& stream) {
	
}

