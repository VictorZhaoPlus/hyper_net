#include "Analysis.h"
#include "NodeType.h"
#include "CoreProtocol.h"
#include "ICapacitySubscriber.h"
#include "OArgs.h"

bool Analysis::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool Analysis::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);

	if (_harbor->GetNodeType() < node_type::USER) {
		_harbor->AddNodeListener(this, "Analysis");

		RGS_HABOR_ARGS_HANDLER(core_proto::TEST_DELAY_RESPONE, Analysis::TestDelayRespone);

		FIND_MODULE(_capacitySubscriber, CapacitySubscriber);
	}

    return true;
}

bool Analysis::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void Analysis::OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port) {
	if (_nodes[nodeType].find(nodeId) == _nodes[nodeType].end()) {
		_nodes[nodeType][nodeId] = {};
	}
}

void Analysis::TestDelayRespone(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args) {
	DBG_INFO("%d-%d:%lld=>%lld,%d", nodeType, nodeId, args.GetDataInt64(0), tools::GetTimeMillisecond() - args.GetDataInt64(0), _capacitySubscriber->GetOverLoad(nodeType, nodeId));
}

void Analysis::OnTimer(IKernel * kernel, s64 tick) {
	IArgs<1, 64> args;
	args << tick;
	args.Fix();

	for (auto itr = _nodes.begin(); itr != _nodes.end(); ++itr) {
		for (auto itrType = itr->second.begin(); itrType != itr->second.end(); ++itrType)
			_harbor->Send(itr->first, itrType->first, core_proto::TEST_DELAY, args.Out());
	}
}
