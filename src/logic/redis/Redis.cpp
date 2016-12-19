#include "Redis.h"
#include "XmlReader.h"
#include <string>
#include "IScriptEngine.h"
#include "OArgs.h"
#include "RedisCallResult.h"

Redis * Redis::s_self = nullptr;
IKernel * Redis::s_kernel = nullptr;

std::vector<Redis::Context> Redis::s_contexts;
std::unordered_map<olib::OString<32>, olib::OString<512>, olib::OStringHash<32>> Redis::s_scripts;

bool Redis::Initialize(IKernel * kernel) {
    s_self = this;
    s_kernel = kernel;

	olib::XmlReader reader;
	std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/server_conf.xml";
	if (!reader.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
		return false;
	}

	const olib::IXmlObject& units = reader.Root()["redis"][0]["context"];
	for (s32 i = 0; i < units.Count(); ++i) {
		Context ctx;
		ctx.ip = units[i].GetAttributeString("ip");
		ctx.port = units[i].GetAttributeInt32("port");

		if (!Connect(kernel, ctx))
			return false;

		s_contexts.push_back(ctx);
	}

	olib::XmlReader scriptReader;
	std::string scriptPath = std::string(tools::GetAppPath()) + "/envir/script.xml";
	if (!scriptReader.LoadXml(scriptPath.c_str())) {
		OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
		return false;
	}

	const olib::IXmlObject& scripts = scriptReader.Root()["script"];
	for (s32 i = 0; i < scripts.Count(); ++i) {
		const char * proc = scripts[i].GetAttributeString("name");
		const char * script = scripts[i].CData();

		s_scripts[proc] = script;
	}

    return true;
}

bool Redis::Launched(IKernel * kernel) {
    return true;
}

bool Redis::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

bool Redis::Exec(const s64 id, const char* command, const OArgs& args, const std::function<bool(IKernel *, const IRedisResult *)>& f) {
	if (s_contexts.empty())
		return false;

	Ping(s_kernel, id);

	CommandBuff buf;
	buf.size = 0;

	SafeSprintf(buf.data + buf.size, sizeof(buf.data) - buf.size, "*%d\r\n$%d\r\n%s\r\n", args.Count() + 1, strlen(command), command);
	buf.size += (s32)strlen(buf.data + buf.size);

	Append(buf, args);

	s_contexts[id % s_contexts.size()].ctx->obuf = buf.data;
	redisReply * reply = NULL;
	redisGetReply(s_contexts[id % s_contexts.size()].ctx, (void**)&reply);
	if (NULL == reply) {
		return false;
	}

	if (REDIS_REPLY_ERROR == reply->type || REDIS_REPLY_NIL == reply->type) {
		freeReplyObject(reply);
		return false;
	}

	bool ret = true;
	if (f) {
		RedisCallResult rst(reply);
		ret = f(s_kernel, &rst);
	}

	freeReplyObject(reply);
	return ret;

}

bool Redis::Call(const s64 id, const char* proc, const s32 keyCount, const OArgs& args, const std::function<bool(IKernel *, const IRedisResult *)>& f) {
	if (s_contexts.empty())
		return false;

	Ping(s_kernel, id);

	if (!LoadScript(s_kernel, id, proc))
		return false;

	CommandBuff buf;
	buf.size = 0;

	const char * scriptId = s_contexts[id % s_contexts.size()].scriptIds[proc].GetString();
	s32 len = (s32)strlen(scriptId);
	SafeSprintf(buf.data + buf.size, sizeof(buf.data) - buf.size, "*%d\r\n$7\r\nEVALSHA\r\n$%d\r\n%s\r\n", args.Count() + 2, len, scriptId);
	buf.size += (s32)strlen(buf.data + buf.size);

	Append(buf, args);

	s_contexts[id % s_contexts.size()].ctx->obuf = buf.data;
	redisReply * reply = NULL;
	redisGetReply(s_contexts[id % s_contexts.size()].ctx, (void**)&reply);
	if (NULL == reply) {
		return false;
	}

	if (REDIS_REPLY_ERROR == reply->type || REDIS_REPLY_NIL == reply->type) {
		freeReplyObject(reply);
		return false;
	}

	bool ret = true;
	if (f) {
		RedisCallResult rst(reply);
		ret = f(s_kernel, &rst);
	}

	freeReplyObject(reply);
	return ret;
}

bool Redis::Connect(IKernel * kernel, Context& ctx) {
	if (ctx.ctx) {
		redisFree(ctx.ctx);
		ctx.ctx = nullptr;
	}

	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 50000;

	ctx.ctx = redisConnectWithTimeout(ctx.ip.GetString(), ctx.port, tv);
	OASSERT(ctx.ctx, "wtf");
	if (!ctx.ctx)
		return false;

	ctx.scriptIds.clear();
	return true;
}

