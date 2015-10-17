#ifndef __AGENT_H__
#define __AGENT_H__
#include "util.h"
#include "IModule.h"
#include <unordered_map>

class AgentSession;
class IScriptEngine;
class IScriptArgumentReader;
class IScriptResultWriter;
class IScriptModule;
class Agent : public IModule, public ISessionFactory {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	static Agent * Self() { return s_self; }

	virtual ISession * Create();
	virtual void Recover(ISession *);

	static s64 OnOpen(AgentSession * session);
	static s32 OnRecv(const s64 id, const void * context, const s32 size);
	static void OnClose(const s64 id);

	static void Send(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer);
	static void Kick(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer);

private:
	static Agent * s_self;
    static IKernel * s_kernel;
	static IScriptEngine * s_scriptEngine;

	static IScriptModule * s_module;
	static s64 s_nextSessionId;
	static std::unordered_map<s64, AgentSession*> s_sessions;
};

#endif //__AGENT_H__

