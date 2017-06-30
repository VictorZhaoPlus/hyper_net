#include "Analysis.h"
#include "NodeType.h"
#include "ICapacitySubscriber.h"
#include "OArgs.h"
#include "IProtocolMgr.h"

bool Analysis::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool Analysis::Launched(IKernel * kernel) {
	if (OMODULE(Harbor)->GetNodeType() < PROTOCOL_ID("node_type", "user")) {
		OMODULE(Harbor)->AddNodeListener(this, "Analysis");

		RGS_HABOR_ARGS_HANDLER(PROTOCOL_ID("analysis", "test_delay_respone"), Analysis::TestDelayRespone);
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
	DBG_INFO("%d-%d:%lld=>%lld,%d", nodeType, nodeId, args.GetDataInt64(0), tools::GetTimeMillisecond() - args.GetDataInt64(0), OMODULE(CapacitySubscriber)->GetOverLoad(nodeType, nodeId));
}

void Analysis::OnTimer(IKernel * kernel, s32 beatCount, s64 tick) {
	IArgs<1, 64> args;
	args << tick;
	args.Fix();

	for (auto itr = _nodes.begin(); itr != _nodes.end(); ++itr) {
		for (auto itrType = itr->second.begin(); itrType != itr->second.end(); ++itrType)
			OMODULE(Harbor)->Send(itr->first, itrType->first, PROTOCOL_ID("analysis", "test_delay"), args.Out());
	}
}
