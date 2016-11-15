/*
 * File: ILogin.h
 * Author: ooeyusea
 *
 * Created On February 16, 2016, 08:31 AM
 */

#ifndef __ILOGIN_H__
#define __ILOGIN_H__
 
#include "IModule.h"
#include <unordered_map>
#include <vector>

class IBuffer;
class OArgs;
class IRole {
public:
	virtual ~IRole() {}

	virtual void Pack(IBuffer& buf) = 0;
	virtual void Update(const OArgs& arg) = 0;
};

class OBuffer;
class IObject;
class IRoleMgr {
public:
	virtual ~IRoleMgr() {}
	
	virtual bool GetRoleList(s64 account, const std::function<void (IKernel * kernel, const s64 actorId, IRole * role)>& f) = 0;
	virtual IRole * CreateRole(s64 actorId, const OBuffer& buf) = 0;
	virtual bool DeleteRole(s64 actorId, IRole * role) = 0;
	virtual void Recover(IRole * role) = 0;

	virtual bool LoadRole(const s64 actorId, IObject * object) = 0;
	virtual void PrepareRecover(IObject * object) = 0;
};

class IGate : public IModule {
public:
	virtual ~IGate() {}

	virtual void SetRoleMgr(IRoleMgr * roleMgr) = 0;
};

class IDistributionStrategy {
public:
	virtual ~IDistributionStrategy() {}

	virtual s32 ChooseLogic(s64 actorId) = 0;
};

class IDistribution : public IModule {
public:
	virtual ~IDistribution() {}

	virtual void SetStrategy(IDistributionStrategy * strategy) = 0;
};

typedef std::function<bool(IKernel *, IObject * object, const OBuffer& buf)> ProtocolCB;
class ILogic : public IModule {
public:
	virtual ~ILogic() {}

	virtual void SetRoleMgr(IRoleMgr * roleMgr) = 0;

	virtual void RegProtocolHandler(s32 messageId, const ProtocolCB& handler, const char * debug) = 0;
};
#define RGS_PROTOCOL_HANDLER(messageId, handler) _logic->RegProtocolHandler(messageId, std::bind(&handler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), #handler)

class IPacketSender : public IModule {
public:
	virtual ~IPacketSender() {}

	virtual void Send(const s32 gate, const s32 actorId, const s32 msgId, const OBuffer& buf) = 0;
	virtual void Brocast(const std::unordered_map<s32, std::vector<s64>>& actors, const s32 msgId, const OBuffer& buf) = 0;
};

#endif /*__ILOGIN_H__ */
