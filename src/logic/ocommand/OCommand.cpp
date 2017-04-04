#include "OCommand.h"
#include "IObjectMgr.h"
#include "IProtocolMgr.h"
#include "OArgs.h"
#include "OBuffer.h"
#include "IHarbor.h"
#include "UserNodeType.h"
#include "IObjectLocator.h"

namespace command_def {
#pragma pack(push, 1)
	struct CommandTo {
		s32 cmd;
		s64 sender;
		s64 reciever;
		s32 cb;
	};

	struct CommandForward {
		s32 cmd;
		s64 sender;
		s64 reciever;
	};

#pragma pack(pop)
}

bool OCommand::Initialize(IKernel * kernel) {
    _kernel = kernel;
	_nextFailCbId = 0;

    return true;
}

bool OCommand::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);
	FIND_MODULE(_protocolMgr, ProtocolMgr);

	_protoCommandTo = _protocolMgr->GetId("proto_cmd", "command_to");
	_protoCommandForward = _protocolMgr->GetId("proto_cmd", "command_forward");
	_protoCommandResult = _protocolMgr->GetId("proto_cmd", "command_result");

	if (_harbor->GetNodeType() == user_node_type::LOGIC) {
		FIND_MODULE(_objectMgr, ObjectMgr);
		_harbor->AddNodeListener(this, "OCommand");

		RGS_HABOR_HANDLER(_protoCommandResult, OCommand::OnForwardCommand);
		RGS_HABOR_ARGS_HANDLER(_protoCommandResult, OCommand::OnCommandResult);
	}
	else if (_harbor->GetNodeType() == user_node_type::RELATION) {
		FIND_MODULE(_objectLocator, ObjectLocator);

		RGS_HABOR_HANDLER(_protoCommandResult, OCommand::OnCommand);
	}

    return true;
}

bool OCommand::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void OCommand::Command(s32 cmd, const IObject * sender, const s64 reciever, const OArgs& args, const CommandFailCB& fail) {
	IObject * object = _objectMgr->FindObject(reciever);
	if (object) {
		_cbs.Call(cmd, _kernel, sender->GetID(), object, args);
	}
	else {
		command_def::CommandTo head;
		head.cmd = cmd;
		head.sender = sender->GetID();
		head.reciever = reciever;
		
		if (fail) {
			++_nextFailCbId;
			if (_nextFailCbId < 0)
				_nextFailCbId = 1;
			_failed[_nextFailCbId] = fail;
			head.cb = _nextFailCbId;
		}
		else
			head.cb = 0;

		_harbor->PrepareSend(user_node_type::RELATION, 1, _protoCommandTo, sizeof(command_def::CommandTo) + args.GetSize());
		_harbor->Send(user_node_type::RELATION, 1, &head, sizeof(head));
		_harbor->Send(user_node_type::RELATION, 1, args.GetContext(), args.GetSize());
	}
}

bool OCommand::CommandTo(const s64 reciver, const s32 cmd, const OArgs & args) {
	if (_harbor->GetNodeType() == user_node_type::RELATION) {
		s32 logic = _objectLocator->FindObjectLogic(reciver);
		if (logic > 0) {
			command_def::CommandForward head;
			head.cmd = cmd;
			head.sender = 0;
			head.reciever = reciver;

			_harbor->PrepareSend(user_node_type::LOGIC, logic, _protoCommandForward, sizeof(command_def::CommandTo) + args.GetSize());
			_harbor->Send(user_node_type::LOGIC, logic, &head, sizeof(head));
			_harbor->Send(user_node_type::LOGIC, logic, args.GetContext(), args.GetSize());
		}
	}
	else {
		command_def::CommandTo head;
		head.cmd = cmd;
		head.sender = 0;
		head.reciever = reciver;
		head.cb = 0;

		_harbor->PrepareSend(user_node_type::RELATION, 1, _protoCommandTo, sizeof(command_def::CommandTo) + args.GetSize());
		_harbor->Send(user_node_type::RELATION, 1, &head, sizeof(head));
		_harbor->Send(user_node_type::RELATION, 1, args.GetContext(), args.GetSize());
	}
	return true;
}

void OCommand::OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) {
	if (nodeType == user_node_type::RELATION)
		_failed.clear();
}

void OCommand::OnCommand(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& stream) {
	const void * p = nullptr;
	if (!stream.ReadStruct(p, sizeof(command_def::CommandTo))) {
		OASSERT(false, "wtf");
		return;
	}

	command_def::CommandTo * info = (command_def::CommandTo *)p;
	s32 logic = _objectLocator->FindObjectLogic(info->reciever);
	if (logic > 0) {
		command_def::CommandForward head;
		head.cmd = info->cmd;
		head.sender = info->sender;
		head.reciever = info->reciever;

		OBuffer left = stream.Left();
		_harbor->PrepareSend(user_node_type::LOGIC, logic, _protoCommandForward, sizeof(command_def::CommandTo) + left.GetSize());
		_harbor->Send(user_node_type::LOGIC, logic, &head, sizeof(head));
		_harbor->Send(user_node_type::LOGIC, logic, left.GetContext(), left.GetSize());

		if (info->cb > 0) {
			IArgs<5, 64> args;
			args << info->cmd << info->sender << info->reciever << info->cb << true;
			args.Fix();
			_harbor->Send(nodeType, nodeId, _protoCommandResult, args.Out());
		}
	}
	else {
		if (info->cb > 0) {
			IArgs<5, 64> args;
			args << info->cmd << info->sender << info->reciever << info->cb << false;
			args.Fix();
			_harbor->Send(nodeType, nodeId, _protoCommandResult, args.Out());
		}
	}
}

void OCommand::OnForwardCommand(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& stream) {
	const void * p = nullptr;
	if (!stream.ReadStruct(p, sizeof(command_def::CommandForward))) {
		OASSERT(false, "wtf");
		return;
	}

	command_def::CommandForward * info = (command_def::CommandForward *)p;
	IObject * object = _objectMgr->FindObject(info->reciever);
	if (object) {
		OBuffer left = stream.Left();
		_cbs.Call(info->cmd, _kernel, info->sender, object, OArgs(left.GetContext(), left.GetSize()));
	}
}

void OCommand::OnCommandResult(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	bool ok = args.GetDataBool(4);
	s32 cb = args.GetDataInt32(3);
	if (!ok) {
		OASSERT(_failed.find(cb) != _failed.end(), "wtf");
		_failed[cb](_kernel, args.GetDataInt32(0), args.GetDataInt64(1), args.GetDataInt64(2));
	}

	_failed.erase(cb);
}
