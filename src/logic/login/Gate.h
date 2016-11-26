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
#include "IHarbor.h"

#define MAX_ROLE_NAME 64
#define KEY_LEN 64

class OBuffer;
class OArgs;
class IIdMgr;
class IProtocolMgr;
class ICacheDB;
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
	void OnTransMsgToActor(IKernel * kernel, s32 nodeType, s32 nodeId, const void * context, const s32 size);

	void SendToClient(IKernel * kernel, const s64 id, const s32 msgId, const OBuffer& buf);

	bool CheckToken(const char * token, TokenData& data, bool login);
	void BuildToken(char * token, s32 size, const TokenData& data);

private:
    IKernel * _kernel;
	IHarbor * _harbor;
	IAgent * _agent;
	IRoleMgr * _roleMgr;
	IIdMgr * _idMgr;
	IProtocolMgr * _protocolMgr;
	ICacheDB * _cacheDB;

	bool _singleLogic;
	s32 _maxRole;
	olib::OString<KEY_LEN> _loginKey;
	olib::OString<KEY_LEN> _tokenKey;

	s32 _loginAckId;
	s32 _selectRoleAckId;
	s32 _createRoleAckId;
	s32 _deleteRoleAckId;
	s32 _reselectRoleAckId;

	s32 _noError;
	s32 _errorInvalidToken;
	s32 _errorReadAccountFailed;
	s32 _errorLoadRoleListFailed;
	s32 _errorDistributeLogicFailed;
	s32 _errorBindLogicFailed;
	s32 _errorTooMuchRole;
	s32 _errorCreateRoleFailed;
	s32 _errorDeleteRoleFailed;

	std::unordered_map<s64, Player> _players;
	std::unordered_map<s64, s64> _actors;
	std::unordered_map<s32, std::unordered_set<s64>> _logicPlayers;

	std::unordered_map<s32, ProtoHandlerType> _protos;
};

#endif //__GATE_H__

