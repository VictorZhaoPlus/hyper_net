#ifndef __SCENECLIENT_H__
#define __SCENECLIENT_H__
#include "util.h"
#include "IScene.h"
#include "singleton.h"
#include <unordered_map>

#define MAX_SCENE_LEN 32

class IEventEngine;
class IHarbor;
class SceneClient : public ISceneClient, public OHolder<SceneClient> {
	struct Area {
		std::string scene;
		Position center;
		s16 range;
		AreaCallBack f;
	};

	struct Scene {
		std::vector<Area*> areas;
	};

public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual s64 RegisterArea(s8 type, const char * scene, s16 x, s16 y, s16 z, s16 range, const AreaCallBack& f);
	virtual void SwitchTo(IObject * object, const char * scene, const Position& pos, const s64 copyId = 0);
	virtual void SwitchTo(IObject * object, IObject * scene, const Position& pos);

	virtual std::vector<Position> FindPath(IObject * scene, const Position& start, const Position& end, float radius = 0);
	virtual Position RandomInRange(IObject * scene, const Position& start, const Position& end, float radius);
	virtual Position RayCast(IObject * scene, const Position& start, const Position& end, float radius);

	virtual s32 GetAreaType(IObject * object);

private:
    IKernel * _kernel;
	IEventEngine * _event;
	IHarbor * _harbor;

	std::unordered_map<s32, Area> _areas;
	std::unordered_map<std::string, Scene> _scenes;
};

#endif //__SCENE_H__

