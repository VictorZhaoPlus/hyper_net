#ifndef __SCENECONTROLLER_H__
#define __SCENECONTROLLER_H__
#include "util.h"
#include "IKernel.h"
using namespace core;

class IObject;
class SceneController : public ITimer {
public:
	SceneController() : _scene(nullptr) {}
	~SceneController() {}

	virtual void OnStart(IKernel * kernel, s64 tick);
	virtual void OnTimer(IKernel * kernel, s32 beatCount, s64 tick);
	virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick);

	void OnCreate(IObject * scene);

	IObject * FindOrCreate(s64 objectId, s8 objectType);
	IObject * Find(s64 objectId);

	void OnObjectEnter(IKernel * kernel, IObject * object);
	void OnObjectUpdate(IKernel * kernel, IObject * object);
	void OnObjectLeave(IKernel * kernel, s64 objectId);

private:
	IObject * _scene;
};

#endif //__SCENECONTROLLER_H__

