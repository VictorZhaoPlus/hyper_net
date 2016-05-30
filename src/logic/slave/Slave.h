#ifndef __SLAVE_H__
#define __SLAVE_H__
#include "util.h"
#include "ISlave.h"
#include "Define.h"
#include <unordered_map>

class IHarbor;
class OArgs;
class Slave : public ISlave {
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

	static void OpenNewNode(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);

	static Slave * Self() { return s_self; }

private:
	static void StartNode(IKernel * kernel, const char * cmd);
	static void StartNewNode(IKernel * kernel, const char * name, const char * cmd, const s32 nodeType, const s32 nodeId);

private:
	static Slave * s_self;
    static IKernel * s_kernel;
	static IHarbor * s_harbor;

	static s32 s_startPort;
	static s32 s_endPort;
	static s32 s_startOutPort;
	static s32 s_endOutPort;
	static std::unordered_map<s32, Execute> s_executes;
	static std::unordered_map<s64, Node> s_cmds;
};

#endif //__SLAVE_H__

