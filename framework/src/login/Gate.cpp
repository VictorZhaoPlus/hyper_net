#include "Gate.h"
#include "OBuffer.h"
#include "IIdMgr.h"
#include "XmlReader.h"
#include "IProtocolMgr.h"
#include "ICacheDB.h"
#include "Md5.h"
#include "base64.h"
#include "OArgs.h"

#define MAX_TOKEN_SIZE 256
#define DELAY_SEND_MEASUREMENT 100

class DelaySendTimer : public ITimer {
public:
	DelaySendTimer(const void * context, const s32 size) { _packet.assign((const char *)context, size); }
	virtual ~DelaySendTimer() {}

	virtual void OnStart(IKernel * kernel, s64 tick) {};
	virtual void OnTimer(IKernel * kernel, s32 beatCount, s64 tick) {
		if (_actors.empty())
			Gate::Instance()->Brocast(_packet.c_str(), (s32)_packet.size());
		else {
			for (auto actorId : _actors)
				Gate::Instance()->Send(actorId, _packet.c_str(), (s32)_packet.size());
		}
	}
	virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick) { DEL this; }

	virtual void OnPause(IKernel * kernel, s64 tick) {}
	virtual void OnResume(IKernel * kernel, s64 tick) {}

	void AddActor(s64 actorId) { _actors.insert(actorId); }

private:
	std::string _packet;
	std::unordered_set<s64> _actors;
};

bool Gate::Initialize(IKernel * kernel) {
    _kernel = kernel;

	olib::XmlReader reader;
	std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/server_conf.xml";
	if (!reader.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
		return false;
	}

	const olib::IXmlObject& login = reader.Root()["login"][0];
	_singleLogic = login.GetAttributeBoolean("single_logic");
	_maxRole = login.GetAttributeBoolean("max_role");

    return true;
}

bool Gate::Launched(IKernel * kernel) {
	if (OMODULE(Harbor)->GetNodeType() == PROTOCOL_ID("node_type", "gate")) {
		RGS_AGENT_LISTENER(this);

		OMODULE(Harbor)->AddNodeListener(this, "Gate");
		RGS_HABOR_ARGS_HANDLER(PROTOCOL_ID("login", "bind_account_ack"), Gate::OnRecvBindAccountAck);
		RGS_HABOR_ARGS_HANDLER(PROTOCOL_ID("login", "distribute_logic_ack"), Gate::OnRecvDistributeAck);
		RGS_HABOR_ARGS_HANDLER(PROTOCOL_ID("login", "bind_player_ack"), Gate::OnBindLogicAck);
		RGS_HABOR_ARGS_HANDLER(PROTOCOL_ID("login", "kick_from_account"), Gate::OnRecvKickFromAccount);
		RGS_HABOR_ARGS_HANDLER(PROTOCOL_ID("login", "kick_from_logic"), Gate::OnRecvKickFromLogic);
		RGS_HABOR_HANDLER(PROTOCOL_ID("login", "transmit_to_actor"), Gate::OnTransMsgToActor);
		RGS_HABOR_HANDLER(PROTOCOL_ID("login", "brocast_to_actor"), Gate::OnBrocastMsgToActor);

		_protos[PROTOCOL_ID("proto", "login_req")] = &Gate::OnRecvLoginReq;
		_protos[PROTOCOL_ID("proto", "reconect_req")] = &Gate::OnRecvReconnectReq;
		_protos[PROTOCOL_ID("proto", "select_role_req")] = &Gate::OnRecvSelectRoleReq;
		_protos[PROTOCOL_ID("proto", "create_role_req")] = &Gate::OnRecvCreateRoleReq;
		_protos[PROTOCOL_ID("proto", "delete_role_req")] = &Gate::OnRecvDeleteRoleReq;
		_protos[PROTOCOL_ID("proto", "reselect_role_req")] = &Gate::OnRecvReselectRole;

	}

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
		Reset(kernel, id, ST_NONE, PROTOCOL_ID("node_type", "user"));
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

void Gate::OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) {
	if (nodeType == PROTOCOL_ID("node_type", "account")) {
		std::unordered_map<s64, Player> tmp(_players);
		for (auto itr = tmp.begin(); itr != tmp.end(); ++itr)
			OMODULE(Agent)->Kick(itr->first);
	}
	else if (nodeType == PROTOCOL_ID("node_type", "logic")) {
		std::unordered_set<s64> tmp(_logicPlayers[nodeId]);
		for (auto agentId : tmp)
			OMODULE(Agent)->Kick(agentId);
	}
}

