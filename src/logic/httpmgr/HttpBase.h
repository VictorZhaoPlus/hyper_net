#ifndef __HTTPBASE_H__
#define __HTTPBASE_H__
#include "IHttpMgr.h"
#include <curl/curl.h>
#include "OStream.h"

#define MAX_URL 256
#define MAX_FIELD 256
#define DEFAULT_CONTENT_SIZE 256

class HttpBase : public IHttpBase, public IAsyncHandler {
	enum {
		HT_NONE = 0,
		HT_POST,
		HT_GET,
	};
public:
	HttpBase(CURL * curl)
		: _curl(curl)
		, _type(HT_NONE)
		, _handler(nullptr)
		, _errCode(0)
		, _content(DEFAULT_CONTENT_SIZE) {
	}
	virtual ~HttpBase() {}

	virtual bool OnExecute(IKernel * kernel);
	virtual void OnSuccess(IKernel * kernel);
	virtual void OnFailed(IKernel * kernel, bool nonviolent);
	virtual void OnRelease(IKernel * kernel) { DEL this; }

	void Post(IHttpHandler * handler, const char * url, const char * field);
	void Get(IHttpHandler * handler, const char * url);

	inline void SetUrl(const char * url) { SafeSprintf(_url, sizeof(_url), "%s", url); }

	inline void SetErrCode(const s32 errCode) { _errCode = errCode; }
	void Append(const void * context, const s32 size);

private:
	CURL * _curl;
	char _url[MAX_URL];
	char _field[MAX_FIELD];
	IHttpHandler * _handler;
	s32 _type;

	s32 _errCode;
	olib::OStream _content;
};

#endif //__HTTPBASE_H__
