#include "AgentSession.h"
#include "Agent.h"

AgentSession::AgentSession() 
	: _id (0) {
}

AgentSession::~AgentSession() {

}

void AgentSession::OnConnected(IKernel * kernel) {
	_id = Agent::OnOpen(this);
}

s32 AgentSession::OnRecv(IKernel * kernel, const void * context, const s32 size) {
	return Agent::OnRecv(_id, context, size);
}

void AgentSession::OnError(IKernel * kernel, const s32 error) {

}

void AgentSession::OnDisconnected(IKernel * kernel) {
	Agent::OnClose(_id);
}
