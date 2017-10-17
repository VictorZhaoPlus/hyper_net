#include "SceneClient.h"
#include "IHarbor.h"
#include "IEventEngine.h"
#include "IObjectMgr.h"
#include "IObjectTimer.h"
#include "OBuffer.h"
#include "ILogin.h"

#define MAX_SYNC_SCENE_PACKET_LEN 4096

bool SceneClient::Initialize(IKernel * kernel) {
    _kernel = kernel;

	_nextAreaId = 1;
    return true;
}

bool SceneClient::Launched(IKernel * kernel) {
	if (OMODULE(Harbor)->GetNodeType() == OID("node_type", "logic")) {
		RGS_PROTOCOL_HANDLER(OID("cli_scene", "enter_scene"), SceneClient::OnRecvEnterScene);
		RGS_PROTOCOL_HANDLER(OID("cli_scene", "enter_area"), SceneClient::OnRecvEnterArea);

		_syncSceneSetting = OMODULE(ObjectMgr)->CalcPropSetting("scene");
	}
    return true;
}

bool SceneClient::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

s64 SceneClient::RegisterArea(s8 type, const char * scene, s16 x, s16 y, s16 z, s16 range, const AreaCallBack& f) {
	auto itr = _scenes.find(scene);
	OASSERT(itr != _scenes.end(), "wtf");
	if (itr != _scenes.end()) {
		itr->second.areas[_nextAreaId] = { type, scene, { x, y, z }, range, f };
		++_nextAreaId;
	}
	return 0;
}

void SceneClient::AppearOn(IObject * object, const char * scene, const Position& pos, const s64 copyId, const bool appear) {
	auto itr = _scenes.find(scene);
	OASSERT(itr != _scenes.end(), "wtf");
	OASSERT(object->GetPropInt8(OPROP("appeared")) == 0, "wtf");

	object->SetPropString(OPROP("sceneId"), scene);
	object->SetPropInt16(OPROP("x"), pos.x);
	object->SetPropInt16(OPROP("y"), pos.y);
	object->SetPropInt16(OPROP("z"), pos.z);
	
	if (copyId == 0) {
		OASSERT(itr->second.isWild, "wtf");
		object->SetPropInt64(OPROP("sceneCopyId"), DistributeSceneCopy(_kernel, scene));
	}
	else
		object->SetPropInt64(OPROP("sceneCopyId"), copyId);

	OMODULE(EventEngine)->Exec(OID("evt_scene", "appear_on_map"), &object, sizeof(object));

	SendSceneInfo(_kernel, object);
	object->SetPropInt8(OPROP("appeared"), 1);
	StartSync(_kernel, object);

	if (appear)
		SendAppearScene(_kernel, object);
}

void SceneClient::Disappear(IObject * object) {
	if (object->GetPropInt8(OPROP("appeared"))) {
		StopSync(_kernel, object);
		if (object->GetPropInt8(OPROP("appeared")) == 1)
			SendDisappearScene(_kernel, object);

		OMODULE(EventEngine)->Exec(OID("evt_scene", "disappear_from_map"), &object, sizeof(object));
	}
}

void SceneClient::SwitchTo(IObject * object, const char * scene, const Position& pos, const s64 copyId) {
	std::string oldScene = object->GetPropString(OPROP("sceneId"));
	auto itrPrev = _scenes.find(oldScene.c_str());
	auto itr = _scenes.find(scene);
	OASSERT(itrPrev != _scenes.end() && itr != _scenes.end(), "wtf");

	scene_event::SwitchScene info;
	info.object = object;
	info.from = { oldScene.c_str(), { object->GetPropInt16(OPROP("x")), object->GetPropInt16(OPROP("y")), object->GetPropInt16(OPROP("z")) } };
	info.to = { scene, pos };

	OMODULE(EventEngine)->Exec(OID("evt_scene", "prepare_switch_scene"), &info, sizeof(info));

	if (object->GetPropInt8(OPROP("appeared")) == 1)
		SendDisappearScene(_kernel, object);
	if (itrPrev->second.isWild)
		LeaveSceneCopy(_kernel, oldScene.c_str(), object->GetPropInt64(OPROP("sceneCopyId")));

	object->SetPropString(OPROP("sceneId"), scene);
	object->SetPropInt16(OPROP("x"), pos.x);
	object->SetPropInt16(OPROP("y"), pos.y);
	object->SetPropInt16(OPROP("z"), pos.z);

	if (copyId == 0) {
		OASSERT(itr->second.isWild, "wtf");
		object->SetPropInt64(OPROP("sceneCopyId"), DistributeSceneCopy(_kernel, scene));
	}
	else
		object->SetPropInt64(OPROP("sceneCopyId"), copyId);

	SendSceneInfo(_kernel, object);

	OMODULE(EventEngine)->Exec(OID("evt_scene", "switch_scene"), &info, sizeof(info));
}

