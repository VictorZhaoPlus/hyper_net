#ifndef __REDIS_H__
#define __REDIS_H__
#include "util.h"
#include "IRedis.h"
#include <vector>
#include <list>
#include <unordered_map>
#include "hiredis.h"
#include "OString.h"
#include "sds.h"

class Redis : public IRedis {
	struct Context {
		olib::OString<32> ip;
		s32 port;
		redisContext* ctx;
		std::unordered_map<olib::OString<32>, olib::OString<32>, olib::OStringHash<32>> scriptIds;

		Context() : ctx(nullptr) {}
	};
	struct CommandBuff {
		s32 size;
		char data[10240];
	};
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	static Redis * Self() { return s_self; }
	static IKernel * GetKernel() { return s_kernel; }

	virtual bool Exec(const s64 id, const char* command, const OArgs& args, const std::function<bool(IKernel *, const IRedisResult *)>& f);
	virtual bool Call(const s64 id, const char* proc, const s32 keyCount, const OArgs& args, const std::function<bool(IKernel *, const IRedisResult *)>& f);

	static bool Connect(IKernel * kernel, Context& ctx);
	static bool LoadScript(IKernel * kernel, const s64 id, const char * proc);
	static bool Ping(IKernel * kernel, const s64 id);

	static void Append(CommandBuff& buf, const OArgs& args);
	static void Append(CommandBuff& buf, s8 val);
	static void Append(CommandBuff& buf, s16 val);
	static void Append(CommandBuff& buf, s32 val);
	static void Append(CommandBuff& buf, s64 val);
	static void Append(CommandBuff& buf, float val);
	static void Append(CommandBuff& buf, const char * val);
	static void Append(CommandBuff& buf, const void * val, const s32 size);

private:
	static Redis * s_self;
    static IKernel * s_kernel;

	static std::vector<Context> s_contexts;
	static std::unordered_map<olib::OString<32>, olib::OString<512>, olib::OStringHash<32>> s_scripts;
};

#endif //__REDIS_H__

