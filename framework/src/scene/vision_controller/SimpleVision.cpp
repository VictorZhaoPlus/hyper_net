#include "SimpleVision.h"
#include "IObjectMgr.h"
#include "OBuffer.h"
#include "..\Scene.h"

IObject * SimpleVision::FindOrCreate(s64 objectId) {
	auto itr = _units.find(objectId);
	if (itr != _units.end())
		return itr->second;

	IObject * object = CREATE_OBJECT("SceneUnit");
	object->SetPropInt8(OPROP("status"), SV_ST_NONE);
	object->SetPropInt64(OPROP("objectId"), objectId);

	_units[objectId] = object;
	return object;
}

IObject * SimpleVision::Find(s64 objectId) {
	auto itr = _units.find(objectId);
	if (itr != _units.end())
		return itr->second;

	return nullptr;
}

void SimpleVision::OnObjectEnter(IKernel * kernel, IObject * object) {
	s8 state = object->GetPropInt8(OPROP("status"));
	OASSERT(state == SV_ST_NONE || state == SV_ST_DEL || state == SV_ST_NEW_DEL, "wtf");

	if (state == SV_ST_DEL)
		object->SetPropInt8(OPROP("status"), SV_ST_UPDATE);
	else
		object->SetPropInt8(OPROP("status"), SV_ST_NEW);
}

void SimpleVision::OnObjectUpdate(IKernel * kernel, IObject * object) {
}

void SimpleVision::OnObjectLeave(IKernel * kernel, s64 objectId) {
	IObject * object = Find(objectId);
	if (object) {
		s8 state = object->GetPropInt8(OPROP("status"));
		OASSERT(state == SV_ST_NEW || state == SV_ST_UPDATE, "wtf");

		if (state == SV_ST_NEW)
			object->SetPropInt8(OPROP("status"), SV_ST_NEW_DEL);
		else
			object->SetPropInt8(OPROP("status"), SV_ST_DEL);
	}
}

void SimpleVision::OnTimer(IKernel * kernel, s32 beatCount, s64 tick) {
	std::set<IObject *> add;
	std::set<IObject *> remove;
	for (auto itr = _units.begin(); itr != _units.end(); ++itr) {
		IObject * a = itr->second;

		s8 state = a->GetPropInt8(OPROP("status"));
		if (state == SV_ST_NEW)
			add.insert(a);
		else if (state == SV_ST_DEL || state == SV_ST_NEW_DEL)
			remove.insert(a);

		if (state == SV_ST_NEW_DEL)
			continue;
		
		std::vector<IObject *> interest;
		std::vector<IObject *> notInterest;

		for (auto itr2 = _units.begin(); itr2 != _units.end(); ++itr2) {
			if (itr->first != itr2->first) {
				IObject * b = itr2->second;

				s8 stateRhs = b->GetPropInt8(OPROP("status"));
				if (state == SV_ST_NEW_DEL)
					continue;

				if (state == SV_ST_NEW) {
					if (stateRhs != SV_ST_DEL)
						interest.push_back(b);
				}
				else if (state == SV_ST_UPDATE) {
					if (stateRhs == SV_ST_NEW)
						interest.push_back(b);
					else if (stateRhs == SV_ST_DEL)
						notInterest.push_back(b);
				}
				else {
					if (stateRhs != SV_ST_NEW)
						notInterest.push_back(b);
				}
			}
		}

		Scene::Instance()->DealInterest(itr->first, a->GetPropInt32(OPROP("logic")), interest, notInterest);
		Scene::Instance()->DealWatcher(itr->first, a->GetPropInt32(OPROP("logic")), interest, notInterest);
	}

	for (auto * object : add)
		object->SetPropInt8(OPROP("status"), SV_ST_UPDATE);

	for (auto * object : remove) {
		_units.erase(object->GetPropInt64(OPROP("objectId")));

		OMODULE(ObjectMgr)->Recove(object);
	}
}
