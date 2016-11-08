#ifndef __STARTER_H__
#define __STARTER_H__
#include "util.h"
#include "ICluster.h"
#include <unordered_map>
#include "IHarbor.h"
#include "singleton.h"

class ICapacitySubscriber;
class StartNodeTimer;
class Starter : public IStarter, public INodeListener, public OHolder<Starter> {
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

	virtual void SetStrategy(IStartStrategy * strategy) { _strategy = strategy; }

	virtual void OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port);
	virtual void OnClose(IKernel * kernel, s32 nodeType, s32 nodeId);

	void OnNodeTimerStart(IKernel * kernel, s32 type, s64 tick);
	void OnNodeTimer(IKernel * kernel, s32 type, s64 tick);
	void OnNodeTimerEnd(IKernel * kernel, s32 type, s64 tick);

	void StartNode(IKernel * kernel, s32 nodeType, s32 nodeId);

private:
    IKernel * _kernel;
	IHarbor * _harbor;
	IStartStrategy * _strategy;
	ICapacitySubscriber * _capacitySubscriber;

	s32 _checkInterval;
	s32 _deadInterval;

	std::unordered_map<s32, Execute> _executes;
	std::unordered_map<s32, NodeGroup> _nodes;
};

#endif //__STARTER_H__
