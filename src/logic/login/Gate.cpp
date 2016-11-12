#include "Gate.h"
#include "OBuffer.h"
#include "IHarbor.h"
#include "UserNodeType.h"
#include "FrameworkProtocol.h"
#include "IIdMgr.h"

bool Gate::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool Gate::Launched(IKernel * kernel) {
    return true;
}

bool Gate::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void Gate::OnAgentOpen(IKernel * kernel, const s64 id) {
	OASSERT(_players.find(id) == _players.end(), "duplicate agent id");
	_players[id] = { id, 0, 0, 0, 0, ST_NONE };
}

void Gate::OnAgentClose(IKernel * kernel, const s64 id) {
	OASSERT(_players.find(id) != _players.end(), "where is agent???");

	if (_players[id].state != ST_NONE) {
		Reset(kernel, id, ST_NONE, node_type::USER);
	}

	_players.erase(id);
}

s32 Gate::OnAgentRecvPacket(IKernel * kernel, const s64 id, const void * context, const s32 size) {
	if (size < sizeof(s32) * 2)
		return 0;

	s32 len = ((s32*)context)[1];
	if (size < len)
		return 0;

	s32 msgId = ((s16*)context)[0];
	if (_protos.find(msgId) != _protos.end()) {
		OBuffer buf((const char*)context + sizeof(s32) * 2, len);
		(this->*_protos[msgId])(kernel, id, buf);
	}
	else {
		if (_players[id].state == ST_ONLINE) {
			TransMsgToLogic(kernel, id, context, len);
		}
	}

	return len;
}

void Gate::OnRecvLoginReq(IKernel * kernel, const s64 id, const OBuffer& buf) {

}

void Gate::OnRecvConnectReq(IKernel * kernel, const s64 id, const OBuffer& buf) {

}

void Gate::OnRecvBindAccountAck(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s64 agentId = args.GetDataInt64(0);
	s64 accountId = args.GetDataInt64(1);

	if (_players.find(agentId) != _players.end()) {
		Player& player = _players[agentId];
		OASSERT(player.state == ST_AUTHENING && player.accountId == accountId, "wtf");

		//load role list
		bool ret = _roleMgr->GetRoleList(accountId, [&player](IKernel * kernel, s64 actorId, IRole * role) {
			player.roles.push_back({ actorId, role });
		});

		if (ret) {
			//send ok;
		}
		else {
			Reset(kernel, agentId, ST_NONE, node_type::USER);

			//send failed;
		}
	}
}

void Gate::OnRecvSelectRoleReq(IKernel * kernel, const s64 id, const OBuffer& buf) {
	s64 actorId = 0;
	if (!buf.Read(actorId))
		return;

	OASSERT(_players.find(id) != _players.end(), "where is agent???");
	Player& player = _players[id];

	if (player.state == ST_ROLELOADED) {
		if (std::find_if(player.roles.begin(), player.roles.end(), [actorId](const Role& role) {
			return role.actorId == actorId;
		}) == player.roles.end()) {
			return;
		}

		if (_singleLogic) {
			player.selectActorId = actorId;
			player.logic = 1;
			player.state = ST_BINDING;

			_actors[actorId] = id;
			_logicPlayers[1].insert(id);

			IArgs<1, 32> args;
			args << actorId;
			args.Fix();

			_harbor->Send(user_node_type::LOGIC, 1, framework_proto::BIND_PLAYER, args.Out());
		}
		else {
			player.state = ST_DISTRIBUTE;

			IArgs<2, 32> args;
			args << id << actorId;
			args.Fix();

			_harbor->Send(user_node_type::SCENEMGR, 1, framework_proto::DISTRIBUTE_LOGIC_REQ, args.Out());
		}
	}
}

void Gate::OnRecvDistributeAck(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s64 agentId = args.GetDataInt64(0);
	s64 actorId = args.GetDataInt64(1);
	s32 logic = args.GetDataInt32(2);

	if (_players.find(agentId) != _players.end()) {
		if (logic > 0) {
			Player& player = _players[agentId];
			OASSERT(player.state == ST_DISTRIBUTE, "wtf");

			player.selectActorId = actorId;
			player.logic = logic;
			player.state = ST_BINDING;

			_actors[actorId] = agentId;
			_logicPlayers[logic].insert(agentId);

			IArgs<1, 32> args;
			args << actorId;
			args.Fix();

			_harbor->Send(user_node_type::LOGIC, logic, framework_proto::BIND_PLAYER, args.Out());
		}
		else {
			Reset(kernel, agentId, ST_ROLELOADED, node_type::USER);

			//send to client
		}
	}
}

