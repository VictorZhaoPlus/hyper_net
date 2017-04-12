#include "SceneController.h"
#include "IObjectMgr.h"

void SceneController::OnStart(IKernel * kernel, s64 tick) {

}

void SceneController::OnTimer(IKernel * kernel, s32 beatCount, s64 tick) {

}

void SceneController::OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {

}

void SceneController::OnCreate(IObject * scene) {
	_scene = scene;
}

IObject * SceneController::FindOrCreate(s64 objectId, s8 objectType) {

}

IObject * SceneController::Find(s64 objectId) {

}

void SceneController::OnObjectEnter(IKernel * kernel, IObject * object) {

}

void SceneController::OnObjectUpdate(IKernel * kernel, IObject * object) {

}

void SceneController::OnObjectLeave(IKernel * kernel, s64 objectId) {

}