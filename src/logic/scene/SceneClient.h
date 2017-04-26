#ifndef __SCENECLIENT_H__
#define __SCENECLIENT_H__
#include "util.h"
#include "IScene.h"
#include "singleton.h"
#include <unordered_map>

#define MAX_SCENE_LEN 32

class IEventEngine;
class IHarbor;
class IProp;
class IProtocolMgr;
class IObjectMgr;
class IObjectTimer;
class IPacketSender;
class ILogic;
class SceneClient : public ISceneClient, public OHolder<SceneClient> {
	struct Area {
		s8 type;
		std::string scene;
		Position center;
		s16 range;
		AreaCallBack cb;
	};

	struct Scene {
		bool isWild;
		std::unordered_map<s32, Area> areas;
	};

	struct Property {
		const IProp * sceneId;
		const IProp * sceneCopyId;
		const IProp * x;
		const IProp * y;
		const IProp * z;
		const IProp * appeared;
		const IProp * syncTimer;
		const IProp * sync;
		const IProp * gate;
		const IProp * firstAppear;
	};

	struct Proto {
		s32 appear;
		s32 disappear;
		s32 update;
	};

public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual s64 RegisterArea(s8 type, const char * scene, s16 x, s16 y, s16 z, s16 range, const AreaCallBack& f);

	virtual void AppearOn(IObject * object, const char * scene, const Position& pos, const s64 copyId = 0, const bool appear = false);
	virtual void Disappear(IObject * object);
	virtual void SwitchTo(IObject * object, const char * scene, const Position& pos, const s64 copyId = 0);

	virtual Position RandomInRange(const char * scene, const s32 copyId, const Position& start, const Position& end, float radius);
	virtual Position Random(const char * scene, const s32 copyId);

	virtual std::vector<Position> FindPath(const char * scene, const s32 copyId, const Position& start, const Position& end, float radius = 0);
	virtual Position RayCast(const char * scene, const s32 copyId, const Position& start, const Position& end, float radius);

	virtual s32 GetAreaType(IObject * object);

	bool OnRecvEnterScene(IKernel * kernel, IObject * object, const OBuffer& buf);
	bool OnRecvEnterArea(IKernel * kernel, IObject * object, const OBuffer& buf);

	void SendAppearScene(IKernel * kernel, IObject * object);
	void SendDisappearScene(IKernel * kernel, IObject * object);
	void SendUpdateObject(IKernel * kernel, IObject * object);
	void SendSceneInfo(IKernel * kernel, IObject * object);

	s32 DistributeSceneCopy(IKernel * kernel, const char * scene);
	void LeaveSceneCopy(IKernel * kernel, const char * scene, s64 copyId);

	void StartSync(IKernel * kernel, IObject * object);
	void StopSync(IKernel * kernel, IObject * object);
	void OnSyncTick(IKernel * kernel, IObject * object, s32 beatCount, s64 tick);

private:
    IKernel * _kernel;
	IEventEngine * _eventEngine;
	IHarbor * _harbor;
	IProtocolMgr * _protocolMgr;
	IObjectMgr * _objectMgr;
	IObjectTimer * _objectTimer;
	IPacketSender * _packetSender;
	ILogic * _logic;

	s32 _nextAreaId;
	std::unordered_map<std::string, Scene> _scenes;
	s32 _syncInterval;
	s16 _areaCorrect;

	Property _props;
	Proto _proto;

	s32 _clientSceneInfo;

	s32 _eventAppearOnMap;
	s32 _eventDisappearOnMap;
	s32 _eventPrepareSwitchScene;
	s32 _eventSwitchScene;
	s32 _eventPlayerAppear;
	s32 _eventPlayerFirstAppear;

	std::vector<const IProp *> _syncProps;
};

#endif //__SCENE_H__

