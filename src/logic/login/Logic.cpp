#include "Logic.h"
#include "IObjectMgr.h"
#include "UserNodeType.h"
#include "IProtocolMgr.h"
#include "OBuffer.h"
#include "IObjectMgr.h"
#include "IEventEngine.h"
#include "OArgs.h"

namespace logic {
	class RemoveObjectTimer : public ITimer {
	public:
		RemoveObjectTimer(IObject * object) : _id(object->GetID()) {}
		virtual ~RemoveObjectTimer() {}

		virtual void OnStart(IKernel * kernel, s64 tick) {}
		virtual void OnTimer(IKernel * kernel, s64 tick) {}
		virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {
			if (nonviolent)
				Logic::Instance()->Recover(kernel, _id);
			DEL this;
		}

		virtual void OnPause(IKernel * kernel, s64 tick) {}
		virtual void OnResume(IKernel * kernel, s64 tick) {}

	private:
		s64 _id;
	};
}

bool Logic::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool Logic::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);
	if (_harbor->GetNodeType() == user_node_type::LOGIC) {
		FIND_MODULE(_protocolMgr, ProtocolMgr);
		_noError = _protocolMgr->GetId("error", "no_error");
		_errorLoadPlayerFailed = _protocolMgr->GetId("error", "load_player_failed");

		_harbor->AddNodeListener(this, "Logic");
		_proto.bindPlayerReq = _protocolMgr->GetId("proto_login", "bind_player_req");
		_proto.bindPlayerAck = _protocolMgr->GetId("proto_login", "bind_player_ack");
		_proto.unbindPlayerReq = _protocolMgr->GetId("proto_login", "unbind_player_req");
		_proto.transmitToLogic = _protocolMgr->GetId("proto_login", "transmit_to_logic");
		_proto.addPlayer = _protocolMgr->GetId("proto_login", "add_player");
		_proto.removePlayer = _protocolMgr->GetId("proto_login", "remove_player");

		RGS_HABOR_ARGS_HANDLER(_proto.bindPlayerReq, Logic::OnBindLogic);
		RGS_HABOR_ARGS_HANDLER(_proto.unbindPlayerReq, Logic::OnUnbindLogic);
		RGS_HABOR_HANDLER(_proto.transmitToLogic, Logic::OnTransMsg);

		FIND_MODULE(_objectMgr, ObjectMgr);
		_prop.account = _objectMgr->CalcProp("account");
		_prop.gate = _objectMgr->CalcProp("gate");
		_prop.logic = _objectMgr->CalcProp("logic");
		_prop.recoverTimer = _objectMgr->CalcProp("recoverTimer");

		FIND_MODULE(_eventEngine, EventEngine);
		_eventOnline = _protocolMgr->GetId("event", "online");
		_eventReconnect = _protocolMgr->GetId("event", "reconnect");
		_eventGateLost = _protocolMgr->GetId("event", "gate_lost");
		_eventRecover = _protocolMgr->GetId("event", "recover");
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
				if (object->GetPropInt32(_prop.gate) > 0) {
					object->SetPropInt32(_prop.gate, 0, false);

					_eventEngine->Exec(_eventGateLost, &object, sizeof(object));

					OASSERT(object->GetPropInt64(_prop.recoverTimer) == 0, "wtf");
					logic::RemoveObjectTimer * timer = NEW logic::RemoveObjectTimer(object);
					object->SetPropInt64(_prop.recoverTimer, (s64)timer);
					START_TIMER(timer, 0, 1, _recoverInverval);
				}
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

		_harbor->Send(nodeType, nodeId, _proto.bindPlayerAck, args.Out());

		_eventEngine->Exec(_eventReconnect, &object, sizeof(object));

		IArgs<1, 32> notify;
		notify << actorId;
		notify.Fix();
		_harbor->Send(user_node_type::SCENEMGR, 1, _proto.addPlayer, notify.Out());

		OASSERT(object->GetPropInt64(_prop.recoverTimer) != 0, "wtf");
		logic::RemoveObjectTimer * timer = (logic::RemoveObjectTimer*)object->GetPropInt64(_prop.recoverTimer);
		kernel->KillTimer(timer);
		object->SetPropInt64(_prop.recoverTimer, 0);
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

			_harbor->Send(nodeType, nodeId, _proto.bindPlayerAck, args.Out());

			_eventEngine->Exec(_eventOnline, &object, sizeof(object));

			IArgs<1, 32> notify;
			notify << actorId;
			notify.Fix();
			_harbor->Send(user_node_type::SCENEMGR, 1, _proto.addPlayer, notify.Out());
		}
		else {
			_objectMgr->Recove(object);

			IArgs<3, 32> args;
			args << actorId << accountId << _errorLoadPlayerFailed;
			args.Fix();

			_harbor->Send(nodeType, nodeId, _proto.bindPlayerAck, args.Out());
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

		_eventEngine->Exec(_eventGateLost, &object, sizeof(object));

		OASSERT(object->GetPropInt64(_prop.recoverTimer) == 0, "wtf");
		logic::RemoveObjectTimer * timer = NEW logic::RemoveObjectTimer(object);
		object->SetPropInt64(_prop.recoverTimer, (s64)timer);
		START_TIMER(timer, 0, 1, _recoverInverval);
	}
}

void Logic::OnTransMsg(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& stream) {
	OASSERT(stream.GetSize() >= sizeof(s64) + sizeof(s32) * 2, "wtf");

	s64 actorId = 0;
	if (!stream.Read(actorId))
		return;

	s32 msgId = 0;
	if (!stream.Read(msgId))
		return;

	IObject * object = _objectMgr->FindObject(actorId);
	OASSERT(object, "wtf");
	if (object) {
		OASSERT(object->GetPropInt32(_prop.gate) == nodeId, "wtf");
	
		_protos.Call(msgId, kernel, object, stream.Left());
	}
}

void Logic::Recover(IKernel * kernel, const s64 id) {
	IObject * object = _objectMgr->FindObject(id);
	OASSERT(object, "wtf");
	if (object) {
		object->SetPropInt64(_prop.recoverTimer, 0);

		_eventEngine->Exec(_eventRecover, &object, sizeof(object));

		_roleMgr->PrepareRecover(object);
		_objectMgr->Recove(object);

		IArgs<1, 32> notify;
		notify << id;
		notify.Fix();
		_harbor->Send(user_node_type::SCENEMGR, 1, _proto.removePlayer, notify.Out());
	}
}
