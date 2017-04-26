#ifndef __SCENE_H__
#define __SCENE_H__
#include "util.h"
#include "IScene.h"
#include "singleton.h"
#include <unordered_map>

class OArgs;
class IHarbor;
class IObjectMgr;
class IObject;
class ICapacityPublisher;
class IProp;
class SceneController;
class IProtocolMgr;
class Scene : public IScene, public OHolder<Scene> {
	struct Property {
		const IProp * sceneId;
		const IProp * copyId;
		const IProp * controller;
		const IProp * x;
		const IProp * y;
		const IProp * z;
	};

	struct Proto {
		s32 createScene;
		s32 appear;
		s32 disappear;
		s32 update;
		s32 confirmScene;
		s32 recoverScene;
		s32 dealInterest;
		s32 dealWatcher;
	};

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

private:
    IKernel * _kernel;
	IHarbor * _harbor;
	IObjectMgr * _objectMgr;
	IVisibleChecker * _visibleChecker;
	IProtocolMgr * _protocolMgr;

	Property _prop;
	Proto _proto;

	s32 _updateInterval;
};

#endif //__SCENE_H__