void Gate::OnBindLogicAck(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s64 actorId = args.GetDataInt64(1);

	if (_actors.find(actorId) != _actors.end()) {
		OASSERT(_players.find(_actors[actorId]) != _players.end(), "wtf");
		Player& player = _players[_actors[actorId]];
		OASSERT(player.state == ST_BINDING && actorId == player.selectActorId, "wtf");

		player.state = ST_ONLINE;
		player.lastActorId = actorId;

		//update last actor
	}
}

void Gate::OnRecvCreateRoleReq(IKernel * kernel, const s64 id, const OBuffer& buf) {
	OASSERT(_players.find(id) != _players.end(), "where is agent???");
	Player& player = _players[id];
	if (player.state == ST_ROLELOADED) {
		if (player.roles.size() >= _maxRole) {
			//send failed
			return;
		}

		s64 actorId = _idMgr->AllocId();
		IRole * role = _roleMgr->CreateRole(actorId, buf);
		if (role) {
			player.roles.push_back({ actorId, role });

			//send ok
		}
		else {
			//send failed
		}
	}
}

void Gate::OnRecvDeleteRoleReq(IKernel * kernel, const s64 id, const OBuffer& buf) {
	OASSERT(_players.find(id) != _players.end(), "where is agent???");
	Player& player = _players[id];
	if (player.state == ST_ROLELOADED) {
		auto itr = std::find_if(player.roles.begin(), player.roles.end(), [id](const Role& role) {
			return role.actorId == id;
		});

		if (itr != player.roles.end()) {
			if (_roleMgr->DeleteRole(itr->actorId, itr->role)) {
				player.roles.erase(itr);
				//send ok
			}
			else {
				//send failed
			}
		}
	}
}

void Gate::OnRecvKickFromAccount(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s64 agentId = args.GetDataInt64(0);
	if (_players.find(agentId) != _players.end()) {
		Player& player = _players[agentId];
		OASSERT(player.state >= ST_ROLELOADED, "wtf");

		Reset(kernel, agentId, ST_NONE, user_node_type::ACCOUNT);
	}
}

void Gate::OnRecvKickFromLogic(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s64 actorId = args.GetDataInt64(0);
	if (_actors.find(actorId) != _actors.end()) {
		OASSERT(_players.find(_actors[actorId]) != _players.end(), "wtf");
		Player& player = _players[_actors[actorId]];
		OASSERT(player.state == ST_ONLINE && player.logic == nodeId, "wtf");

		Reset(kernel, _actors[actorId], ST_NONE, user_node_type::LOGIC);
	}
}

void Gate::Reset(IKernel * kernel, s64 id, s8 state, s32 from) {
	OASSERT(state == ST_NONE || state == ST_ROLELOADED, "wtf");
	Player& player = _players[id];
	s8 old = player.state;
	if (old > ST_DISTRIBUTE) {
		OASSERT(player.logic > 0 && player.selectActorId != 0, "wtf");

		IArgs<1, 32> args;
		args << player.selectActorId;
		args.Fix();
		_harbor->Send(user_node_type::LOGIC, player.logic, framework_proto::UNBIND_PLAYER, args.Out());

		_actors.erase(player.selectActorId);
		_logicPlayers[player.logic].erase(id);

		player.selectActorId = 0;
		player.logic = 0;
	}

	if (old > ST_NONE && state == ST_NONE) {
		IArgs<1, 32> args;
		args << player.accountId;
		args.Fix();
		_harbor->Send(user_node_type::ACCOUNT, 1, framework_proto::UNBIND_ACCOUNT, args.Out());

		player.accountId = 0;
		for (const auto& role : player.roles) {
			_roleMgr->Recover(role.role);
		}
	}
	player.state = state;
}

void Gate::TransMsgToLogic(IKernel * kernel, const s64 id, const void * context, const s32 size) {
	Player& player = _players[id];
	_harbor->PrepareSend(user_node_type::LOGIC, player.logic, framework_proto::TRANSMIT_TO_LOGIC, sizeof(s64) + size);
	_harbor->Send(user_node_type::LOGIC, player.logic, &id, sizeof(id));
	_harbor->Send(user_node_type::LOGIC, player.logic, context, size);
}
