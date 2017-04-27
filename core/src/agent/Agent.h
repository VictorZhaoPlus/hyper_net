#ifndef __AGENT_H__
#define __AGENT_H__
#include "util.h"
#include "IAgent.h"
#include <unordered_map>
#include <singleton.h>

class AgentSession;
class IScriptEngine;
class IScriptArgumentReader;
class IScriptResultWriter;
class IScriptModule;
class Agent : public IAgent, public ISessionFactory, public OHolder<Agent> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void SetListener(IAgentListener * listener, const char * debug) { _listener = listener; }

	virtual ISession * Create();
	virtual void Recover(ISession *);

	s64 OnOpen(AgentSession * session);
	s32 OnRecv(const s64 id, const void * context, const s32 size);
	void OnClose(const s64 id);

	virtual void Send(const s64 id, const void * context, const s32 size);
	virtual void Kick(const s64 id);

private:
    IKernel * _kernel;
	IAgentListener * _listener;

	s64 s_nextSessionId;
	std::unordered_map<s64, AgentSession*> s_sessions;
};

#endif //__AGENT_H__

