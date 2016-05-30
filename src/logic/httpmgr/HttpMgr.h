#ifndef __HTTPMGR_H__
#define __HTTPMGR_H__
#include "util.h"
#include "IHttpMgr.h"
#include <curl/curl.h>
#include <vector>

class HttpMgr : public IHttpMgr {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	static HttpMgr * Self() { return s_self; }

	virtual void Get(const s64 id, IHttpHandler * handler, const char * uri);
	virtual void Post(const s64 id, IHttpHandler * handler, const char * url, const char * field);
	virtual void Stop(IHttpHandler * handler);

private:
	static HttpMgr * s_self;
    static IKernel * s_kernel;

	static std::vector<CURL *> s_urls;
};

#endif //__HTTPMGR_H__

