#ifndef __LOGIN_H__
#define __LOGIN_H__
#include "util.h"
#include "IModule.h"
#include "singleton.h"
#include "IHarbor.h"
#include <unordered_map>
#include <unordered_set>

class IProtocolMgr;
class Login : public IModule, public INodeListener, public OHolder<Login> {
	enum {
		ST_OFFLINE = 0,
		ST_ONLINE,
		ST_SWITCH,
	};
	struct Account {
		s64 accountId;
		s8 state;
		s32 gateId;
		s64 agentId;
		s32 switchGateId;
		s64 switchAgentId;
		s32 tokenCount;
	};

	struct Proto {
		s32 bindAccountReq;
		s32 bindAccountAck;
		s32 unbindAccountReq;
		s32 kickFromAccount;
	};

public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port) {}
	virtual void OnClose(IKernel * kernel, s32 nodeType, s32 nodeId);

	void OnRecvBindAccount(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);
	void OnRecvUnbindAccount(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);

	void BindAccountSuccess(IKernel * kernel, const Account& account);

private:
    IKernel * _kernel;

	std::unordered_map<s64, Account> _accounts;
	std::unordered_map<s32, std::unordered_set<s64>> _gateAccounts;
	std::unordered_map<s32, std::unordered_set<s64>> _switchGateAccounts;
};

#endif //__LOGIN_H__

