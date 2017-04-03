#ifndef __OCOMMAND_H__
#define __OCOMMAND_H__
#include "util.h"
#include "IOCommand.h"
#include "singleton.h"
#include "OCallback.h"

class IObject;
class OArgs;
class IObjectMgr;
class IProtocolMgr;
class OBuffer;
class IHarbor;
class OCommand : public IOCommand, public ITimer, public OHolder<OCommand> {
	typedef olib::CallbackType<s32, CommandCB>::type CMD_CB_POOL;
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void RegisterCmd(s32 cmd, const CommandCB& handler, const char * debug) { _cbs.Register(cmd, handler, debug); }
	virtual void Command(s32 cmd, const IObject * sender, const s64 reciever, const OArgs& args, const CommandFailCB& fail);

	//can't use in logic
	virtual bool CommandTo(const s64 reciver, const s32 cmd, const OArgs & args);

	void OnCommand(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& stream);
	void OnForwardCommand(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& stream);

	virtual void OnStart(IKernel * kernel, s64 tick) {}
	virtual void OnTimer(IKernel * kernel, s32 beatCount, s64 tick);
	virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {}

private:
    IKernel * _kernel;
	IHarbor * _harbor;
	IObjectMgr * _objectMgr;
	IProtocolMgr * _protocolMgr;

	CMD_CB_POOL _cbs;
};

#endif //__OCOMMAND_H__

