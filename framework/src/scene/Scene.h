#ifndef __SCENE_H__
#define __SCENE_H__
#include "util.h"
#include "IScene.h"
#include "singleton.h"
#include <unordered_map>

class OArgs;
class IObject;
class Scene : public IScene, public OHolder<Scene> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void SetVisibleChecker(IVisibleChecker * checker) { _visibleChecker = checker; }

	inline bool IsVisiable(IObject * object, IObject * test) {
		if (_visibleChecker)
			return _visibleChecker->IsVisiable(object, test);
		return true; 
	}

	void CreateScene(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args);

	void EnterScene(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args);
	void LeaveScene(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args);

	void UpdateObject(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args);

	void ReadProps(IKernel * kernel, IObject * object, const OBuffer& args);

	void DealInterest(s64 id, s32 logic, const std::vector<IObject*>& interest, const std::vector<IObject*>& notInterest);
	void DealWatcher(s64 id, s32 logic, const std::vector<IObject*>& interest, const std::vector<IObject*>& notInterest);

private:
    IKernel * _kernel;
	IVisibleChecker * _visibleChecker;

	s32 _updateInterval;
};

#endif //__SCENE_H__

