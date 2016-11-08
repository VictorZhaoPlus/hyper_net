#include "AnalysisClient.h"
#include "NodeType.h"
#include "IHarbor.h"
#include "OArgs.h"
#include "CoreProtocol.h"

bool AnalysisClient::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool AnalysisClient::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);

	if (_harbor->GetNodeType() < node_type::USER)
		return true;

	RGS_HABOR_ARGS_HANDLER(core_proto::TEST_DELAY_RESPONE, AnalysisClient::TestDelay);
    return true;
}

bool AnalysisClient::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void AnalysisClient::TestDelay(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args) {
	_harbor->Send(nodeType, nodeId, core_proto::TEST_DELAY_RESPONE, args);
}
