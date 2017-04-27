#ifndef __REDISCALLRESULT_H__
#define __REDISCALLRESULT_H__
#include "IRedis.h"
#include "hiredis.h"

class RedisCallResult : public IRedisResult {
public:
	RedisCallResult(redisReply * reply) : _reply(reply) {}
	virtual ~RedisCallResult() {}

	virtual s8 AsInt8() const;
	virtual s16 AsInt16() const;
	virtual s32 AsInt32() const;
	virtual s64 AsInt64() const;
	virtual float AsFloat() const;
	virtual const char* AsString() const;
	virtual const void* AsStruct(const s32 size) const;
	virtual const void* AsBlob(s32& size) const;

	virtual s32 Count() const;
	virtual bool GetResult(s32 idx, const std::function<bool(IKernel *, const IRedisResult *)>& f) const;

private:
	redisReply * _reply;
};

#endif //__REDISCALLRESULT_H__
