#ifndef __IVISIONCONTROLLER_H__
#define __IVISIONCONTROLLER_H__
#include "IScene.h"

class IObject;
class IVisionController : public ITimer {
public:
	virtual ~IVisionController() {}

	virtual void OnCreate(IObject * scene) = 0;
	virtual IObject * FindOrCreate(s64 objectId) = 0;
	virtual IObject * Find(s64 objectId) = 0;

	virtual void OnObjectEnter(IKernel * kernel, IObject * object) = 0;
	virtual void OnObjectUpdate(IKernel * kernel, IObject * object) = 0;
	virtual void OnObjectLeave(IKernel * kernel, s64 objectId) = 0;

	virtual void OnStart(IKernel * kernel, s64 tick) {}
	virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {}
};

IVisionController * CreateVisionController(const char * type);

#endif //__IVISIONCONTROLLER_H__

