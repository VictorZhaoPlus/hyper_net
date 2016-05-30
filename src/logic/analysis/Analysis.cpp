#include "Analysis.h"
#include "NodeType.h"
#include "NodeProtocol.h"
#include "ICapacitySubscriber.h"
#include "OArgs.h"

Analysis * Analysis::s_self = nullptr;
IKernel * Analysis::s_kernel = nullptr;
IHarbor * Analysis::s_harbor = nullptr;
ICapacitySubscriber * Analysis::s_capacitySubscriber = nullptr;

std::unordered_map<s32, std::unordered_map<s32, Analysis::DataSample>> Analysis::s_nodes;

bool Analysis::Initialize(IKernel * kernel) {
    s_self = this;
    s_kernel = kernel;

    return true;
}

bool Analysis::Launched(IKernel * kernel) {
	s_harbor = (IHarbor*)kernel->FindModule("Harbor");
	OASSERT(s_harbor, "where is harbor");

	if (s_harbor->GetNodeType() < node_type::USER) {
		s_harbor->AddNodeListener(this, "Analysis");

		REGPROTOCOL(node_proto::TEST_DELAY_RESPONE, Analysis::TestDelayRespone);

		s_capacitySubscriber = (ICapacitySubscriber*)kernel->FindModule("CapacitySubscriber");
		OASSERT(s_capacitySubscriber, "where is CapacitySubscriber");
	}

    return true;
}

bool Analysis::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void Analysis::OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port) {
	if (s_nodes[nodeType].find(nodeId) == s_nodes[nodeType].end()) {
		s_nodes[nodeType][nodeId] = {};
	}
}

void Analysis::TestDelayRespone(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args) {
	DBG_INFO("%d-%d:%lld=>%lld,%d", nodeType, nodeId, args.GetDataInt64(0), tools::GetTimeMillisecond() - args.GetDataInt64(0), s_capacitySubscriber->GetOverLoad(nodeType, nodeId));
}

void Analysis::OnTimer(IKernel * kernel, s64 tick) {
	IArgs<1, 64> args;
	args << tick;
	args.Fix();

	for (auto itr = s_nodes.begin(); itr != s_nodes.end(); ++itr) {
		for (auto itrType = itr->second.begin(); itrType != itr->second.end(); ++itrType)
			s_harbor->Send(itr->first, itrType->first, node_proto::TEST_DELAY, args.Out());
	}
}
