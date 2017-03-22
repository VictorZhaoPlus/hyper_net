#ifndef __SLAVE_H__
#define __SLAVE_H__
#include "util.h"
#include "IModule.h"
#include "Define.h"
#include <unordered_map>

class IHarbor;
class OArgs;
class IProtocolMgr;
class Slave : public IModule {
	struct Execute {
		s32 type;
		char name[MAX_NODE_NAME_LEN];
		char cmd[MAX_CMD_LEN];
	};

	struct Node {
		char cmd[MAX_CMD_LEN];
	};

public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	void OpenNewNode(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);

private:
	void StartNode(IKernel * kernel, const char * cmd);
	void StartNewNode(IKernel * kernel, const char * name, const char * cmd, const s32 nodeType, const s32 nodeId);

private:
    IKernel * _kernel;
	IHarbor * _harbor;
	IProtocolMgr * _protocolMgr;

	s32 _startPort;
	s32 _endPort;
	s32 _startOutPort;
	s32 _endOutPort;
	std::unordered_map<s32, Execute> _executes;
	std::unordered_map<s64, Node> _cmds;
};

#endif //__SLAVE_H__

