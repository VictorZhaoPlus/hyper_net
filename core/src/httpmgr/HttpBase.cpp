#include "HttpBase.h"
size_t WriteData(void * buffer, size_t size, size_t nmemb, void * param) {
	HttpBase * command = (HttpBase*)param;
	OASSERT(command, "wtf");

	if (nullptr == buffer)
		return -1;

	command->Append(buffer, (s32)size * (s32)nmemb);
	return nmemb;
}

bool HttpBase::OnExecute(IKernel * kernel) {
	OASSERT(_type != HT_NONE, "http request is not initialize");
	curl_easy_reset(_curl);
	curl_easy_setopt(_curl, CURLOPT_TIMEOUT, 10);
	curl_easy_setopt(_curl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(_curl, CURLOPT_URL, _url);
	curl_easy_setopt(_curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, WriteData);
	curl_easy_setopt(_curl, CURLOPT_WRITEDATA, (void*)this);

	if (_type == HT_POST) {
		curl_easy_setopt(_curl, CURLOPT_POST, 1);
		curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, _field);
	}

	_errCode = curl_easy_perform(_curl);
	return _errCode == CURLE_OK;
}

void HttpBase::OnSuccess(IKernel * kernel) {
	_handler->OnSuccess(kernel, _content.data(), (s32)_content.size());
	_handler->SetBase(nullptr);
	_handler = nullptr;
}

void HttpBase::OnFailed(IKernel * kernel, bool nonviolent) {
	_handler->OnFail(kernel, _errCode);
	_handler->SetBase(nullptr);
	_handler = nullptr;
}

void HttpBase::Post(IHttpHandler * handler, const char * url, const char * field) {
	SafeSprintf(_url, sizeof(_url), "%s", url);
	SafeSprintf(_field, sizeof(_field), "%s", field);
	_type = HT_POST;
	handler->SetBase(this);
	_handler = handler;
}

void HttpBase::Get(IHttpHandler * handler, const char * url) {
	SafeSprintf(_url, sizeof(_url), "%s", url);
	_type = HT_GET;
	handler->SetBase(this);
	_handler = handler;
}

void HttpBase::Append(const void * context, const s32 size) {
	_content.append((const char*)context, size);
}
