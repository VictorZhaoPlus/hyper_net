#include "CapacitySubscriber.h"
#include "OArgs.h"
#include "IProtocolMgr.h"

bool CapacitySubscriber::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool CapacitySubscriber::Launched(IKernel * kernel) {
	OMODULE(Harbor)->AddNodeListener(this, "CapacitySubscriber");

	RGS_HABOR_ARGS_HANDLER(PROTOCOL_ID("capacity", "over_load"), CapacitySubscriber::ReadLoad);
    return true;
}

bool CapacitySubscriber::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

s32 CapacitySubscriber::Choose(const s32 nodeType) {
	s32 find = 0;
	s32 rate = -1;
	for (auto itr = _servers[nodeType].begin(); itr != _servers[nodeType].end(); ++itr) {
		if (rate == -1 || itr->second < rate) {
			rate = itr->second;
			find = itr->first;
		}
	}

	return find;
}

bool CapacitySubscriber::CheckOverLoad(const s32 nodeType, const s32 overload) {
	s32 sum = 0;
	for (auto itr = _servers[nodeType].begin(); itr != _servers[nodeType].end(); ++itr)
		sum += itr->second;

	return sum > overload;
}

s32 CapacitySubscriber::GetOverLoad(const s32 nodeType, const s32 nodeId) {
	return _servers[nodeType][nodeId];
}

void CapacitySubscriber::ReadLoad(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args) {
	s32 overlaod = args.GetDataInt32(0);

	_servers[nodeType][nodeId] = overlaod;
}

void CapacitySubscriber::OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) {
	_servers[nodeType].erase(nodeId);
}
