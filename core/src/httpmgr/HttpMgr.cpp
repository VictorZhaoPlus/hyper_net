#include "HttpMgr.h"
#include "HttpBase.h"
#include "XmlReader.h"
#include "tools.h"
#include <string>

HttpMgr * HttpMgr::s_self = nullptr;
IKernel * HttpMgr::s_kernel = nullptr;

std::vector<CURL *> HttpMgr::s_urls;

bool HttpMgr::Initialize(IKernel * kernel) {
    s_self = this;
    s_kernel = kernel;

	olib::XmlReader reader;
	std::string coreConfigPath = std::string(tools::GetWorkPath()) + "/config/server_conf.xml";
	if (!reader.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
		return false;
	}

	const char * name = kernel->GetCmdArg("name");

	s32 threadCount = reader.Root()["extend"][0][name][0]["async"][0].GetAttributeInt32("thread");
	for (s32 i = 0; i < threadCount; ++i) {
		s_urls.push_back(curl_easy_init());
	}

    return true;
}

bool HttpMgr::Launched(IKernel * kernel) {
    return true;
}

bool HttpMgr::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void HttpMgr::Get(const s64 id, IHttpHandler * handler, const char * uri) {
	OASSERT(!handler->GetBase(), "can't push http handler twice");
	HttpBase * base = NEW HttpBase(s_urls[id % s_urls.size()]);
	base->Get(handler, uri);

	IKernel * kernel = s_kernel;
	START_ASYNC(id, base);
}

void HttpMgr::Post(const s64 id, IHttpHandler * handler, const char * url, const char * field) {
	OASSERT(!handler->GetBase(), "can't push http handler twice");
	HttpBase * base = NEW HttpBase(s_urls[id % s_urls.size()]);
	base->Post(handler, url, field);

	IKernel * kernel = s_kernel;
	START_ASYNC(id, base);
}

void HttpMgr::Stop(IHttpHandler * handler) {
	OASSERT(handler->GetBase(), "http handler is not start");
	if (handler->GetBase())
		s_kernel->StopAsync((HttpBase*)handler->GetBase());
}
