#ifndef __HTTPBASE_H__
#define __HTTPBASE_H__
#include "util.h"
#include "IKernel.h"
using namespace core;
#include <curl/curl.h>
#include "OStream.h"

#define MAX_URL 256
#define MAX_FIELD 256
#define DEFAULT_CONTENT_SIZE 256

class HttpBase : public IHttpBase {
public:
	HttpBase(IHttpHandler * handler) : _handler(handler), _errCode(0), _content(DEFAULT_CONTENT_SIZE) {
		SafeMemset(_url, sizeof(_url), 0, sizeof(_url));
	}
	virtual ~HttpBase() {}

	inline const char * GetUrl() const { return _url; }
	inline void SetUrl(const char * url) { SafeMemcpy(_url, sizeof(_url) - 1, url, strlen(url)); }

	inline void SetErrCode(const s32 errCode) { _errCode = errCode; }
	void Append(const void * context, const s32 size);

	void Complete(IKernel * kernel);

	virtual void SetOpt(CURL * curl) = 0;
	virtual void Release() = 0;

private:
	char _url[MAX_URL + 1];
	IHttpHandler * _handler;

	s32 _errCode;
	olib::OStream _content;
};

class GetRequest : public HttpBase {
public:
	static HttpBase * Create(IHttpHandler * handler, const char * uri) {
		GetRequest * ret = NEW GetRequest(handler);
		ret->SetUrl(uri);
		return ret;
	}

	virtual void SetOpt(CURL * curl) {}
	virtual void Release() { DEL this; }

private:
	GetRequest(IHttpHandler * handler) : HttpBase(handler) {}
	virtual ~GetRequest() {}
};

class PostRequest : public HttpBase {
public:
	static HttpBase * Create(IHttpHandler * handler, const char * url, const char * field) {
		PostRequest * ret = NEW PostRequest(handler);
		ret->SetUrl(url);
		ret->SetField(field);
		return ret;
	}

	virtual void SetOpt(CURL * curl);
	virtual void Release() { DEL this; }

	inline void SetField(const char * field) { SafeMemcpy(_field, sizeof(_field) - 1, field, strlen(field)); }

private:
	PostRequest(IHttpHandler * handler) : HttpBase(handler) {
		SafeMemset(_field, sizeof(_field), 0, sizeof(_field));
	}
	virtual ~PostRequest() {}

	char _field[MAX_FIELD + 1];
};

#endif //__HTTPBASE_H__
