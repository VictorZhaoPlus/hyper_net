#include "Scene.h"
#include "IHarbor.h"
#include "UserNodeType.h"
#include "OArgs.h"
#include "OBuffer.h"
#include "IObjectMgr.h"
#include "Vision.h"
#include "IProtocolMgr.h"

bool Scene::Initialize(IKernel * kernel) {
    _kernel = kernel;
	_visibleChecker = nullptr;

    return true;
}

bool Scene::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);
	if (_harbor->GetNodeType() == user_node_type::SCENE) {
		FIND_MODULE(_objectMgr, ObjectMgr);
		FIND_MODULE(_protocolMgr, ProtocolMgr);

		_prop.sceneId = _objectMgr->CalcProp("scene_id");
		_prop.copyId = _objectMgr->CalcProp("copy_id");
		_prop.controller = _objectMgr->CalcProp("controller");
		_prop.x = _objectMgr->CalcProp("x");
		_prop.y = _objectMgr->CalcProp("y");
		_prop.z = _objectMgr->CalcProp("z");
		
		_proto.createScene = _protocolMgr->GetId("proto_scene", "create_scene");
		_proto.appear = _protocolMgr->GetId("proto_scene", "appear");
		_proto.disappear = _protocolMgr->GetId("proto_scene", "disappear");
		_proto.update = _protocolMgr->GetId("proto_scene", "update");
		_proto.confirmScene = _protocolMgr->GetId("proto_scene", "comfirm_scene");
		_proto.recoverScene = _protocolMgr->GetId("proto_scene", "recover_scene");
		_proto.dealInterest = _protocolMgr->GetId("proto_scene", "deal_interest");
		_proto.dealWatcher = _protocolMgr->GetId("proto_scene", "deal_watcher");

		RGS_HABOR_ARGS_HANDLER(_proto.createScene, Scene::CreateScene);
		RGS_HABOR_HANDLER(_proto.appear, Scene::EnterScene);
		RGS_HABOR_HANDLER(_proto.disappear, Scene::LeaveScene);
		RGS_HABOR_HANDLER(_proto.update, Scene::UpdateObject);
	}
    return true;
}

bool Scene::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void Scene::CreateScene(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args) {
	s64 id = args.GetDataInt64(0);
	const char * sceneId = args.GetDataString(1);
	s32 copyId = args.GetDataInt32(2);

	IArgs<2, 64> ntf;
	ntf << sceneId << copyId;
	ntf.Fix();
	_harbor->Send(nodeType, nodeId, _proto.confirmScene, ntf.Out());


	if (_objectMgr->FindObject(id))
		return;

	IObject * scene = CREATE_OBJECT_BYID(_objectMgr, "Scene", id);
	OASSERT(scene, "wtf");

	scene->SetPropString(_prop.sceneId, sceneId);
	scene->SetPropInt32(_prop.copyId, copyId);

	Vision * controller = NEW Vision();
	scene->SetPropInt64(_prop.controller, (s64)controller);
	controller->OnCreate(scene);

	START_TIMER(controller, 0, TIMER_BEAT_FOREVER, _updateInterval);
}

void Scene::EnterScene(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args) {
	s64 sceneId = 0;
	s64 objectId = 0;
	if (!args.ReadMulti(sceneId, objectId)) {
		OASSERT(false, "wtf");
		return;
	}

	IObject * scene = _objectMgr->FindObject(sceneId);
	OASSERT(scene, "wtf");
	if (scene) {
		Vision * controller = (Vision *)scene->GetPropInt64(_prop.controller);
		OASSERT(controller, "wtf");

		IObject * object = controller->FindOrCreate(objectId);
		OASSERT(object, "wtf");

		ReadProps(_kernel, object, args);
		controller->OnObjectEnter(kernel, object);
	}
}

void Scene::LeaveScene(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args) {
	s64 sceneId = 0;
	s64 objectId = 0;
	if (!args.ReadMulti(sceneId, objectId)) {
		OASSERT(false, "wtf");
		return;
	}

	IObject * scene = _objectMgr->FindObject(sceneId);
	OASSERT(scene, "wtf");
	if (scene) {
		Vision * controller = (Vision *)scene->GetPropInt64(_prop.controller);
		OASSERT(controller, "wtf");

		controller->OnObjectLeave(kernel, objectId);
	}
}

void Scene::UpdateObject(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args) {
	s64 sceneId = 0;
	s64 objectId = 0;
	if (!args.ReadMulti(sceneId, objectId)) {
		OASSERT(false, "wtf");
		return;
	}

	IObject * scene = _objectMgr->FindObject(sceneId);
	OASSERT(scene, "wtf");
	if (scene) {
		Vision * controller = (Vision *)scene->GetPropInt64(_prop.controller);
		IObject * object = controller->Find(objectId);
		OASSERT(object, "wtf");

		ReadProps(_kernel, object, args);
		controller->OnObjectUpdate(kernel, object);
	}
}

void Scene::ReadProps(IKernel * kernel, IObject * object, const OBuffer& args) {
	s16 x, y, z;
	if (!args.ReadMulti(x, y, z)) {
		OASSERT(false, "wtf");
		return;
	}

	object->SetPropInt16(_prop.x, x);
	object->SetPropInt16(_prop.y, y);
	object->SetPropInt16(_prop.z, z);

	s8 count = 0;
	if (!args.Read(count)) {
		OASSERT(false, "wtf");
		return;
	}

	for (s32 i = 0; i < count; ++i) {
		s32 name = 0;
		if (!args.Read(name)) {
			OASSERT(false, "wtf");
			return;
		}

		const IProp * prop = _objectMgr->CalcProp(name);
		OASSERT(prop, "wtf");

		switch (prop->GetType(object)) {
		case DTYPE_INT8: {
				s8 val = 0;
				if (!args.Read(val)) {
					OASSERT(false, "wtf");
					return;
				}
				object->SetPropInt8(prop, val);
			}
			break;
		case DTYPE_INT16: {
				s16 val = 0;
				if (!args.Read(val)) {
					OASSERT(false, "wtf");
					return;
				}
				object->SetPropInt16(prop, val);
			}
			break;
		case DTYPE_INT32: {
				s32 val = 0;
				if (!args.Read(val)) {
					OASSERT(false, "wtf");
					return;
				}
				object->SetPropInt32(prop, val);
			}
			break;
		case DTYPE_INT64: {
				s64 val = 0;
				if (!args.Read(val)) {
					OASSERT(false, "wtf");
					return;
				}
				object->SetPropInt64(prop, val);
			}
			break;
		case DTYPE_FLOAT: {
				float val = 0.f;
				if (!args.Read(val)) {
					OASSERT(false, "wtf");
					return;
				}
				object->SetPropFloat(prop, val);
			}
			break;
		}
	}
}

