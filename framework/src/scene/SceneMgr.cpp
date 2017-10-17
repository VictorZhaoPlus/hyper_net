#include "SceneMgr.h"
#include "OBuffer.h"
#include "OArgs.h"
#include "IIdMgr.h"

bool SceneMgr::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool SceneMgr::Launched(IKernel * kernel) {
	if (OMODULE(Harbor)->GetNodeType() == OID("node_type", "scenemgr")) {
		RGS_HABOR_HANDLER(OID("scene", "appear"), SceneMgr::OnRecvAppear);
		RGS_HABOR_HANDLER(OID("scene", "disappear"), SceneMgr::OnRecvDisappear);
		RGS_HABOR_HANDLER(OID("scene", "update"), SceneMgr::OnRecvUpdate);
		RGS_HABOR_ARGS_HANDLER(OID("scene", "comfirm_scene"), SceneMgr::OnRecvConfirm);
		RGS_HABOR_ARGS_HANDLER(OID("scene", "recover_scene"), SceneMgr::OnRecvRecover);
	}
    return true;
}

bool SceneMgr::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void SceneMgr::OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port) {
	if (nodeType == OID("node_type", "scene")) {
		auto itr = std::find(_nodes.begin(), _nodes.end(), nodeId);
		if (itr == _nodes.end())
			_nodes.push_back(nodeId);
	}
}

void SceneMgr::OnRecvAppear(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer & args) {
	const char * scene = nullptr;
	s64 copyId = 0;
	if (!args.ReadMulti(scene, copyId)) {
		OASSERT(false, "wtf");
		return;
	}

	auto rst = FindOrCreate(kernel, scene, copyId);
	OASSERT(rst.first > 0, "wtf");
	
	IArgs<3, 64> ntf;
	ntf << rst.second << scene << copyId;
	ntf.Fix();
	OMODULE(Harbor)->Send(OID("node_type", "scene"), rst.first, OID("scene", "create_scene"), ntf.Out());

	OBuffer left = args.Left();
	OMODULE(Harbor)->PrepareSend(OID("node_type", "scene"), rst.first, OID("scene", "appear"), sizeof(rst.second) + left.GetSize());
	OMODULE(Harbor)->Send(OID("node_type", "scene"), rst.first, &rst.second, sizeof(rst.second));
	OMODULE(Harbor)->Send(OID("node_type", "scene"), rst.first, left.GetContext(), left.GetSize());
}

void SceneMgr::OnRecvDisappear(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer & args) {
	const char * scene = nullptr;
	s64 copyId = 0;
	if (!args.ReadMulti(scene, copyId)) {
		OASSERT(false, "wtf");
		return;
	}

	auto rst = Find(kernel, scene, copyId);
	OASSERT(rst.first > 0, "wtf");

	OBuffer left = args.Left();
	OMODULE(Harbor)->PrepareSend(OID("node_type", "scene"), rst.first, OID("scene", "disappear"), sizeof(rst.second) + left.GetSize());
	OMODULE(Harbor)->Send(OID("node_type", "scene"), rst.first, &rst.second, sizeof(rst.second));
	OMODULE(Harbor)->Send(OID("node_type", "scene"), rst.first, left.GetContext(), left.GetSize());
}

void SceneMgr::OnRecvUpdate(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer & args) {
	const char * scene = nullptr;
	s64 copyId = 0;
	if (!args.ReadMulti(scene, copyId)) {
		OASSERT(false, "wtf");
		return;
	}

	auto rst = Find(kernel, scene, copyId);
	OASSERT(rst.first > 0, "wtf");

	OBuffer left = args.Left();
	OMODULE(Harbor)->PrepareSend(OID("node_type", "scene"), rst.first, OID("scene", "update"), sizeof(rst.second) + left.GetSize());
	OMODULE(Harbor)->Send(OID("node_type", "scene"), rst.first, &rst.second, sizeof(rst.second));
	OMODULE(Harbor)->Send(OID("node_type", "scene"), rst.first, left.GetContext(), left.GetSize());
}

void SceneMgr::OnRecvConfirm(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	auto& info = _scenes[args.GetDataString(0)][args.GetDataInt64(1)];
	OASSERT(info.distribute == nodeId, "wtf");

	info.real = nodeId;
	info.distribute = 0;
}

void SceneMgr::OnRecvRecover(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	auto& info = _scenes[args.GetDataString(0)][args.GetDataInt64(1)];
	OASSERT(info.real == nodeId, "wtf");

	info.real = 0;
}

std::pair<s32, s64> SceneMgr::FindOrCreate(IKernel * kernel, const char * scene, s64 copyId) {
	auto& info = _scenes[scene][copyId];
	if (info.real > 0)
		return std::make_pair(info.real, info.id);
	else {
		if (info.distribute == 0) {
			if (info.id == 0)
				info.id = OMODULE(IdMgr)->AllocId();
			if (_distributor)
				info.distribute = _distributor->ChooseSceneNode();
			else {
				OASSERT(!_nodes.empty(), "wtf");
				info.distribute = _nodes[rand() % _nodes.size()];
			}
		}
		return std::make_pair(info.distribute, info.id);
	}
}

std::pair<s32, s64> SceneMgr::Find(IKernel * kernel, const char * scene, s64 copyId) {
	auto& info = _scenes[scene][copyId];
	if (info.real > 0)
		return std::make_pair(info.real, info.id);
	return std::make_pair(info.distribute, info.id);
}
