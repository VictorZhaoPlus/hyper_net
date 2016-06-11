#include "CapacitySubscriber.h"
#include "CoreProtocol.h"
#include "Define.h"

CapacitySubscriber * CapacitySubscriber::s_self = nullptr;
IKernel * CapacitySubscriber::s_kernel = nullptr;
IHarbor * CapacitySubscriber::s_harbor = nullptr;

std::unordered_map<s32, std::unordered_map<s32, s32>> CapacitySubscriber::s_servers;

bool CapacitySubscriber::Initialize(IKernel * kernel) {
    s_self = this;
    s_kernel = kernel;

    return true;
}

bool CapacitySubscriber::Launched(IKernel * kernel) {
	s_harbor = (IHarbor*)kernel->FindModule("Harbor");
	OASSERT(s_harbor, "where is harbor");
	s_harbor->AddNodeListener(this, "CapacitySubscriber");

	REGPROTOCOL(core_proto::OVER_LOAD, CapacitySubscriber::ReadLoad);

    return true;
}

bool CapacitySubscriber::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

s32 CapacitySubscriber::Choose(const s32 nodeType) {
	s32 find = INVALID_NODE_ID;
	s32 rate = -1;
	for (auto itr = s_servers[nodeType].begin(); itr != s_servers[nodeType].end(); ++itr) {
		if (rate == -1 || itr->second < rate) {
			rate = itr->second;
			find = itr->first;
		}
	}

	return find;
}

bool CapacitySubscriber::CheckOverLoad(const s32 nodeType, const s32 overload) {
	s32 sum = 0;
	for (auto itr = s_servers[nodeType].begin(); itr != s_servers[nodeType].end(); ++itr)
		sum += itr->second;

	return sum > overload;
}

s32 CapacitySubscriber::GetOverLoad(const s32 nodeType, const s32 nodeId) {
	return s_servers[nodeType][nodeId];
}

void CapacitySubscriber::ReadLoad(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args) {
	s32 overlaod = args.GetDataInt32(0);

	s_servers[nodeType][nodeId] = overlaod;
}

void CapacitySubscriber::OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) {
	s_servers[nodeType].erase(nodeId);
}
