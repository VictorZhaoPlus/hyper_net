#include "Logic.h"
#include "IObjectMgr.h"
#include "FrameworkProtocol.h"
#include "UserNodeType.h"
#include "IProtocolMgr.h"
#include "OBuffer.h"

bool Logic::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool Logic::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);
	if (_harbor->GetNodeType() == user_node_type::LOGIC) {
		_harbor->AddNodeListener(this, "Logic");
		RGS_HABOR_ARGS_HANDLER(framework_proto::BIND_PLAYER, Logic::OnBindLogic);
		RGS_HABOR_ARGS_HANDLER(framework_proto::UNBIND_PLAYER, Logic::OnUnbindLogic);
		RGS_HABOR_HANDLER(framework_proto::TRANSMIT_TO_LOGIC, Logic::OnTransMsg);

		FIND_MODULE(_objectMgr, ObjectMgr);
		_prop.account = _objectMgr->CalcProp("account");
		_prop.gate = _objectMgr->CalcProp("gate");
		_prop.logic = _objectMgr->CalcProp("logic");

		FIND_MODULE(_protocolMgr, ProtocolMgr);
		_noError = _protocolMgr->GetId("error", "no_error");
		_errorLoadPlayerFailed = _protocolMgr->GetId("error", "load_player_failed");
	}

    return true;
}

bool Logic::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void Logic::OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) {
	if (nodeType == user_node_type::GATE) {
		for (auto actorId : _gateActors[nodeId]) {
			IObject * object = _objectMgr->FindObject(actorId);
			OASSERT(object, "wtf");
			if (object) {
				object->SetPropInt32(_prop.gate, 0, false);

				//drop reconnect event;
			}
		}
		_gateActors[nodeId].clear();
	}
}

void Logic::RegProtocolHandler(s32 messageId, const ProtocolCB& handler, const char * debug) {
	_protos.Register(messageId, handler, debug);
}

void Logic::OnBindLogic(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s64 actorId = args.GetDataInt64(0);
	s64 accountId = args.GetDataInt64(1);

	IObject * object = _objectMgr->FindObject(actorId);
	if (object) {
		object->SetPropInt32(_prop.gate, nodeId, false);
		_gateActors[nodeId].insert(actorId);

		IArgs<3, 32> args;
		args << actorId << accountId << _noError;
		args.Fix();

		_harbor->Send(nodeType, nodeId, framework_proto::BIND_PLAYER_ACK, args.Out());

		//drop reconnect event;
	}
	else {
		object = CREATE_OBJECT_BYID(_objectMgr, "Player", actorId);
		OASSERT(object, "wtf");

		object->SetPropInt64(_prop.account, accountId, false);
		object->SetPropInt32(_prop.gate, nodeId, false);
		object->SetPropInt32(_prop.logic, _harbor->GetNodeId(), false);

		if (_roleMgr->LoadRole(actorId, object)) {
			_gateActors[nodeId].insert(actorId);

			IArgs<3, 32> args;
			args << actorId << accountId << _noError;
			args.Fix();

			_harbor->Send(nodeType, nodeId, framework_proto::BIND_PLAYER_ACK, args.Out());

			//drop online event;
			//drop online complete event;
		}
		else {
			_objectMgr->Recove(object);

			IArgs<3, 32> args;
			args << actorId << accountId << _errorLoadPlayerFailed;
			args.Fix();

			_harbor->Send(nodeType, nodeId, framework_proto::BIND_PLAYER_ACK, args.Out());
		}
	}
}

void Logic::OnUnbindLogic(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s64 actorId = args.GetDataInt64(0);
	IObject * object = _objectMgr->FindObject(actorId);
	OASSERT(object, "wtf");
	if (object) {
		OASSERT(object->GetPropInt32(_prop.gate) == nodeId, "wtf");

		object->SetPropInt32(_prop.gate, 0, false);
		_gateActors[nodeId].erase(actorId);

		//drop gate lost event;
	}
}

void Logic::OnTransMsg(IKernel * kernel, s32 nodeType, s32 nodeId, const void * context, const s32 size) {
	OASSERT(size >= sizeof(s64) + sizeof(s32) * 2, "wtf");

	s64 actorId = *(s64*)context;
	s32 msgId = *(s32*)((const char*)context + sizeof(s64));
	OBuffer buf((const char*)context + sizeof(s64) + sizeof(s32) * 2, size - sizeof(s64) + sizeof(s32) * 2);

	IObject * object = _objectMgr->FindObject(actorId);
	OASSERT(object, "wtf");
	if (object) {
		OASSERT(object->GetPropInt32(_prop.gate) == nodeId, "wtf");
	
		_protos.Call(msgId, kernel, object, buf);
	}
}
