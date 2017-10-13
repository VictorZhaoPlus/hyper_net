#ifndef __SIMPLEVISION_H__
#define __SIMPLEVISION_H__
#include "../IVisionController.h"

class SimpleVision : public IVisionController {
	enum {
		SV_ST_NONE = 0,
		SV_ST_NEW,
		SV_ST_UPDATE,
		SV_ST_DEL,
		SV_ST_NEW_DEL,
	};

	struct SceneUnit {
		s8 state;
		std::set<s64> interests;
		std::set<s64> watchers;
	};
public:
	SimpleVision() {}
	virtual ~SimpleVision() {}

	virtual void OnCreate(IObject * scene) {}
	virtual IObject * FindOrCreate(s64 objectId);
	virtual IObject * Find(s64 objectId);

	virtual void OnObjectEnter(IKernel * kernel, IObject * object);
	virtual void OnObjectUpdate(IKernel * kernel, IObject * object);
	virtual void OnObjectLeave(IKernel * kernel, s64 objectId);

	virtual void OnTimer(IKernel * kernel, s32 beatCount, s64 tick);

private:
	std::unordered_map<s64, IObject *> _units;
};

#endif //__SIMPLEVISION_H__

