/*
 * File: IRedis.h
 * Author: ooeyusea
 *
 * Created On February 16, 2016, 08:31 AM
 */

#ifndef __IREDIS_H__
#define __IREDIS_H__
 
#include "IModule.h"
#include <functional>

class IRedisResult {
public:
	virtual ~IRedisResult() {}

	virtual s8 AsInt8() = 0;
	virtual s16 AsInt16() = 0;
	virtual s32 AsInt32() = 0;
	virtual s64 AsInt64() = 0;
	virtual float AsFloat() = 0;
	virtual const char* AsString() = 0;
	virtual const void* AsStruct(const s32 size) = 0;
	virtual const void* AsBlob(s32& size) = 0;

	virtual s32 Count() = 0;
	virtual bool GetResult(s32 idx, const std::function<bool(IKernel *, const IRedisResult *)>& f) = 0;
};

class OArgs;
class IRedis : public IModule {
public:
	virtual ~IRedis() {}

	virtual bool Exec(const s64 id, const char* command, const OArgs& args, const std::function<bool(IKernel *, const IRedisResult *)>& f) = 0;
	virtual bool Call(const s64 id, const char* proc, const s32 keyCount, const OArgs& args, const std::function<bool (IKernel *, const IRedisResult *)>& f) = 0;
};

#endif /*__IREDIS_H__ */
