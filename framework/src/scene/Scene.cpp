#include "Scene.h"
#include "IHarbor.h"
#include "OArgs.h"
#include "OBuffer.h"
#include "IObjectMgr.h"
#include "IProtocolMgr.h"
#include "IVisionController.h"

bool Scene::Initialize(IKernel * kernel) {
    _kernel = kernel;
	_visibleChecker = nullptr;

    return true;
}

bool Scene::Launched(IKernel * kernel) {
	if (OMODULE(Harbor)->GetNodeType() == PROTOCOL_ID("node_type", "scene")) {
		RGS_HABOR_ARGS_HANDLER(PROTOCOL_ID("scene", "create_scene"), Scene::CreateScene);
		RGS_HABOR_HANDLER(PROTOCOL_ID("scene", "appear"), Scene::EnterScene);
		RGS_HABOR_HANDLER(PROTOCOL_ID("scene", "disappear"), Scene::LeaveScene);
		RGS_HABOR_HANDLER(PROTOCOL_ID("scene", "update"), Scene::UpdateObject);

		OMODULE(ObjectMgr)->ExtendT<std::set<s64>>("SceneUnit", "interest");
		OMODULE(ObjectMgr)->ExtendT<std::set<s64>>("SceneUnit", "watcher");
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
	OMODULE(Harbor)->Send(nodeType, nodeId, PROTOCOL_ID("scene", "comfirm_scene"), ntf.Out());


	if (OMODULE(ObjectMgr)->FindObject(id))
		return;

	IObject * scene = CREATE_OBJECT_BYID("Scene", id);
	OASSERT(scene, "wtf");

	scene->SetPropString(OPROP("sceneId"), sceneId);
	scene->SetPropInt32(OPROP("copyId"), copyId);

	IVisionController * controller = CreateVisionController("simple");
	scene->SetPropInt64(OPROP("controller"), (s64)controller);
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

	IObject * scene = OMODULE(ObjectMgr)->FindObject(sceneId);
	OASSERT(scene, "wtf");
	if (scene) {
		IVisionController * controller = (IVisionController *)scene->GetPropInt64(OPROP("controller"));
		OASSERT(controller, "wtf");

		IObject * object = controller->FindOrCreate(objectId);
		OASSERT(object, "wtf");
		object->SetPropInt32(OPROP("logic"), nodeId);

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

	IObject * scene = OMODULE(ObjectMgr)->FindObject(sceneId);
	OASSERT(scene, "wtf");
	if (scene) {
		IVisionController * controller = (IVisionController *)scene->GetPropInt64(OPROP("controller"));
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

	IObject * scene = OMODULE(ObjectMgr)->FindObject(sceneId);
	OASSERT(scene, "wtf");
	if (scene) {
		IVisionController * controller = (IVisionController *)scene->GetPropInt64(OPROP("controller"));
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

	object->SetPropInt16(OPROP("x"), x);
	object->SetPropInt16(OPROP("y"), y);
	object->SetPropInt16(OPROP("z"), z);

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

		const IProp * prop = OMODULE(ObjectMgr)->CalcProp(name);
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

void Scene::DealInterest(s64 id, s32 logic, const std::vector<IObject*>& interest, const std::vector<IObject*>& notInterest) {
	olib::Buffer<4096> buf;

	buf << id << (s32)interest.size();
	for (auto * b : interest)
		buf << b->GetPropInt64(OPROP("objectId")) << b->GetPropInt32(OPROP("logic"));

	buf << (s32)notInterest.size();
	for (auto * b : interest)
		buf << b->GetPropInt64(OPROP("objectId"));

	OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "logic"), logic, PROTOCOL_ID("scene", "deal_interest"), buf.Out());
}

void Scene::DealWatcher(s64 id, s32 logic, const std::vector<IObject*>& interest, const std::vector<IObject*>& notInterest) {
	olib::Buffer<4096> buf;

	buf << id << (s32)interest.size();
	for (auto * b : interest)
		buf << b->GetPropInt64(OPROP("objectId")) << b->GetPropInt32(OPROP("gate")) << b->GetPropInt32(OPROP("logic"));

	buf << (s32)notInterest.size();
	for (auto * b : interest) 
		buf << b->GetPropInt64(OPROP("objectId"));

	OMODULE(Harbor)->Send(PROTOCOL_ID("node_type", "logic"), logic, PROTOCOL_ID("scene", "deal_watcher"), buf.Out());
}

