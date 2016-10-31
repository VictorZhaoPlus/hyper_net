#ifndef __REDISCALLRESULT_H__
#define __REDISCALLRESULT_H__
#include "IRedis.h"
#include "hiredis.h"

class RedisCallResult : public IRedisResult {
public:
	RedisCallResult(redisReply * reply) : _reply(reply) {}
	virtual ~RedisCallResult() {}

	virtual s8 AsInt8();
	virtual s16 AsInt16();
	virtual s32 AsInt32();
	virtual s64 AsInt64();
	virtual float AsFloat();
	virtual const char* AsString();
	virtual const void* AsStruct(const s32 size);
	virtual const void* AsBlob(s32& size);

	virtual s32 Count();
	virtual bool GetResult(s32 idx, const std::function<bool(IKernel *, const IRedisResult *)>& f);

private:
	redisReply * _reply;
};

#endif //__REDISCALLRESULT_H__
