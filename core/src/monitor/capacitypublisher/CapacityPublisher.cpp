#include "CapacityPublisher.h"
#include "OArgs.h"
#include "IProtocolMgr.h"

#define CAPACITY_SYNC_INTERVAL 500

bool CapacityPublisher::Initialize(IKernel * kernel) {
    _kernel = kernel;
	_load = 0;

	START_TIMER(this, 0, TIMER_BEAT_FOREVER, CAPACITY_SYNC_INTERVAL);
    return true;
}

bool CapacityPublisher::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);
	_harbor->AddNodeListener(this, "CapacityPublisher");

	FIND_MODULE(_protocolMgr, ProtocolMgr);
	_protoOverLoad = _protocolMgr->GetId("proto_capacity", "over_load");
    return true;
}

bool CapacityPublisher::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void CapacityPublisher::IncreaceLoad(const s32 value) {
	_load += value;
	_changed = true;
}

void CapacityPublisher::DecreaceLoad(const s32 value) {
	_load -= value;
	_changed = true;
}

void CapacityPublisher::OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port) {
	IArgs<1, 64> args;
	args << _load;
	args.Fix();

	_harbor->Send(nodeType, nodeId, _protoOverLoad, args.Out());
}

void CapacityPublisher::OnTimer(IKernel * kernel, s32 beatCount, s64 tick) {
	if (_changed) {
		IArgs<1, 64> args;
		args << _load;
		args.Fix();

		_harbor->Brocast(_protoOverLoad, args.Out());
		_changed = false;
	}
}

