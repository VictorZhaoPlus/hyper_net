#include "CapacityPublisher.h"
#include "OArgs.h"
#include "CoreProtocol.h"

bool CapacityPublisher::Initialize(IKernel * kernel) {
    _kernel = kernel;
	_load = 0;

    return true;
}

bool CapacityPublisher::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);
	_harbor->AddNodeListener(this, "CapacityPublisher");

    return true;
}

bool CapacityPublisher::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void CapacityPublisher::IncreaceLoad(const s32 value) {
	_load += value;

	IArgs<1, 64> args;
	args << _load;
	args.Fix();

	_harbor->Brocast(core_proto::OVER_LOAD, args.Out());
}

void CapacityPublisher::DecreaceLoad(const s32 value) {
	_load -= value;

	IArgs<1, 64> args;
	args << _load;
	args.Fix();

	_harbor->Brocast(core_proto::OVER_LOAD, args.Out());
}

void CapacityPublisher::OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port) {
	IArgs<1, 64> args;
	args << _load;
	args.Fix();

	_harbor->Send(nodeType, nodeId, core_proto::OVER_LOAD, args.Out());
}

