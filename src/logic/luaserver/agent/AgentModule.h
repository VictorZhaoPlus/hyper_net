#ifndef __AGENTMODULE_H__
#define __AGENTMODULE_H__
#include "util.h"
#include "IModule.h"
#include "IAgent.h"

class IScriptEngine;
class IScriptArgumentReader;
class IScriptResultWriter;
class IScriptModule;
class AgentModule : public IModule, public IAgentListener {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	static AgentModule * Self() { return s_self; }

	virtual void OnAgentOpen(IKernel * kernel, const s64 id);
	virtual void OnAgentClose(IKernel * kernel, const s64 id);
	virtual s32 OnAgentRecvPacket(IKernel * kernel, const s64 id, const void * context, const s32 size);

	static void Send(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer);
	static void Kick(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer);

private:
	static AgentModule * s_self;
    static IKernel * s_kernel;
	static IScriptEngine * s_scriptEngine;
	static IAgent * s_agent;

	static IScriptModule * s_module;
};

#endif //__AGENTMODULE_H__

