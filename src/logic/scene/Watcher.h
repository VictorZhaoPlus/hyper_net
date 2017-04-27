#ifndef __WATCHER_H__
#define __WATCHER_H__
#include "util.h"
#include "IScene.h"
#include "singleton.h"

class OBuffer;
class IHarbor;
class IObjectMgr;
class IProp;
class IEventEngine;
class IProtocolMgr;
class IPacketSender;
class Watcher : public IWatcher, public OHolder<Watcher> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void Brocast(IObject * object, const s32 msgId, const OBuffer& buf, bool self = false);

	virtual void QueryNeighbor(IObject * object, const s32 cmd, const OArgs& args, const std::function<void(IKernel*, IObject * object, const ITargetSet * targets)>& cb);

	void DealInterest(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args);
	void DealWatcher(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args);

	void DisapperWhenDestroy(IKernel * kernel, const void * context, const s32 size);

private:
    IKernel * _kernel;
	IHarbor * _harbor;
	IObjectMgr * _objectMgr;
	IEventEngine * _eventEngine;
	IProtocolMgr * _protocolMgr;
	IPacketSender * _packetSender;

	const IProp * _gate;
	const IProp * _logic;
	const IProp * _type;
	s32 _tableInterest;
	s32 _tableWatcher;
	s32 _settingShare;

	s32 _protoDealInterest;
	s32 _protoDealWatcher;

	s32 _eventSceneObjectDestroy;

	s32 _appearId;
	s32 _disappearId;
};

#endif //__SCENE_H__

