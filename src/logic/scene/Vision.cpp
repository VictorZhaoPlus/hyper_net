#include "Vision.h"
#include "IObjectMgr.h"

void Vision::OnStart(IKernel * kernel, s64 tick) {

}

void Vision::OnTimer(IKernel * kernel, s32 beatCount, s64 tick) {

}

void Vision::OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {

}

void Vision::OnCreate(IObject * scene) {
	_scene = scene;
}

IObject * Vision::FindOrCreate(s64 objectId) {

}

IObject * Vision::Find(s64 objectId) {

}

void Vision::OnObjectEnter(IKernel * kernel, IObject * object) {

}

void Vision::OnObjectUpdate(IKernel * kernel, IObject * object) {

}

void Vision::OnObjectLeave(IKernel * kernel, s64 objectId) {

}