void Gate::OnRecvLoginReq(IKernel * kernel, const s64 id, const OBuffer& buf) {
	const char * token = nullptr;
	if (!buf.Read(token))
		return;

	OASSERT(_players.find(id) != _players.end(), "where is agent???");
	Player& player = _players[id];

	if (player.state == ST_NONE) {
		TokenData data;
		if (strlen(token) > MAX_TOKEN_SIZE || !CheckToken(token, data, true)) {
			olib::Buffer<128> buf;
			buf << PROTOCOL_ID("error", "invalid_token");
			SendToClient(kernel, player.agentId, PROTOCOL_ID("proto", "login_ack"), buf.Out());
			return;
		}

		s64 lastActorId = 0;
		s64 bantime = 0;
		bool writeOk = true;
		bool readOk = OMODULE(CacheDB)->Read("account", [](IKernel * kernel, ICacheDBReader * reader) {
			reader->ReadColumn("lastActor");
			reader->ReadColumn("bantime");
		}, [&data, &lastActorId, &bantime, &writeOk, this](IKernel * kernel, ICacheDBReadResult * result) {
			if (result->Count() > 0) {
				lastActorId = result->GetDataInt64(0, 1);
				bantime = result->GetDataInt64(0, 2);
			}
			else {
				OMODULE(CacheDB)->Write("account", [&data](IKernel * kernel, ICacheDBContext * writer) {
					writer->WriteInt64("platform", data.platform);
					writer->WriteInt64("lastActor", 0);
					writer->WriteInt64("bantime", 0);
				}, 1, data.accountID);
			}
		}, 1, data.accountID);

		if (readOk && writeOk) {
			player.accountId = data.accountID;
			player.lastActorId = lastActorId;
			player.state = ST_AUTHENING;

			IArgs<3, 128> args;
			args << player.agentId << player.accountId << 0;
			args.Fix();

			OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "account"), 1, PROTOCOL_ID("login", "bind_account_req"), args.Out());
		}
		else {
			olib::Buffer<128> buf;
			buf << PROTOCOL_ID("error", "read_account_failed");
			SendToClient(kernel, player.agentId, PROTOCOL_ID("proto", "login_ack"), buf.Out());
		}
	}
}

void Gate::OnRecvReconnectReq(IKernel * kernel, const s64 id, const OBuffer& buf) {
	const char * token = nullptr;
	if (!buf.Read(token))
		return;

	OASSERT(_players.find(id) != _players.end(), "where is agent???");
	Player& player = _players[id];

	if (player.state == ST_NONE) {
		TokenData data;
		if (strlen(token) > MAX_TOKEN_SIZE || !CheckToken(token, data, false) || data.count <= 0) {
			olib::Buffer<128> buf;
			buf << PROTOCOL_ID("error", "invalid_token");
			SendToClient(kernel, player.agentId, PROTOCOL_ID("proto", "login_ack"), buf.Out());
			return;
		}

		s64 lastActorId = 0;
		s64 bantime = 0;
		bool hasOne = false;
		bool readOk = OMODULE(CacheDB)->Read("account", [](IKernel * kernel, ICacheDBReader * reader) {
			reader->ReadColumn("lastActor");
			reader->ReadColumn("bantime");
		}, [&data, &lastActorId, &bantime, &hasOne, this](IKernel * kernel, ICacheDBReadResult * result) {
			if (result->Count() > 0) {
				lastActorId = result->GetDataInt64(0, 1);
				bantime = result->GetDataInt64(0, 2);
				hasOne = true;
			}
		}, 1, data.accountID);

		if (readOk && hasOne) {
			player.accountId = data.accountID;
			player.lastActorId = lastActorId;
			player.state = ST_AUTHENING;

			IArgs<3, 128> args;
			args << player.agentId << player.accountId << data.count;
			args.Fix();

			OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "account"), 1, PROTOCOL_ID("login", "bind_account_req"), args.Out());
		}
		else {
			olib::Buffer<128> buf;
			buf << PROTOCOL_ID("error", "read_account_failed");
			SendToClient(kernel, player.agentId, PROTOCOL_ID("proto", "login_ack"), buf.Out());
		}
	}
}

