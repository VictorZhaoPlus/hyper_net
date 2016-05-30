#include "CapacityPublisher.h"
#include "OArgs.h"
#include "NodeProtocol.h"

CapacityPublisher * CapacityPublisher::s_self = nullptr;
IKernel * CapacityPublisher::s_kernel = nullptr;
IHarbor * CapacityPublisher::s_harbor = nullptr;

s32 CapacityPublisher::s_load = 0;

bool CapacityPublisher::Initialize(IKernel * kernel) {
    s_self = this;
    s_kernel = kernel;

    return true;
}

bool CapacityPublisher::Launched(IKernel * kernel) {
	s_harbor = (IHarbor*)kernel->FindModule("Harbor");
	OASSERT(s_harbor, "where is harbor");
	s_harbor->AddNodeListener(this, "CapacityPublisher");

    return true;
}

bool CapacityPublisher::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void CapacityPublisher::IncreaceLoad(const s32 value) {
	s_load += value;

	IArgs<1, 64> args;
	args << s_load;
	args.Fix();

	s_harbor->Brocast(node_proto::OVER_LOAD, args.Out());
}

void CapacityPublisher::DecreaceLoad(const s32 value) {
	s_load -= value;

	IArgs<1, 64> args;
	args << s_load;
	args.Fix();

	s_harbor->Brocast(node_proto::OVER_LOAD, args.Out());
}

void CapacityPublisher::OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port) {
	IArgs<1, 64> args;
	args << s_load;
	args.Fix();

	s_harbor->Send(nodeType, nodeId, node_proto::OVER_LOAD, args.Out());
}

