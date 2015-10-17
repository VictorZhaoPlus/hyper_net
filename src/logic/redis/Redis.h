#ifndef __REDIS_H__
#define __REDIS_H__
#include "util.h"
#include "IModule.h"
#include <vector>
#include <list>

class RedisSession;
class IScriptEngine;
class IScriptModule;
class IScriptArgumentReader;
class IScriptResultWriter;
class Redis : public IModule {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	static void OnRecv(IKernel * kernel, const s64 sequenceId, const void * context, const s32 size);
	static void OnFailed(IKernel * kernel, const s64 sequenceId);

	static void Call(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer);

	static s32 GetSendBufferSize() { return s_sendBufferSize; }
	static s32 GetRecvBufferSize() { return s_recvBufferSize; }
	static s32 GetReconnectInterval() { return s_reconnectTick; }

	static void Reconnect(RedisSession * session);

	static Redis * Self() { return s_self; }
	static IKernel * GetKernel() { return s_kernel; }

private:
	static Redis * s_self;
    static IKernel * s_kernel;
	static IScriptEngine * s_scriptEngine;

	static IScriptModule * s_module;

	static s32 s_sendBufferSize;
	static s32 s_recvBufferSize;
	static s32 s_reconnectTick;

	static std::vector<RedisSession*> s_sessions;
};

#endif //__REDIS_H__

