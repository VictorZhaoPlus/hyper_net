/*
 * File: ILogin.h
 * Author: ooeyusea
 *
 * Created On February 16, 2016, 08:31 AM
 */

#ifndef __ILOGIN_H__
#define __ILOGIN_H__
 
#include "IModule.h"

class IRole {
public:
	virtual ~IRole() {}
};

class OBuffer;
class IRoleMgr {
public:
	virtual ~IRoleMgr() {}
	
	virtual bool GetRoleList(s64 account, const std::function<void (IKernel * kernel, const s64 actorId, IRole * role)>& f) = 0;
	virtual IRole * CreateRole(s64 actorId, const OBuffer& buf) = 0;
	virtual bool DeleteRole(s64 actorId, IRole * role) = 0;
	virtual void Recover(IRole * role) = 0;
};

class ILogin : public IModule {
public:
	virtual ~ILogin() {}

	virtual void SetRoleMgr(IRoleMgr * roleMgr) = 0;
};

#endif /*__ILOGIN_H__ */
