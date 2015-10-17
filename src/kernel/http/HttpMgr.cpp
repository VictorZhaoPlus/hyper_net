#include "HttpMgr.h"
#include "XmlReader.h"
#include <curl/curl.h>
#include "HttpBase.h"
#include "kernel.h"
#include "ConfigMgr.h"

size_t WriteData(void * buffer, size_t size, size_t nmemb, void * param) {
	HttpBase * command = (HttpBase*)param;
	OASSERT(command, "wtf");

	if (nullptr == buffer)
		return -1;

	command->Append(buffer, size * nmemb);
	return nmemb;
}

class HttpRunner : public olib::IRunner<HttpBase> {
public:
	HttpRunner() : _handler(nullptr) { _handler = curl_easy_init(); }
	virtual ~HttpRunner() {}

	virtual bool Execute(HttpBase * command) {
		curl_easy_reset(_handler);
		curl_easy_setopt(_handler, CURLOPT_TIMEOUT, 10);
		curl_easy_setopt(_handler, CURLOPT_VERBOSE, 0L);
		curl_easy_setopt(_handler, CURLOPT_URL, command->GetUrl());
		curl_easy_setopt(_handler, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(_handler, CURLOPT_WRITEFUNCTION, WriteData);
		curl_easy_setopt(_handler, CURLOPT_WRITEDATA, (void*)command);

		command->SetOpt(_handler);
		command->SetErrCode(curl_easy_perform(_handler));
		return true;
	}

	virtual void Release() {
		if (_handler)
			curl_easy_cleanup(_handler);
		DEL this;
	}

private:
	CURL * _handler;
};

bool HttpMgr::Ready() {
    return true;
}

bool HttpMgr::Initialize() {
	s32 threadCount = ConfigMgr::Instance()->GetHttpThreadCount();
	if (threadCount > 0) {
		if (!Start(threadCount, this)) {
			OASSERT(false, "wtf");
			return false;
		}
	}

    return true;
}

bool HttpMgr::Destroy() {
	s32 threadCount = ConfigMgr::Instance()->GetHttpThreadCount();
	if (threadCount > 0)
		Terminate();
    DEL this;
    return true;
}

void HttpMgr::Loop() {
	Process(0);
}

void HttpMgr::Get(const s64 threadId, IHttpHandler * http, const char * uri) {
	HttpBase * base = GetRequest::Create(http, uri);
	OASSERT(base, "wtf");

	Push(threadId, base);
}

void HttpMgr::Post(const s64 threadId, IHttpHandler * http, const char * url, const char * field) {
	HttpBase * base = PostRequest::Create(http, url, field);
	OASSERT(base, "wtf");

	Push(threadId, base);
}

void HttpMgr::Stop(IHttpHandler * http) {
	//Remove(http);
}

void HttpMgr::Complete(HttpBase * command) {
	command->Complete(Kernel::Instance());
	command->Release();
}

olib::IRunner<HttpBase> * HttpMgr::Create() {
	return NEW HttpRunner;
}
