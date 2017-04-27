/*
 * File: IScene.h
 * Author: ooeyusea
 *
 * Created On January 10, 2017, 09:14 AM
 */

#ifndef __ISCENE_H__
#define __ISCENE_H__
#include "IModule.h"
#include <vector>

struct Position {
	s16 x;
	s16 y;
	s16 z;
};
class IObject;
class OBuffer;
class OArgs;

typedef std::function<void(IKernel * kernel, IObject * object)> AreaCallBack;
class ISceneClient : public IModule {
public:
	virtual ~ISceneClient() {}

	virtual s64 RegisterArea(s8 type, const char * scene, s16 x, s16 y, s16 z, s16 range, const AreaCallBack& f) = 0;

	virtual void AppearOn(IObject * object, const char * scene, const Position& pos, const s64 copyId = 0, const bool appear = false) = 0;
	virtual void Disappear(IObject * object) = 0;
	virtual void SwitchTo(IObject * object, const char * scene, const Position& pos, const s64 copyId = 0) = 0;

	virtual Position RandomInRange(const char * scene, const s32 copyId, const Position& start, float radius) = 0;
	virtual Position Random(const char * scene, const s32 copyId) = 0;

	virtual std::vector<Position> FindPath(const char * scene, const s32 copyId, const Position& start, const Position& end, float radius = 0) = 0;
	virtual Position RayCast(const char * scene, const s32 copyId, const Position& start, const Position& end, float radius) = 0;

	virtual s32 GetAreaType(IObject * object) = 0;
};

class ISceneDistributor {
public:
	virtual ~ISceneDistributor() {}

	virtual s32 ChooseSceneNode() = 0;
};

class ISceneMgr : public IModule {
public:
	virtual ~ISceneMgr() {}

	virtual void SetDistributor(ISceneDistributor * distributor) = 0;
};

class IVisibleChecker {
public:
	virtual ~IVisibleChecker() {}

	virtual bool IsVisiable(IObject * object, IObject * test) = 0;
};

class IScene : public IModule {
public:
	virtual ~IScene() {}

	virtual void SetVisibleChecker(IVisibleChecker * checker) = 0;
};

class ITargetSet {
public:
	virtual ~ITargetSet() {}

	virtual s32 Count() const = 0;
	virtual s64 GetTarget(const s32 idx) const = 0;
};

class IWatcher : public IModule {
public:
	virtual ~IWatcher() {}

	virtual void Brocast(IObject * object, const s32 msgId, const OBuffer& buf, bool self = false) = 0;

	virtual void QueryNeighbor(IObject * object, const s32 cmd, const OArgs& args, const std::function<void(IKernel*, IObject * object, const ITargetSet * targets)>& cb) = 0;
};

namespace scene_event {
	struct ScenePos {
		const char * scene;
		Position pos;
	};

	struct SwitchScene {
		IObject * object;
		ScenePos from;
		ScenePos to;
	};
}

#endif /*__ISCENE_H__ */
