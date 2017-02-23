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

class IScene : public IModule {
public:
	virtual ~IScene() {}
};

typedef std::function<void(IKernel * kernel, const char * scene, s16 x, s16 y, s16 z, s16 range)> AreaCallBack;
class ISceneClient : public IModule {
public:
	virtual ~ISceneClient() {}

	virtual s64 RegisterArea(s8 type, const char * scene, s16 x, s16 y, s16 z, s16 range, const AreaCallBack& f) = 0;
	virtual void SwitchTo(IObject * object, const char * scene, const Position& pos, const s64 copyId = 0) = 0;
	virtual void SwitchTo(IObject * object, IObject * scene, const Position& pos) = 0;

	virtual std::vector<Position> FindPath(IObject * scene, const Position& start, const Position& end, float radius = 0) = 0;
	virtual Position RandomInRange(IObject * scene, const Position& start, const Position& end, float radius) = 0;
	virtual Position RayCast(IObject * scene, const Position& start, const Position& end, float radius) = 0;

	virtual s32 GetAreaType(IObject * object) = 0;
};

enum {
	WSCR_NONE = 0,
	WSCR_ADD,
	WSCR_REPLACE,
};

class ICellVisibleChecker {
public:
	virtual ~ICellVisibleChecker() {}

	virtual bool IsVisiable(IObject * object, IObject * test) = 0;
};

class ICell : public IModule {
public:
	virtual ~ICell() {}

	virtual void SetVisibleChecker(ICellVisibleChecker * checker) = 0;
};

class IWatcherSelector {
public:
	virtual ~IWatcherSelector() {}

	virtual s32 Check(IObject * object, s64 id, s32 type, s64& eliminateId) = 0;
	virtual s64 Pop(IObject * object, s64 id, s32 type) = 0;
};

class IWatcher : public IModule {
public:
	virtual ~IWatcher() {}

	virtual void Brocast(IObject * object, const s32 msgId, const OBuffer& buf, bool self = false) = 0;

	virtual void QueryNeighbor(IObject * object, const std::function<void(IKernel*, IObject * object)>& f) = 0;
	virtual bool IsNeighbor(IObject * object, s64 id) = 0;

	virtual void SetSelector(IWatcherSelector * selector) = 0;
};

#endif /*__ISCENE_H__ */
