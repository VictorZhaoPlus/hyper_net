#include "Distribution.h"
#include "OArgs.h"
#include "IProtocolMgr.h"
#include "UserNodeType.h"

bool Distribution::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool Distribution::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);
	if (_harbor->GetNodeType() == user_node_type::SCENEMGR) {
		FIND_MODULE(_protocolMgr, ProtocolMgr);

		_harbor->AddNodeListener(this, "Distribution");
		_proto.distributeLogicReq = _protocolMgr->GetId("proto_login", "distribute_logic_req");
		_proto.distributeLogicAck = _protocolMgr->GetId("proto_login", "distribute_logic_ack");
		_proto.addPlayer = _protocolMgr->GetId("proto_login", "add_player");
		_proto.removePlayer = _protocolMgr->GetId("proto_login", "remove_player");

		RGS_HABOR_ARGS_HANDLER(_proto.distributeLogicReq, Distribution::OnRecvDistributeLogic);
		RGS_HABOR_ARGS_HANDLER(_proto.addPlayer, Distribution::OnRecvAddPlayer);
		RGS_HABOR_ARGS_HANDLER(_proto.removePlayer, Distribution::OnRecvRemovePlayer);
	}
    return true;
}

bool Distribution::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

s32 Distribution::ChooseLogic(s64 actorId) {
	if (_logices.empty())
		return 0;

	return _logices[rand() % (s32)_logices.size()];
}

void Distribution::OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port) {
	if (nodeType == user_node_type::LOGIC) {
		auto itr = std::find(_logices.begin(), _logices.end(), nodeId);
		if (itr == _logices.end())
			_logices.push_back(nodeId);
	}
}

void Distribution::OnRecvDistributeLogic(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s64 agentId = args.GetDataInt64(0);
	s64 actorId = args.GetDataInt64(1);
	s32 logic = 0;

	auto itr = _distributes.find(actorId);
	if (itr != _distributes.end())
		logic = itr->second;
	else {
		auto itrPlayer = _players.find(actorId);
		if (itrPlayer != _players.end())
			logic = itrPlayer->second;
		else
			logic = _strategy ? _strategy->ChooseLogic(actorId) : ChooseLogic(actorId);

		if (logic > 0)
			_distributes[actorId] = logic;
	}

	IArgs<3, 128> ret;
	ret << agentId << actorId << logic;
	ret.Fix();

	_harbor->Send(nodeType, nodeId, _proto.distributeLogicAck, ret.Out());
}

void Distribution::OnRecvAddPlayer(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s64 actorId = args.GetDataInt64(0);
	_players[actorId] = nodeId;
	_distributes.erase(actorId);
}

void Distribution::OnRecvRemovePlayer(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s64 actorId = args.GetDataInt64(0);
	_players.erase(actorId);
}