bool Redis::LoadScript(IKernel * kernel, const s64 id, const char * proc) {
	Context & ctx = s_contexts[id % s_contexts.size()];
	auto itr = s_scripts.find(proc);
	if (itr != s_scripts.end()) {
		auto itrId = ctx.scriptIds.find(proc);
		if (itrId == ctx.scriptIds.end()) {
			IArgs<2, 1024> args;
			args << "LOAD" << itr->second.GetString();
			return s_self->Exec(id, "SCRIPT", args.Out(), [&ctx, proc](IKernel * kernel, const IRedisResult * ret) {
				ctx.scriptIds[proc] = ret->AsString();
				return true;
			});
		}
	}
	return false;
}

bool Redis::Ping(IKernel * kernel, const s64 id) {
	Context & ctx = s_contexts[id % s_contexts.size()];
	if (ctx.ctx && ctx.ctx->err == REDIS_OK)
		return true;
	return Connect(kernel, ctx);
}

void Redis::Append(CommandBuff& buf, const OArgs& args) {
	for (s32 i = 0; i < args.Count(); i++) {
		switch (args.GetType(i)) {
		case ArgType::TYPE_STRING: Append(buf, args.GetDataString(i)); break;
		//case ArgType::TYPE_STUCT: Append(buf, args.GetDataStruct(i)); break;
		case ArgType::TYPE_FLOAT: Append(buf, args.GetDataFloat(i)); break;
		case ArgType::TYPE_INT64: Append(buf, args.GetDataInt64(i)); break;
		case ArgType::TYPE_INT32: Append(buf, args.GetDataInt32(i)); break;
		case ArgType::TYPE_INT16: Append(buf, args.GetDataInt16(i)); break;
		case ArgType::TYPE_INT8: Append(buf, args.GetDataInt8(i)); break;
		case ArgType::TYPE_BOOL: Append(buf, args.GetDataBool(i) ? 1 : 0); break;
		}
	}
}

void Redis::Append(CommandBuff& buf, s8 val) {
	char temp[32] = { 0 };
	SafeSprintf(temp, sizeof(temp), "%d", val);

	SafeSprintf(buf.data + buf.size, sizeof(buf.data) - buf.size, "$%d\r\n%s\r\n", (s32)strlen(temp), temp);
	buf.size += (s32)strlen(buf.data + buf.size);
}

void Redis::Append(CommandBuff& buf, s16 val) {
	char temp[32] = { 0 };
	SafeSprintf(temp, sizeof(temp), "%d", val);

	SafeSprintf(buf.data + buf.size, sizeof(buf.data) - buf.size, "$%d\r\n%s\r\n", (s32)strlen(temp), temp);
	buf.size += (s32)strlen(buf.data + buf.size);
}

void Redis::Append(CommandBuff& buf, s32 val) {
	char temp[32] = { 0 };
	SafeSprintf(temp, sizeof(temp), "%d", val);

	SafeSprintf(buf.data + buf.size, sizeof(buf.data) - buf.size, "$%d\r\n%s\r\n", (s32)strlen(temp), temp);
	buf.size += (s32)strlen(buf.data + buf.size);
}

void Redis::Append(CommandBuff& buf, s64 val) {
	char temp[32] = { 0 };
	SafeSprintf(temp, sizeof(temp), "%lld", val);

	SafeSprintf(buf.data + buf.size, sizeof(buf.data) - buf.size, "$%d\r\n%s\r\n", (s32)strlen(temp), temp);
	buf.size += (s32)strlen(buf.data + buf.size);
}

void Redis::Append(CommandBuff& buf, float val) {
	char temp[32] = { 0 };
	SafeSprintf(temp, sizeof(temp), "%.2f", val);

	SafeSprintf(buf.data + buf.size, sizeof(buf.data) - buf.size, "$%d\r\n%s\r\n", (s32)strlen(temp), temp);
	buf.size += (s32)strlen(buf.data + buf.size);
}

void Redis::Append(CommandBuff& buf, const char * val) {
	SafeSprintf(buf.data + buf.size, sizeof(buf.data) - buf.size, "$%d\r\n%s\r\n", (s32)strlen(val), val);
	buf.size += (s32)strlen(buf.data + buf.size);
}

void Redis::Append(CommandBuff& buf, const void * val, const s32 size) {
	SafeSprintf(buf.data + buf.size, sizeof(buf.data) - buf.size, "$%d\r\n", size);
	buf.size += (s32)strlen(buf.data + buf.size);

	SafeMemcpy(buf.data + buf.size, sizeof(buf.data) - buf.size, val, size);
	buf.size += size;
	SafeSprintf(buf.data + buf.size, sizeof(buf.data) - buf.size, "\r\n");
	buf.size += (s32)strlen(buf.data + buf.size);
}
