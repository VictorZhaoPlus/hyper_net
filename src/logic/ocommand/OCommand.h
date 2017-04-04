#ifndef __OCOMMAND_H__
#define __OCOMMAND_H__
#include "util.h"
#include "IOCommand.h"
#include "singleton.h"
#include "OCallback.h"
#include "IHarbor.h"

class IObject;
class OArgs;
class IObjectMgr;
class IProtocolMgr;
class OBuffer;
class IObjectLocator;
class OCommand : public IOCommand, public INodeListener, public OHolder<OCommand> {
	typedef olib::CallbackType<s32, CommandCB>::type CMD_CB_POOL;
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void RegisterCmd(s32 cmd, const CommandCB& handler, const char * debug) { _cbs.Register(cmd, handler, debug); }
	virtual void Command(s32 cmd, const IObject * sender, const s64 reciever, const OArgs& args, const CommandFailCB& fail);

	//can't use in logic
	virtual bool CommandTo(const s64 reciver, const s32 cmd, const OArgs & args);

	virtual void OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port) {}
	virtual void OnClose(IKernel * kernel, s32 nodeType, s32 nodeId);

	void OnCommand(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& stream);
	void OnForwardCommand(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& stream);
	void OnCommandResult(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);

private:
    IKernel * _kernel;
	IHarbor * _harbor;
	IObjectMgr * _objectMgr;
	IProtocolMgr * _protocolMgr;
	IObjectLocator * _objectLocator;

	s32 _protoCommandTo;
	s32 _protoCommandForward;
	s32 _protoCommandResult;

	CMD_CB_POOL _cbs;

	std::unordered_map<s32, CommandFailCB> _failed;
	s32 _nextFailCbId;
};

#endif //__OCOMMAND_H__

