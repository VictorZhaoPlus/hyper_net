/*
 * File: IHttpMgr.h
 * Author: ooeyusea
 *
 * Created On March 25, 2016, 11:56 AM
 */

#ifndef __IHTTPMGR_H__
#define __IHTTPMGR_H__
 
#include "IModule.h"

class IHttpBase {
public:
	virtual ~IHttpBase() {}
};

class IHttpHandler {
public:
	IHttpHandler() : _base(nullptr) {}
	virtual ~IHttpHandler() {}

	inline void SetBase(IHttpBase * base) { _base = base; }
	inline IHttpBase * GetBase() { return _base; }

	virtual void OnSuccess(IKernel * kernel, const void * context, const s32 size) = 0;
	virtual void OnFail(IKernel * kernel, const s32 errCode) = 0;
	virtual void OnRelease() = 0;

private:
	IHttpBase * _base;
};

class IHttpMgr : public IModule {
public:
	virtual ~IHttpMgr() {}

	virtual void Get(const s64 id, IHttpHandler * handler, const char * uri) = 0;
	virtual void Post(const s64 id, IHttpHandler * handler, const char * url, const char * field) = 0;
	virtual void Stop(IHttpHandler * handler) = 0;

};

#endif /*__IHTTPMGR_H__ */