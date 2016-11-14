#ifndef __LOGIC_H__
#define __LOGIC_H__
#include "util.h"
#include "ILogin.h"
#include "singleton.h"
#include "IHarbor.h"
#include <unordered_map>
#include <unordered_set>
#include "OCallback.h"

class IObjectMgr;
class IProtocolMgr;
class Logic : public ILogic, public INodeListener, public OHolder<Logic> {
	struct Props {
		s32 account;
		s32 gate;
		s32 logic;
	};

	typedef olib::CallbackType<s32, ProtocolCB>::type ProtocolPool;
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void SetRoleMgr(IRoleMgr * roleMgr) { _roleMgr = roleMgr; }

	virtual void OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port) {}
	virtual void OnClose(IKernel * kernel, s32 nodeType, s32 nodeId);

	virtual void RegProtocolHandler(s32 messageId, const ProtocolCB& handler, const char * debug);
	
	void OnBindLogic(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);
	void OnUnbindLogic(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);
	void OnTransMsg(IKernel * kernel, s32 nodeType, s32 nodeId, const void * context, const s32 size);

private:
    IKernel * _kernel;
	IRoleMgr * _roleMgr;
	IHarbor * _harbor;
	IObjectMgr * _objectMgr;
	IProtocolMgr * _protocolMgr;

	Props _prop;
	s32 _noError;
	s32 _errorLoadPlayerFailed;

	std::unordered_map<s32, std::unordered_set<s64>> _gateActors;

	ProtocolPool _protos;
};

#endif //__LOGIC_H__

