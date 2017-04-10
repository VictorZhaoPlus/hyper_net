#ifndef __WATCHER_H__
#define __WATCHER_H__
#include "util.h"
#include "IScene.h"
#include "singleton.h"

class OArgs;
class IHarbor;
class IObjectMgr;
class IProp;
class IEventEngine;
class IProtocolMgr;
class IPacketSender;
class IOCommand;
class IShadowMgr;
class Watcher : public IWatcher, public OHolder<Watcher> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void Brocast(IObject * object, const s32 msgId, const OBuffer& buf, bool self = false);

	virtual void QueryNeighbor(IObject * object, const std::function<void(IKernel*, IObject * object)>& f);
	virtual bool IsNeighbor(IObject * object, s64 id);

	virtual s32 Check(IObject * object, s64 id, s32 type, s64& eliminateId);
	virtual s64 Pop(IObject * object, s64 id, s32 type);

	void AddInterest(IKernel * kernel, const s64 sender, IObject * reciever, const OArgs& args);
	void RemoveInterest(IKernel * kernel, const s64 sender, IObject * reciever, const OArgs& args);

	void AddWatcher(IKernel * kernel, const s64 sender, IObject * reciever, const OArgs& args);
	void RemoveWatcher(IKernel * kernel, const s64 sender, IObject * reciever, const OArgs& args);

	void DisapperWhenDestroy(IKernel * kernel, const void * context, const s32 size);
	void RemoveAllInterestWhenDestroy(IKernel * kernel, const void * context, const s32 size);

private:
    IKernel * _kernel;
	IHarbor * _harbor;
	IObjectMgr * _objectMgr;
	IEventEngine * _eventEngine;
	IProtocolMgr * _protocolMgr;
	IPacketSender * _packetSender;
	IOCommand * _command;
	IShadowMgr * _shadowMgr;

	const IProp * _gate;
	const IProp * _logic;
	const IProp * _type;
	s32 _tableInterest;
	s32 _tableWatcher;
	s32 _settingShare;

	s32 _eventSceneObjectDestroy;

	s32 _appearId;
	s32 _disappearId;
};

#endif //__SCENE_H__

