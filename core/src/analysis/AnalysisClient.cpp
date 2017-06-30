#include "AnalysisClient.h"
#include "IHarbor.h"
#include "OArgs.h"
#include "IProtocolMgr.h"

bool AnalysisClient::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool AnalysisClient::Launched(IKernel * kernel) {
	if (OMODULE(Harbor)->GetNodeType() < PROTOCOL_ID("node_type", "user"))
		return true;

	RGS_HABOR_ARGS_HANDLER(PROTOCOL_ID("analysis", "test_delay"), AnalysisClient::TestDelay);
    return true;
}

bool AnalysisClient::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void AnalysisClient::TestDelay(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args) {
	OMODULE(Harbor)->Send(nodeType, nodeId, PROTOCOL_ID("analysis", "test_delay_respone"), args);
}
