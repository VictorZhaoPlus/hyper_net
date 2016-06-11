#include "AnalysisClient.h"
#include "NodeType.h"
#include "IHarbor.h"
#include "OArgs.h"
#include "CoreProtocol.h"

AnalysisClient * AnalysisClient::s_self = nullptr;
IKernel * AnalysisClient::s_kernel = nullptr;
IHarbor * AnalysisClient::s_harbor = nullptr;

bool AnalysisClient::Initialize(IKernel * kernel) {
    s_self = this;
    s_kernel = kernel;

    return true;
}

bool AnalysisClient::Launched(IKernel * kernel) {
	s_harbor = (IHarbor*)kernel->FindModule("Harbor");
	OASSERT(s_harbor, "where is harbor");

	if (s_harbor->GetNodeType() < node_type::USER)
		return true;

	REGPROTOCOL(core_proto::TEST_DELAY_RESPONE, AnalysisClient::TestDelay);
    return true;
}

bool AnalysisClient::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void AnalysisClient::TestDelay(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args) {
	s_harbor->Send(nodeType, nodeId, core_proto::TEST_DELAY_RESPONE, args);
}
