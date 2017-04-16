#include "SceneClient.h"
#include "IHarbor.h"
#include "IEventEngine.h"
#include "UserNodeType.h"
#include "IObjectMgr.h"
#include "IProtocolMgr.h"
#include "OBuffer.h"
#define MAX_SYNC_SCENE_PACKET_LEN 4096

bool SceneClient::Initialize(IKernel * kernel) {
    _kernel = kernel;

	_nextAreaId = 1;
    return true;
}

bool SceneClient::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);
	if (_harbor->GetNodeType() == user_node_type::LOGIC) {
		FIND_MODULE(_eventEngine, EventEngine);
		FIND_MODULE(_protocolMgr, ProtocolMgr);
		FIND_MODULE(_objectMgr, ObjectMgr);

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
		_areas[_nextAreaId] = { type, scene, { x, y, z }, range, f };
		itr->second.areas.push_back(&_areas[_nextAreaId]);
		++_nextAreaId;
	}
	return 0;
}

void SceneClient::AppearOn(IObject * object, const char * scene, const Position& pos, const s64 copyId, const bool appear) {
	auto itr = _scenes.find(scene);
	OASSERT(itr != _scenes.end(), "wtf");
	OASSERT(object->GetPropInt8(_props.appeared) == 0, "wtf");

	object->SetPropString(_props.sceneId, scene);
	object->SetPropInt16(_props.x, pos.x);
	object->SetPropInt16(_props.y, pos.y);
	object->SetPropInt16(_props.z, pos.z);
	
	if (copyId == 0) {
		OASSERT(itr->second.isWild, "wtf");
		object->SetPropInt64(_props.sceneCopyId, DistributeSceneCopy(_kernel, scene));
	}
	else
		object->SetPropInt64(_props.sceneCopyId, copyId);

	_eventEngine->Exec(_eventAppearOnMap, &object, sizeof(object));

	SendSceneInfo(_kernel, object);
	object->SetPropInt8(_props.appeared, 1);
	StartSync(_kernel, object);

	if (appear)
		SendAppearScene(_kernel, object);
}

void SceneClient::Disappear(IObject * object) {
	if (object->GetPropInt8(_props.appeared)) {
		StopSync(_kernel, object);
		if (object->GetPropInt8(_props.appeared) == 1)
			SendDisappearScene(_kernel, object);

		_eventEngine->Exec(_eventDisappearOnMap, &object, sizeof(object));
	}
}

void SceneClient::SwitchTo(IObject * object, const char * scene, const Position& pos, const s64 copyId) {
	std::string oldScene = object->GetPropString(_props.sceneId);
	auto itrPrev = _scenes.find(oldScene.c_str());
	auto itr = _scenes.find(scene);
	OASSERT(itrPrev != _scenes.end() && itr != _scenes.end(), "wtf");

	scene_event::SwitchScene info;
	info.object = object;
	info.from = { oldScene.c_str(), { object->GetPropInt16(_props.x), object->GetPropInt16(_props.y), object->GetPropInt16(_props.z) } };
	info.to = { scene, pos };

	_eventEngine->Exec(_eventPrepareSwitchScene, &info, sizeof(info));

	if (object->GetPropInt8(_props.appeared) == 1)
		SendDisappearScene(_kernel, object);
	if (itrPrev->second.isWild)
		LeaveSceneCopy(_kernel, oldScene.c_str(), object->GetPropInt64(_props.sceneCopyId));

	object->SetPropString(_props.sceneId, scene);
	object->SetPropInt16(_props.x, pos.x);
	object->SetPropInt16(_props.y, pos.y);
	object->SetPropInt16(_props.z, pos.z);

	if (copyId == 0) {
		OASSERT(itr->second.isWild, "wtf");
		object->SetPropInt64(_props.sceneCopyId, DistributeSceneCopy(_kernel, scene));
	}
	else
		object->SetPropInt64(_props.sceneCopyId, copyId);

	SendSceneInfo(_kernel, object);

	_eventEngine->Exec(_eventSwitchScene, &info, sizeof(info));
}

Position SceneClient::RandomInRange(const char * scene, const s32 copyId, const Position& start, const Position& end, float radius) {

}

Position SceneClient::Random(const char * scene, const s32 copyId) {

}

std::vector<Position> SceneClient::FindPath(const char * scene, const s32 copyId, const Position& start, const Position& end, float radius) {

}

Position SceneClient::RayCast(const char * scene, const s32 copyId, const Position& start, const Position& end, float radius) {

}

s32 SceneClient::GetAreaType(IObject * object) {

}

void SceneClient::SendAppearScene(IKernel * kernel, IObject * object) {
	olib::Buffer<MAX_SYNC_SCENE_PACKET_LEN> buf;
	buf << object->GetPropString(_props.sceneId) << object->GetPropInt64(_props.sceneCopyId) << object->GetID();

	for (const auto * prop : _syncProps) {
		switch (prop->GetType(object)) {
			case DTYPE_INT8: buf << object->GetPropInt8(prop); break;
			case DTYPE_INT16: buf << object->GetPropInt16(prop); break;
			case DTYPE_INT32: buf << object->GetPropInt32(prop); break;
			case DTYPE_INT64: buf << object->GetPropInt64(prop); break;
			case DTYPE_FLOAT: buf << object->GetPropFloat(prop); break;
		}
	}

	OBuffer out = buf.Out();
	_harbor->PrepareSend(user_node_type::SCENEMGR, 1, _proto.appear, out.GetSize());
	_harbor->Send(user_node_type::SCENEMGR, 1, out.GetContext(), out.GetSize());
}

void SceneClient::SendDisappearScene(IKernel * kernel, IObject * object) {
	olib::Buffer<MAX_SYNC_SCENE_PACKET_LEN> buf;
	buf << object->GetPropString(_props.sceneId) << object->GetPropInt64(_props.sceneCopyId) << object->GetID();

	OBuffer out = buf.Out();
	_harbor->PrepareSend(user_node_type::SCENEMGR, 1, _proto.disappear, out.GetSize());
	_harbor->Send(user_node_type::SCENEMGR, 1, out.GetContext(), out.GetSize());
}

void SceneClient::SendUpdateObject(IKernel * kernel, IObject * object) {
	olib::Buffer<MAX_SYNC_SCENE_PACKET_LEN> buf;
	buf << object->GetPropString(_props.sceneId) << object->GetPropInt64(_props.sceneCopyId) << object->GetID();

	for (const auto * prop : _syncProps) {
		switch (prop->GetType(object)) {
		case DTYPE_INT8: buf << object->GetPropInt8(prop); break;
		case DTYPE_INT16: buf << object->GetPropInt16(prop); break;
		case DTYPE_INT32: buf << object->GetPropInt32(prop); break;
		case DTYPE_INT64: buf << object->GetPropInt64(prop); break;
		case DTYPE_FLOAT: buf << object->GetPropFloat(prop); break;
		}
	}

	OBuffer out = buf.Out();
	_harbor->PrepareSend(user_node_type::SCENEMGR, 1, _proto.update, out.GetSize());
	_harbor->Send(user_node_type::SCENEMGR, 1, out.GetContext(), out.GetSize());
}

void SceneClient::SendSceneInfo(IKernel * kernel, IObject * object) {

}

s32 SceneClient::DistributeSceneCopy(IKernel * kernel, const char * scene) {
	return 1;
}

void SceneClient::LeaveSceneCopy(IKernel * kernel, const char * scene, s64 copyId) {

}

void SceneClient::StartSync(IKernel * kernel, IObject * object) {

}

void SceneClient::StopSync(IKernel * kernel, IObject * object) {

}