void Gate::OnRecvBindAccountAck(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s64 agentId = args.GetDataInt64(0);
	s64 accountId = args.GetDataInt64(1);
	s32 errorCode = args.GetDataInt32(2);
	s32 tokenCount = args.GetDataInt32(3);

	if (_players.find(agentId) != _players.end()) {
		Player& player = _players[agentId];
		OASSERT(player.state == ST_AUTHENING && player.accountId == accountId, "wtf");
		if (player.accountId != accountId)
			return;

		if (errorCode == 0) {
			bool ret = _roleMgr->GetRoleList(accountId, [&player](IKernel * kernel, s64 actorId, IRole * role) {
				player.roles.push_back({ actorId, role });
			});

			if (ret) {
				TokenData data = { player.accountId, 0, (s32)(tools::GetTimeMillisecond() / 1000), tokenCount };
				char token[MAX_TOKEN_SIZE + 1];
				BuildToken(token, sizeof(token) - 1, data);

				olib::Buffer<4096> buf;
				buf << PROTOCOL_ID("error", "no_error") << token << (s32)player.roles.size();
				for (const auto& role : player.roles) {
					buf << role.actorId;
					role.role->Pack(buf);
				}

				SendToClient(kernel, player.agentId, PROTOCOL_ID("proto", "login_ack"), buf.Out());
			}
			else {
				Reset(kernel, agentId, ST_NONE, PROTOCOL_ID("node_type", "user"));

				olib::Buffer<128> buf;
				buf << PROTOCOL_ID("error", "load_role_failed");
				SendToClient(kernel, player.agentId, PROTOCOL_ID("proto", "login_ack"), buf.Out());
			}
		}
		else {
			olib::Buffer<128> buf;
			buf << errorCode;
			SendToClient(kernel, player.agentId, PROTOCOL_ID("proto", "login_ack"), buf.Out());
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

			IArgs<2, 32> args;
			args << actorId << player.accountId;
			args.Fix();

			OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "logic"), 1, PROTOCOL_ID("login", "bind_player_req"), args.Out());
		}
		else {
			player.state = ST_DISTRIBUTE;

			IArgs<2, 32> args;
			args << id << actorId;
			args.Fix();

			OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "distributor"), 1, PROTOCOL_ID("login", "distribute_logic_req"), args.Out());
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

			IArgs<2, 32> args;
			args << actorId << player.accountId;
			args.Fix();

			OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "logic"), logic, PROTOCOL_ID("login", "bind_player_req"), args.Out());
		}
		else {
			Reset(kernel, agentId, ST_ROLELOADED, PROTOCOL_ID("node_type", "user"));

			olib::Buffer<128> buf;
			buf << PROTOCOL_ID("error", "distribute_role_failed");
			SendToClient(kernel, agentId, PROTOCOL_ID("proto", "select_role_ack"), buf.Out());
		}
	}
}

