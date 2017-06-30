#include "Command.h"
#include "IScriptEngine.h"
#include "IHarbor.h"
#include "IProtocolMgr.h"
#include "OBuffer.h"
#include "IObjectMgr.h"

bool Command::Initialize(IKernel * kernel) {
	_kernel = kernel;
    return true;
}

bool Command::Launched(IKernel * kernel) {
	_module = OMODULE(ScriptEngine)->CreateModule("command");

	if (OMODULE(Harbor)->GetNodeType() == PROTOCOL_ID("node_type", "logic")) {
		RGS_HABOR_HANDLER(PROTOCOL_ID("command", "call"), Command::OnCall);
		RGS_HABOR_HANDLER(PROTOCOL_ID("command", "call_no_ret"), Command::OnCallNoRet);
		RGS_HABOR_HANDLER(PROTOCOL_ID("command", "respone"), Command::OnRespone);

		RGS_SCRIPT_FUNC(_module, "call_wait", Command::Call);
		RGS_SCRIPT_FUNC(_module, "call", Command::CallNoRet);
		RGS_SCRIPT_FUNC(_module, "respone", Command::Respone);
	}
	else if (OMODULE(Harbor)->GetNodeType() == PROTOCOL_ID("node_type", "relation")) {
		RGS_HABOR_HANDLER(PROTOCOL_ID("command", "call_forward"), Command::OnCallForward);
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
	OMODULE(Harbor)->PrepareSend(PROTOCOL_ID("node_type", "relation"), 1, PROTOCOL_ID("command", "call_forward"), sizeof(s64) * 2 + sizeof(s32) + size);
	OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "relation"), 1, &id, sizeof(id));
	OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "relation"), 1, &reciever, sizeof(reciever));
	OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "relation"), 1, &sequenceId, sizeof(sequenceId));
	OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "relation"), 1, buffer, size);

	writer->Write("i", sequenceId);
}

void Command::CallNoRet(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer) {
	s64 reciever = 0;
	s32 size = 0;
	const void * buffer = nullptr;

	if (!reader->Read("lS", &reciever, &buffer, &size))
		return;

	OMODULE(Harbor)->PrepareSend(PROTOCOL_ID("node_type", "relation"), 1, PROTOCOL_ID("command", "call_no_ret"), sizeof(s64) + size);
	OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "relation"), 1, &reciever, sizeof(reciever));
	OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "relation"), 1, buffer, size);
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

		OMODULE(Harbor)->PrepareSend(itr->second.nodeType, itr->second.nodeId, PROTOCOL_ID("command", "respone"), sizeof(s64) + sizeof(s32) + size);
		OMODULE(Harbor)->Send(itr->second.nodeType, itr->second.nodeId, &itr->second.sender, sizeof(s64));
		OMODULE(Harbor)->Send(itr->second.nodeType, itr->second.nodeId, &itr->second.sequenceId, sizeof(s32));
		OMODULE(Harbor)->Send(itr->second.nodeType, itr->second.nodeId, buffer, size);

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

