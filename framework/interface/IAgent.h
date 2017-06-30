/*
 * File: IAgent.h
 * Author: ooeyusea
 *
 * Created On February 16, 2016, 08:31 AM
 */

#ifndef __IAGENT_H__
#define __IAGENT_H__
 
#include "IModule.h"

class IAgentListener {
public:
	virtual ~IAgentListener() {}

	virtual void OnAgentOpen(IKernel * kernel, const s64 id) = 0;
	virtual void OnAgentClose(IKernel * kernel, const s64 id) = 0;
	virtual s32 OnAgentRecvPacket(IKernel * kernel, const s64 id, const void * context, const s32 size) = 0;
};

class IAgent : public IModule {
public:
	virtual ~IAgent() {}

	virtual void SetListener(IAgentListener * listener, const char * debug) = 0;

	virtual void Send(const s64 id, const void * context, const s32 size) = 0;
	virtual void Kick(const s64 id) = 0;
};

#define RGS_AGENT_LISTENER(listener) { \
    char debug[4096] = {0}; \
    SafeSprintf(debug, sizeof(debug), "%s:%d", __FILE__, __LINE__); \
    OMODULE(Agent)->SetListener(listener, debug);\
}

#endif /*__IAGENT_H__ */