void Gate::OnBindLogicAck(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s64 actorId = args.GetDataInt64(0);
	s64 accountId = args.GetDataInt64(1);
	s32 errorCode = args.GetDataInt32(2);

	if (_actors.find(actorId) != _actors.end()) {
		OASSERT(_players.find(_actors[actorId]) != _players.end(), "wtf");
		Player& player = _players[_actors[actorId]];
		OASSERT(player.state == ST_BINDING && actorId == player.selectActorId, "wtf");

		if (errorCode) {
			player.state = ST_ONLINE;
			player.lastActorId = actorId;

			OMODULE(CacheDB)->Write("account", [&player, actorId](IKernel * kernel, ICacheDBContext * writer) {
				writer->WriteInt64("lastActor", actorId);
			}, 1, player.accountId);

			const s32 tokenCount = args.GetDataInt32(3);
		}
		else {
			Reset(kernel, _actors[actorId], ST_ROLELOADED, PROTOCOL_ID("node_type", "user"));

			olib::Buffer<128> buf;
			buf << PROTOCOL_ID("error", "bind_logic_failed");
			SendToClient(kernel, _actors[actorId], PROTOCOL_ID("proto", "select_role_ack"), buf.Out());
		}
	}
}

void Gate::OnUpdateRole(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s64 actorId = args.GetDataInt64(0);
	if (_actors.find(actorId) != _actors.end()) {
		OASSERT(_players.find(_actors[actorId]) != _players.end(), "wtf");
		Player& player = _players[_actors[actorId]];
		OASSERT(player.state == ST_ONLINE && actorId == player.selectActorId && nodeId == player.logic, "wtf");

		auto itr = std::find_if(player.roles.begin(), player.roles.end(), [actorId](const Role& role) {
			return role.actorId == actorId;
		});

		OASSERT(itr != player.roles.end(), "wtf");
		if (itr != player.roles.end())
			itr->role->Update(args);
	}
}

void Gate::OnRecvReselectRole(IKernel * kernel, const s64 id, const OBuffer& buf) {
	OASSERT(_players.find(id) != _players.end(), "where is agent???");
	Player& player = _players[id];
	if (player.state == ST_ONLINE) {
		Reset(kernel, id, ST_ROLELOADED, PROTOCOL_ID("node_type", "user"));

		olib::Buffer<4096> buf;
		buf << PROTOCOL_ID("error", "no_error") << (s32)player.roles.size();
		for (const auto& role : player.roles) {
			buf << role.actorId;
			role.role->Pack(buf);
		}

		SendToClient(kernel, player.agentId, PROTOCOL_ID("proto", "reselect_role_ack"), buf.Out());
	}
}

void Gate::OnRecvCreateRoleReq(IKernel * kernel, const s64 id, const OBuffer& buf) {
	OASSERT(_players.find(id) != _players.end(), "where is agent???");
	Player& player = _players[id];
	if (player.state == ST_ROLELOADED) {
		if (player.roles.size() >= _maxRole) {
			olib::Buffer<128> buf;
			buf << PROTOCOL_ID("error", "too_much_role");
			SendToClient(kernel, id, PROTOCOL_ID("proto", "create_role_ack"), buf.Out());
			return;
		}

		s64 actorId = OMODULE(IdMgr)->AllocId();
		IRole * role = _roleMgr->CreateRole(actorId, buf);
		if (role) {
			player.roles.push_back({ actorId, role });

			olib::Buffer<128> buf;
			buf << PROTOCOL_ID("error", "no_error");
			role->Pack(buf);
			SendToClient(kernel, id, PROTOCOL_ID("proto", "create_role_ack"), buf.Out());
		}
		else {
			olib::Buffer<128> buf;
			buf << PROTOCOL_ID("error", "create_role_failed");
			SendToClient(kernel, id, PROTOCOL_ID("proto", "create_role_ack"), buf.Out());
		}
	}
}

