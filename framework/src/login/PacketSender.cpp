#include "PacketSender.h"
#include "IHarbor.h"
#include "OBuffer.h"
#include "IProtocolMgr.h"

bool PacketSender::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool PacketSender::Launched(IKernel * kernel) {
    return true;
}

bool PacketSender::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void PacketSender::Send(const s32 gate, const s64 actorId, const s32 msgId, const OBuffer& buf, s8 delay) {
	s32 header[2];
	header[0] = msgId;
	header[1] = buf.GetSize() + sizeof(s32) * 2;

	OMODULE(Harbor)->PrepareSend(PROTOCOL_ID("node_type", "gate"), gate, PROTOCOL_ID("login", "transmit_to_actor"), sizeof(s32) * 2 + buf.GetSize() + sizeof(s64) + sizeof(s8));
	OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "gate"), gate, &delay, sizeof(delay));
	OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "gate"), gate, header, sizeof(header));
	OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "gate"), gate, buf.GetContext(), buf.GetSize());
	OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "gate"), gate, &actorId, sizeof(actorId));
}

void PacketSender::Brocast(const std::unordered_map<s32, std::vector<s64>>& actors, const s32 msgId, const OBuffer& buf, s8 delay) {
	s32 header[2];
	header[0] = msgId;
	header[1] = buf.GetSize() + sizeof(s32) * 2;

	for (auto itr = actors.begin(); itr != actors.end(); ++itr) {
		if (!itr->second.empty()) {
			OMODULE(Harbor)->PrepareSend(PROTOCOL_ID("node_type", "gate"), itr->first, PROTOCOL_ID("login", "transmit_to_actor"), sizeof(s32) * 2 + buf.GetSize() + sizeof(s64) * (s32)itr->second.size() + sizeof(s8));
			OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "gate"), itr->first, &delay, sizeof(delay));
			OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "gate"), itr->first, header, sizeof(header));
			OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "gate"), itr->first, buf.GetContext(), buf.GetSize());
			OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "gate"), itr->first, itr->second.data(), sizeof(s64) * (s32)itr->second.size());
		}
	}
}

void PacketSender::Brocast(const s32 msgId, const OBuffer& buf, s8 delay) {
	s32 header[2];
	header[0] = msgId;
	header[1] = buf.GetSize() + sizeof(s32) * 2;

	OMODULE(Harbor)->PrepareBrocast(PROTOCOL_ID("node_type", "gate"), PROTOCOL_ID("login", "brocast_to_actor"), sizeof(s32) * 2 + buf.GetSize() + sizeof(s8));
	OMODULE(Harbor)->Brocast(PROTOCOL_ID("node_type", "gate"), &delay, sizeof(delay));
	OMODULE(Harbor)->Brocast(PROTOCOL_ID("node_type", "gate"), header, sizeof(header));
	OMODULE(Harbor)->Brocast(PROTOCOL_ID("node_type", "gate"), buf.GetContext(), buf.GetSize());
}

