#include "PacketSender.h"
#include "IHarbor.h"
#include "UserNodeType.h"
#include "OBuffer.h"
#include "IProtocolMgr.h"

bool PacketSender::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool PacketSender::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);
	FIND_MODULE(_protocolMgr, ProtocolMgr);

	_proto.transmitToActor = _protocolMgr->GetId("proto_login", "transmit_to_actor");
	_proto.brocastToActor = _protocolMgr->GetId("proto_login", "brocast_to_actor");
    return true;
}

bool PacketSender::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void PacketSender::Send(const s32 gate, const s32 actorId, const s32 msgId, const OBuffer& buf, s8 delay) {
	s32 header[2];
	header[0] = msgId;
	header[1] = buf.GetSize() + sizeof(s32) * 2;

	_harbor->PrepareSend(user_node_type::GATE, gate, _proto.transmitToActor, sizeof(s32) * 2 + buf.GetSize() + sizeof(s64) + sizeof(s8));
	_harbor->Send(user_node_type::GATE, gate, &delay, sizeof(delay));
	_harbor->Send(user_node_type::GATE, gate, header, sizeof(header));
	_harbor->Send(user_node_type::GATE, gate, buf.GetContext(), buf.GetSize());
	_harbor->Send(user_node_type::GATE, gate, &actorId, sizeof(actorId));
}

void PacketSender::Brocast(const std::unordered_map<s32, std::vector<s64>>& actors, const s32 msgId, const OBuffer& buf, s8 delay) {
	s32 header[2];
	header[0] = msgId;
	header[1] = buf.GetSize() + sizeof(s32) * 2;

	for (auto itr = actors.begin(); itr != actors.end(); ++itr) {
		if (!itr->second.empty()) {
			_harbor->PrepareSend(user_node_type::GATE, itr->first, _proto.transmitToActor, sizeof(s32) * 2 + buf.GetSize() + sizeof(s64) * (s32)itr->second.size() + sizeof(s8));
			_harbor->Send(user_node_type::GATE, itr->first, &delay, sizeof(delay));
			_harbor->Send(user_node_type::GATE, itr->first, header, sizeof(header));
			_harbor->Send(user_node_type::GATE, itr->first, buf.GetContext(), buf.GetSize());
			_harbor->Send(user_node_type::GATE, itr->first, itr->second.data(), sizeof(s64) * (s32)itr->second.size());
		}
	}
}

void PacketSender::Brocast(const s32 msgId, const OBuffer& buf, s8 delay) {
	s32 header[2];
	header[0] = msgId;
	header[1] = buf.GetSize() + sizeof(s32) * 2;

	_harbor->PrepareBrocast(user_node_type::GATE, _proto.brocastToActor, sizeof(s32) * 2 + buf.GetSize() + sizeof(s8));
	_harbor->Brocast(user_node_type::GATE, &delay, sizeof(delay));
	_harbor->Brocast(user_node_type::GATE, header, sizeof(header));
	_harbor->Brocast(user_node_type::GATE, buf.GetContext(), buf.GetSize());
}