Position SceneClient::RandomInRange(const char * scene, const s32 copyId, const Position& start, float radius) {
	return start;
}

Position SceneClient::Random(const char * scene, const s32 copyId) {
	return { 0,0,0 };
}

std::vector<Position> SceneClient::FindPath(const char * scene, const s32 copyId, const Position& start, const Position& end, float radius) {
	std::vector<Position> ret;
	ret.push_back(start);
	ret.push_back(end);
	return std::move(ret);
}

Position SceneClient::RayCast(const char * scene, const s32 copyId, const Position& start, const Position& end, float radius) {
	return end;
}

s32 SceneClient::GetAreaType(IObject * object) {
	return 0;
}

bool SceneClient::OnRecvEnterScene(IKernel * kernel, IObject * object, const OBuffer& buf) {
	if (object->GetPropInt8(OPROP("appeared")) == 0) {
		SendAppearScene(kernel, object);
		object->SetPropInt8(OPROP("appeared"), 1);

		OMODULE(EventEngine)->Exec(OID("evt_scene", "player_appear"), &object, sizeof(object));
		if (object->GetPropInt8(OPROP("firstAppear")) == 0) {
			object->SetPropInt8(OPROP("firstAppear"), 1);
			OMODULE(EventEngine)->Exec(OID("evt_scene", "player_first_appear"), &object, sizeof(object));
		}
	}
	return true;
}

bool SceneClient::OnRecvEnterArea(IKernel * kernel, IObject * object, const OBuffer& buf) {
	s32 idx = 0;
	if (!buf.Read(idx))
		return false;

	auto itr = _scenes.find(object->GetPropString(OPROP("sceneId")));
	OASSERT(itr != _scenes.end(), "wtf");
	if (itr != _scenes.end()) {
		auto itrArea = itr->second.areas.find(idx);
		if (itrArea != itr->second.areas.end()) {
			//if (math::CalcDistance(object->GetPropInt16(OPROP("x")), object->GetPropInt16(OPROP("y")), object->GetPropInt16(OPROP("z))
			//	, itrArea->second.center.x, itrArea->second.center.y, itrArea->second.center.z) < itrArea->second.range + _areaCorrect) {
			//	itrArea->second.cb(kernel, object);
			//}
		}
	}
	return true;
}

void SceneClient::SendAppearScene(IKernel * kernel, IObject * object) {
	olib::Buffer<MAX_SYNC_SCENE_PACKET_LEN> buf;
	buf << object->GetPropString(OPROP("sceneId")) << object->GetPropInt64(OPROP("sceneCopyId")) << object->GetID() 
		<< object->GetPropInt16(OPROP("x")) << object->GetPropInt16(OPROP("y")) << object->GetPropInt16(OPROP("z"));
	s8& count = *buf.Reserve<s8>();
	for (auto * prop : object->GetPropsInfo()) {
		if (prop->GetSetting(object) & _syncSceneSetting) {
			switch (prop->GetType(object)) {
			case DTYPE_INT8: buf << object->GetPropInt8(prop); break;
			case DTYPE_INT16: buf << object->GetPropInt16(prop); break;
			case DTYPE_INT32: buf << object->GetPropInt32(prop); break;
			case DTYPE_INT64: buf << object->GetPropInt64(prop); break;
			case DTYPE_FLOAT: buf << object->GetPropFloat(prop); break;
			}
			++count;
		}
	}

	OBuffer out = buf.Out();
	OMODULE(Harbor)->PrepareSend(OID("node_type", "scenemgr"), 1, OID("scene", "appear"), out.GetSize());
	OMODULE(Harbor)->Send(OID("node_type", "scenemgr"), 1, out.GetContext(), out.GetSize());
}

