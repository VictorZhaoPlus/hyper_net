#ifndef __STARTER_H__
#define __STARTER_H__
#include "util.h"
#include "ICluster.h"
#include <unordered_map>
#include "IHarbor.h"

class ICapacitySubscriber;
class StartNodeTimer;
class Starter : public IStarter, public INodeListener {
	struct Execute {
		s32 type;
		s32 min;
		s32 max;
		s32 score;
		s32 rate;
		s32 delay;
		StartNodeTimer * timer;
	};

	struct Node {
		bool online;
		s64 closeTick;
		s32 rate;
	};

	struct NodeGroup {
		s32 max;
		std::unordered_map<s32, Node> nodes;
	};

public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void SetStrategy(IStartStrategy * strategy) { s_strategy = strategy; }

	virtual void OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port);
	virtual void OnClose(IKernel * kernel, s32 nodeType, s32 nodeId);

	static void OnNodeTimerStart(IKernel * kernel, s32 type, s64 tick);
	static void OnNodeTimer(IKernel * kernel, s32 type, s64 tick);
	static void OnNodeTimerEnd(IKernel * kernel, s32 type, s64 tick);

	static void StartNode(IKernel * kernel, s32 nodeType, s32 nodeId);

    static Starter * Self() { return s_self; }

private:
    static Starter * s_self;
    static IKernel * s_kernel;
	static IHarbor * s_harbor;
	static IStartStrategy * s_strategy;
	static ICapacitySubscriber * s_capacitySubscriber;

	static s32 s_checkInterval;
	static s32 s_deadInterval;

	static std::unordered_map<s32, Execute> s_executes;
	static std::unordered_map<s32, NodeGroup> s_nodes;
};

#endif //__STARTER_H__