void Gate::OnRecvDeleteRoleReq(IKernel * kernel, const s64 id, const OBuffer& buf) {
	s64 actorId = 0;
	if (!buf.Read(actorId))
		return;

	OASSERT(_players.find(id) != _players.end(), "where is agent???");
	Player& player = _players[id];
	if (player.state == ST_ROLELOADED) {
		auto itr = std::find_if(player.roles.begin(), player.roles.end(), [actorId](const Role& role) {
			return role.actorId == actorId;
		});

		if (itr != player.roles.end()) {
			if (_roleMgr->DeleteRole(itr->actorId, itr->role)) {
				player.roles.erase(itr);
				
				olib::Buffer<128> buf;
				buf << PROTOCOL_ID("error", "no_error") << actorId;
				SendToClient(kernel, id, PROTOCOL_ID("proto", "delete_role_ack"), buf.Out());
			}
			else {
				olib::Buffer<128> buf;
				buf << PROTOCOL_ID("error", "delete_role_failed");
				SendToClient(kernel, id, PROTOCOL_ID("proto", "delete_role_ack"), buf.Out());
			}
		}
	}
}

void Gate::OnRecvKickFromAccount(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s64 agentId = args.GetDataInt64(0);
	if (_players.find(agentId) != _players.end()) {
		Player& player = _players[agentId];
		OASSERT(player.state >= ST_ROLELOADED, "wtf");

		Reset(kernel, agentId, ST_NONE, PROTOCOL_ID("node_type", "account"));
	}
}

void Gate::OnRecvKickFromLogic(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s64 actorId = args.GetDataInt64(0);
	if (_actors.find(actorId) != _actors.end()) {
		OASSERT(_players.find(_actors[actorId]) != _players.end(), "wtf");
		Player& player = _players[_actors[actorId]];
		OASSERT(player.state == ST_ONLINE && player.logic == nodeId, "wtf");

		Reset(kernel, _actors[actorId], ST_NONE, PROTOCOL_ID("node_type", "logic"));
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
		OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "logic"), player.logic, PROTOCOL_ID("login", "unbind_player_req"), args.Out());

		_actors.erase(player.selectActorId);
		_logicPlayers[player.logic].erase(id);

		player.selectActorId = 0;
		player.logic = 0;
	}

	if (old > ST_NONE && state == ST_NONE) {
		IArgs<1, 32> args;
		args << player.agentId << player.accountId;
		args.Fix();
		OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "account"), 1, PROTOCOL_ID("login", "unbind_account_req"), args.Out());

		player.accountId = 0;
		for (const auto& role : player.roles) {
			_roleMgr->Recover(role.role);
		}
	}
	player.state = state;
}

void Gate::TransMsgToLogic(IKernel * kernel, const s64 id, const void * context, const s32 size) {
	Player& player = _players[id];
	OMODULE(Harbor)->PrepareSend(PROTOCOL_ID("node_type", "logic"), player.logic, PROTOCOL_ID("login", "transmit_to_logic"), sizeof(s64) + size);
	OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "logic"), player.logic, &id, sizeof(id));
	OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "logic"), player.logic, context, size);
}

void Gate::OnTransMsgToActor(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& buf) {
	const void * context = buf.GetContext();
	s32 size = buf.GetSize();

	OASSERT(size >= sizeof(s32) * 2 + sizeof(s64) + sizeof(s8), "wtf");
	
	s8 delay = *(s8*)context;
	s32 * header = (s32 *)((const char *)context + sizeof(s8));
	s32 len = header[1];

	const s64 * actors = (s64*)(((const char*)context) + len + sizeof(s8));
	s32 count = (size - len - sizeof(s8)) / sizeof(64);
	if (delay > 0) {
		DelaySendTimer * timer = NEW DelaySendTimer((const char *)context + sizeof(s8), len);
		for (s32 i = 0; i < count; ++i)
			timer->AddActor(actors[i]);
		START_TIMER(timer, 0, 1, delay * DELAY_SEND_MEASUREMENT);
	}
	else {
		for (s32 i = 0; i < count; ++i) {
			s64 actorId = actors[i];
			if (_actors.find(actorId) != _actors.end()) {
				OASSERT(_players.find(_actors[actorId]) != _players.end(), "wtf");

				Player& player = _players[_actors[actorId]];
				if (player.state == ST_ONLINE)
					OMODULE(Agent)->Send(player.agentId, (const char *)context + sizeof(s8), len);
			}
		}
	}
}

