#include "ObjectLocator.h"
#include "IHarbor.h"
#include "IProtocolMgr.h"
#include "OArgs.h"

bool ObjectLocator::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool ObjectLocator::Launched(IKernel * kernel) {
	if (OMODULE(Harbor)->GetNodeType() == PROTOCOL_ID("node_type", "relation")) {
		RGS_HABOR_ARGS_HANDLER(PROTOCOL_ID("object_locator", "report"), ObjectLocator::OnReport);
	}

    return true;
}

bool ObjectLocator::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void ObjectLocator::Report(s64 id, s32 gate, s32 logic) {
	IArgs<3, 64> args;
	args << id << gate << logic;
	args.Fix();

	OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "relation"), 1, PROTOCOL_ID("object_locator", "report"), args.Out());
}

void ObjectLocator::Erase(s64 id) {
	IArgs<3, 64> args;
	args << id << 0 << 0;
	args.Fix();

	OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "relation"), 1, PROTOCOL_ID("object_locator", "report"), args.Out());
}

void ObjectLocator::OnReport(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args) {
	s64 id = args.GetDataInt64(0);
	s32 gate = args.GetDataInt32(1);
	s32 logic = args.GetDataInt32(2);

	if (gate == 0 && logic == 0)
		_objects.erase(id);
	else
		_objects[id] = { gate, logic };
}

s32 ObjectLocator::FindObjectLogic(s64 id) {
	auto itr = _objects.find(id);
	if (itr != _objects.end())
		return itr->second.logic;
	return 0;
}

s32 ObjectLocator::FindObjectGate(s64 id) {
	auto itr = _objects.find(id);
	if (itr != _objects.end())
		return itr->second.gate;
	return 0;
}


