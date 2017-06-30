#ifndef __GATE_H__
#define __GATE_H__
#include "util.h"
#include "IModule.h"
#include "singleton.h"
#include "IAgent.h"
#include <unordered_map>
#include <unordered_set>
#include "ILogin.h"
#include "IHarbor.h"

class OBuffer;
class OArgs;
class Gate : public IGate, public IAgentListener, public INodeListener, public OHolder<Gate> {
	enum {
		ST_NONE = 0,
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

#pragma pack(push, 4)
	struct TokenData {
		s64 accountID;
		s32 platform;
		s32 timestamp;
		s32 count;
	};
#pragma pack(pop)

public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void OnAgentOpen(IKernel * kernel, const s64 id);
	virtual void OnAgentClose(IKernel * kernel, const s64 id);
	virtual s32 OnAgentRecvPacket(IKernel * kernel, const s64 id, const void * context, const s32 size);

	virtual void OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port) {}
	virtual void OnClose(IKernel * kernel, s32 nodeType, s32 nodeId);

	virtual void SetRoleMgr(IRoleMgr * roleMgr) { _roleMgr = roleMgr; }

	void OnRecvLoginReq(IKernel * kernel, const s64 id, const OBuffer& buf);
	void OnRecvReconnectReq(IKernel * kernel, const s64 id, const OBuffer& buf);
	void OnRecvBindAccountAck(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);

	void OnRecvSelectRoleReq(IKernel * kernel, const s64 id, const OBuffer& buf);
	void OnRecvDistributeAck(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);
	void OnBindLogicAck(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);

	void OnUpdateRole(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);
	void OnRecvReselectRole(IKernel * kernel, const s64 id, const OBuffer& buf);

	void OnRecvCreateRoleReq(IKernel * kernel, const s64 id, const OBuffer& buf);
	void OnRecvDeleteRoleReq(IKernel * kernel, const s64 id, const OBuffer& buf);

	void OnRecvKickFromAccount(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);
	void OnRecvKickFromLogic(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);

	void Reset(IKernel * kernel, s64 id, s8 state, s32 from);

	void TransMsgToLogic(IKernel * kernel, const s64 id, const void * context, const s32 size);
	void OnTransMsgToActor(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& buf);
	void OnBrocastMsgToActor(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& buf);

	void Brocast(const void * context, const s32 size);
	void Send(s64 actorId, const void * context, const s32 size);

	void SendToClient(IKernel * kernel, const s64 id, const s32 msgId, const OBuffer& buf);

	bool CheckToken(const char * token, TokenData& data, bool login);
	void BuildToken(char * token, s32 size, const TokenData& data);

private:
    IKernel * _kernel;
	IRoleMgr * _roleMgr;

	bool _singleLogic;
	s32 _maxRole;
	std::string _loginKey;
	std::string _tokenKey;

	std::unordered_map<s64, Player> _players;
	std::unordered_map<s64, s64> _actors;
	std::unordered_map<s32, std::unordered_set<s64>> _logicPlayers;

	std::unordered_map<s32, ProtoHandlerType> _protos;
};

#endif //__GATE_H__