void Gate::OnBrocastMsgToActor(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& buf) {
	const void * context = buf.GetContext();
	s32 size = buf.GetSize();
	OASSERT(size >= sizeof(s32) * 2 + sizeof(s8), "wtf");

	s8 delay = *(s8*)context;
	s32 * header = (s32 *)((const char *)context + sizeof(s8));
	s32 len = header[1];
	if (delay > 0) {
		DelaySendTimer * timer = NEW DelaySendTimer((const char *)context + sizeof(s8), len);
		START_TIMER(timer, 0, 1, delay * DELAY_SEND_MEASUREMENT);
	}
	else {
		for (auto itr = _players.begin(); itr != _players.end(); ++itr) {
			if (itr->second.state == ST_ONLINE)
				OMODULE(Agent)->Send(itr->second.agentId, (const char *)context + sizeof(s8), len);
		}
	}
}

void Gate::Brocast(const void * context, const s32 size) {
	for (auto itr = _players.begin(); itr != _players.end(); ++itr) {
		if (itr->second.state == ST_ONLINE)
			OMODULE(Agent)->Send(itr->second.agentId, context, size);
	}
}

void Gate::Send(s64 actorId, const void * context, const s32 size) {
	if (_actors.find(actorId) != _actors.end()) {
		OASSERT(_players.find(_actors[actorId]) != _players.end(), "wtf");

		Player& player = _players[_actors[actorId]];
		if (player.state == ST_ONLINE)
			OMODULE(Agent)->Send(player.agentId, context, size);
	}
}

void Gate::SendToClient(IKernel * kernel, const s64 id, const s32 msgId, const OBuffer& buf) {
	s32 header[2];
	header[0] = msgId;
	header[1] = buf.GetSize() + sizeof(s32) * 2;

	OMODULE(Agent)->Send(id, header, sizeof(header));
	OMODULE(Agent)->Send(id, buf.GetContext(), buf.GetSize());
}

bool Gate::CheckToken(const char * token, TokenData& data, bool login) {
	u8 buf[1024];
	u32 size = 1023;
	if (BASE64_OK != Base64Decode(token, (u32)strlen(token), buf, &size))
		return false;
	buf[size] = 0;

	data = *(TokenData*)buf;
	const char * sign = (const char *)(buf + sizeof(TokenData));

	char checkBuf[1024];
	SafeMemcpy(checkBuf, sizeof(checkBuf), buf, sizeof(TokenData));
	SafeSprintf(checkBuf + sizeof(TokenData), sizeof(checkBuf) - sizeof(TokenData), "%s", login ? _loginKey.c_str() : _tokenKey.c_str());
	MD5 md5;
	md5.update(checkBuf, sizeof(TokenData) + login ? _loginKey.size() : _tokenKey.size());
	if (strcmp(md5.toString().c_str(), sign) != 0)
		return false;

	return true;
}

void Gate::BuildToken(char * token, s32 size, const TokenData& data) {
	char checkBuf[1024];
	SafeMemcpy(checkBuf, sizeof(checkBuf), &data, sizeof(TokenData));
	SafeSprintf(checkBuf + sizeof(TokenData), sizeof(checkBuf) - sizeof(TokenData), "%s", _tokenKey.c_str());

	MD5 md5;
	md5.update(checkBuf, sizeof(TokenData) + _tokenKey.size());
	std::string sign = md5.toString();

	char buf[1024];
	SafeMemcpy(buf, sizeof(buf), &data, sizeof(TokenData));
	SafeSprintf(buf + sizeof(TokenData), sizeof(buf) - sizeof(TokenData), "%s", sign.c_str());

	u32 outSize = size;
	s32 ret = Base64Encode((u8*)buf, (u32)(sizeof(TokenData) + sign.size()), token, &outSize);
	OASSERT(ret == BASE64_OK, "wtf");
	token[outSize] = 0;
}
