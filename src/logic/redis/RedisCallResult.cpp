#include "RedisCallResult.h"
#include "tools.h"
#include "Redis.h"

//#define REDIS_REPLY_STRING 1
//#define REDIS_REPLY_ARRAY 2
//#define REDIS_REPLY_INTEGER 3
//#define REDIS_REPLY_NIL 4

s8 RedisCallResult::AsInt8() {
	if (_reply->type == REDIS_REPLY_INTEGER)
		return _reply->integer;

	if (_reply->type == REDIS_REPLY_STRING)
		return (s8)tools::StringAsInt(_reply->str);

	OASSERT(false, "invalid type");
	return 0;
}

s16 RedisCallResult::AsInt16() {
	if (_reply->type == REDIS_REPLY_INTEGER)
		return _reply->integer;

	if (_reply->type == REDIS_REPLY_STRING)
		return (s16)tools::StringAsInt(_reply->str);

	OASSERT(false, "invalid type");
	return 0;
}

s32 RedisCallResult::AsInt32() {
	if (_reply->type == REDIS_REPLY_INTEGER)
		return _reply->integer;

	if (_reply->type == REDIS_REPLY_STRING)
		return tools::StringAsInt(_reply->str);

	OASSERT(false, "invalid type");
	return 0;
}

s64 RedisCallResult::AsInt64() {
	if (_reply->type == REDIS_REPLY_INTEGER)
		return _reply->integer;

	if (_reply->type == REDIS_REPLY_STRING)
		return tools::StringAsInt64(_reply->str);

	OASSERT(false, "invalid type");
	return 0;
}

float RedisCallResult::AsFloat() {
	if (_reply->type == REDIS_REPLY_STRING)
		return tools::StringAsFloat(_reply->str);

	OASSERT(false, "invalid type");
	return 0;
}

const char* RedisCallResult::AsString() {
	if (_reply->type == REDIS_REPLY_STRING)
		return _reply->str;

	OASSERT(false, "invalid type");
	return "";
}

const void* RedisCallResult::AsStruct(const s32 size) {
	if (_reply->type == REDIS_REPLY_STRING) {
		OASSERT(_reply->len == size, "wtf");
		return _reply->str;
	}

	OASSERT(false, "invalid type");
	return nullptr;
}

const void* RedisCallResult::AsBlob(s32& size) {
	if (_reply->type == REDIS_REPLY_STRING) {
		size = _reply->len;
		return _reply->str;
	}

	OASSERT(false, "invalid type");
	return nullptr;
}

s32 RedisCallResult::Count() {
	if (_reply->type == REDIS_REPLY_ARRAY) {
		return _reply->elements;
	}

	OASSERT(false, "invalid type");
	return 0;
}

bool RedisCallResult::GetResult(s32 idx, const std::function<bool(IKernel *, const IRedisResult *)>& f) {
	if (_reply->type == REDIS_REPLY_ARRAY) {
		OASSERT(idx >= 0 && idx < _reply->elements, "out of range");
		if (idx >= 0 && idx < _reply->elements) {
			RedisCallResult rst(_reply->element[idx]);
			return f(Redis::GetKernel(), &rst);
		}
	}

	OASSERT(false, "invalid type");
	return false;
}