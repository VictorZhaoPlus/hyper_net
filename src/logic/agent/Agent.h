#ifndef __AGENT_H__
#define __AGENT_H__
#include "util.h"
#include "IAgent.h"
#include <unordered_map>

class AgentSession;
class IScriptEngine;
class IScriptArgumentReader;
class IScriptResultWriter;
class IScriptModule;
class Agent : public IAgent, public ISessionFactory {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void SetListener(IAgentListener * listener, const char * debug) { s_listener = listener; }

	static Agent * Self() { return s_self; }

	virtual ISession * Create();
	virtual void Recover(ISession *);

	static s64 OnOpen(AgentSession * session);
	static s32 OnRecv(const s64 id, const void * context, const s32 size);
	static void OnClose(const s64 id);

	virtual void Send(const s64 id, const void * context, const s32 size);
	virtual void Kick(const s64 id);

private:
	static Agent * s_self;
    static IKernel * s_kernel;
	static IAgentListener * s_listener;

	static s64 s_nextSessionId;
	static std::unordered_map<s64, AgentSession*> s_sessions;
};

#endif //__AGENT_H__