void SceneClient::SendDisappearScene(IKernel * kernel, IObject * object) {
	olib::Buffer<MAX_SYNC_SCENE_PACKET_LEN> buf;
	buf << object->GetPropString(OPROP("sceneId")) << object->GetPropInt64(OPROP("sceneCopyId")) << object->GetID();

	OBuffer out = buf.Out();
	OMODULE(Harbor)->PrepareSend(OID("node_type", "scenemgr"), 1, OID("scene", "disappear"), out.GetSize());
	OMODULE(Harbor)->Send(OID("node_type", "scenemgr"), 1, out.GetContext(), out.GetSize());
}

void SceneClient::SendUpdateObject(IKernel * kernel, IObject * object) {
	olib::Buffer<MAX_SYNC_SCENE_PACKET_LEN> buf;
	buf << object->GetPropString(OPROP("sceneId")) << object->GetPropInt64(OPROP("sceneCopyId")) << object->GetID()
		<< object->GetPropInt16(OPROP("x")) << object->GetPropInt16(OPROP("y")) << object->GetPropInt16(OPROP("z"));
	s8& count = *buf.Reserve<s8>();
	for (auto * prop : object->GetPropsInfo()) {
		if (prop->GetSetting(object) & _syncSceneSetting) {
			switch (prop->GetType(object)) {
			case DTYPE_INT8: buf << object->GetPropInt8(prop); break;
			case DTYPE_INT16: buf << object->GetPropInt16(prop); break;
			case DTYPE_INT32: buf << object->GetPropInt32(prop); break;
			case DTYPE_INT64: buf << object->GetPropInt64(prop); break;
			case DTYPE_FLOAT: buf << object->GetPropFloat(prop); break;
			}
			++count;
		}
	}

	OBuffer out = buf.Out();
	OMODULE(Harbor)->PrepareSend(OID("node_type", "scenemgr"), 1, OID("scene", "update"), out.GetSize());
	OMODULE(Harbor)->Send(OID("node_type", "scenemgr"), 1, out.GetContext(), out.GetSize());
}

void SceneClient::SendSceneInfo(IKernel * kernel, IObject * object) {
	olib::Buffer<MAX_SYNC_SCENE_PACKET_LEN> buf;
	buf << object->GetPropString(OPROP("sceneId")) << object->GetPropInt64(OPROP("sceneCopyId"));
	buf << object->GetPropInt16(OPROP("x")) << object->GetPropInt16(OPROP("y")) << object->GetPropInt16(OPROP("z"));

	OMODULE(PacketSender)->Send(object->GetPropInt32(OPROP("gate")), object->GetID(), OID("cli_scene", "scene_info"), buf.Out());
}

s32 SceneClient::DistributeSceneCopy(IKernel * kernel, const char * scene) {
	return 1;
}

void SceneClient::LeaveSceneCopy(IKernel * kernel, const char * scene, s64 copyId) {

}

void SceneClient::StartSync(IKernel * kernel, IObject * object) {
	OMODULE(ObjectTimer)->Start(object, OPROP("syncTimer"), 0, TIMER_BEAT_FOREVER, _syncInterval, __FILE__, __LINE__, nullptr, std::bind(&SceneClient::OnSyncTick, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), nullptr);
}

void SceneClient::StopSync(IKernel * kernel, IObject * object) {
	OMODULE(ObjectTimer)->Stop(object, OPROP("syncTimer"));
}

void SceneClient::OnSyncTick(IKernel * kernel, IObject * object, s32 beatCount, s64 tick) {
	if (object->GetPropInt8(OPROP("sync"))) {
		object->SetPropInt8(OPROP("sync"), 0);

		SendUpdateObject(kernel, object);
	}
}
