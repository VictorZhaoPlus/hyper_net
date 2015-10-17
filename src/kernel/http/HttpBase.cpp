#include "HttpBase.h"

void HttpBase::Append(const void * context, const s32 size) {
	while (_content.GetFreeSpace() < size)
		_content.Expand();

	SafeMemcpy(_content.GetFree(), _content.GetFreeSpace(), context, size);
	_content.In(size);
}

void HttpBase::Complete(IKernel * kernel) {
	OASSERT(_handler, "where is http runner");
	if (_errCode == CURLE_OK)
		_handler->OnSuccess(kernel, _content.GetBuff(), _content.GetSize());
	else
		_handler->OnFail(kernel, _errCode);
}

void PostRequest::SetOpt(CURL * curl) {
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS,_field);
}