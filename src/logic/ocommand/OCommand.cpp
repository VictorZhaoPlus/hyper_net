#include "OCommand.h"
#include "IObjectMgr.h"
#include "IProtocolMgr.h"
#include "OArgs.h"
#include "OStream.h"
#include "IHarbor.h"

bool OCommand::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool OCommand::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);
	FIND_MODULE(_objectMgr, ObjectMgr);
	FIND_MODULE(_protocolMgr, ProtocolMgr);

    return true;
}

bool OCommand::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void OCommand::Command(s32 cmd, const IObject * sender, const s64 reciever, const OArgs& args, const CommandFailCB& fail) {

}

bool OCommand::CommandTo(const s64 reciver, const s32 cmd, const OArgs & args) {

}

void OCommand::OnCommand(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& stream) {

}

void OCommand::OnForwardCommand(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& stream) {

}

