#ifndef __GATE_H__
#define __GATE_H__
#include "util.h"
#include "IModule.h"
#include "singleton.h"
#include "IAgent.h"
#include <unordered_map>
#include <unordered_set>
#include "OString.h"
#include "ILogin.h"

#define MAX_ROLE_NAME 64

class IHarbor;
class OBuffer;
class OArgs;
class IIdMgr;
class Gate : public ILogin, public IAgentListener, public OHolder<Gate> {
	enum {
		ST_NONE,
		ST_AUTHENING,
		ST_ROLELOADED,
		ST_DISTRIBUTE,
		ST_BINDING,
		ST_ONLINE,
	};

	struct Role {
		s64 actorId;
		IRole * role;
	};

	struct Player {
		s64 agentId;
		s64 selectActorId;
		s64 accountId;
		s32 logic;
		s64 lastActorId;
		s8 state;

		std::list<Role> roles;
	};

	typedef void (Gate::*ProtoHandlerType)(IKernel * kernel, const s64 id, const OBuffer& buf);

public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void OnAgentOpen(IKernel * kernel, const s64 id);
	virtual void OnAgentClose(IKernel * kernel, const s64 id);
	virtual s32 OnAgentRecvPacket(IKernel * kernel, const s64 id, const void * context, const s32 size);

	virtual void SetRoleMgr(IRoleMgr * roleMgr) { _roleMgr = roleMgr; }

	void OnRecvLoginReq(IKernel * kernel, const s64 id, const OBuffer& buf);
	void OnRecvConnectReq(IKernel * kernel, const s64 id, const OBuffer& buf);
	void OnRecvBindAccountAck(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);

	void OnRecvSelectRoleReq(IKernel * kernel, const s64 id, const OBuffer& buf);
	void OnRecvDistributeAck(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);
	void OnBindLogicAck(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);

	void OnRecvCreateRoleReq(IKernel * kernel, const s64 id, const OBuffer& buf);
	void OnRecvDeleteRoleReq(IKernel * kernel, const s64 id, const OBuffer& buf);

	void OnRecvKickFromAccount(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);
	void OnRecvKickFromLogic(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);

	void Reset(IKernel * kernel, s64 id, s8 state, s32 from);

	void TransMsgToLogic(IKernel * kernel, const s64 id, const void * context, const s32 size);

private:
    IKernel * _kernel;
	IHarbor * _harbor;
	IAgent * _agent;
	IRoleMgr * _roleMgr;
	IIdMgr * _idMgr;

	bool _singleLogic;
	s32 _maxRole;

	std::unordered_map<s64, Player> _players;
	std::unordered_map<s64, s64> _actors;
	std::unordered_map<s32, std::unordered_set<s64>> _logicPlayers;

	std::unordered_map<s32, ProtoHandlerType> _protos;
};

#endif //__GATE_H__

