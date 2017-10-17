#ifndef __DISTRIBUTION_H__
#define __DISTRIBUTION_H__
#include "util.h"
#include "ILogin.h"
#include "singleton.h"
#include <unordered_map>
#include "IHarbor.h"
#include <vector>

class IProtocolMgr;
class Distribution : public IModule, public INodeListener, public OHolder<Distribution> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void SetStrategy(IDistributionStrategy * strategy) { _strategy = strategy; }
	s32 ChooseLogic(s64 actorId);

	virtual void OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port);
	virtual void OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) {}

	void OnRecvDistributeLogic(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);
	void OnRecvAddPlayer(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);
	void OnRecvRemovePlayer(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);

private:
    IKernel * _kernel;
	IDistributionStrategy * _strategy;

	std::unordered_map<s64, s32> _distributes;
	std::unordered_map<s64, s32> _players;

	std::vector<s32> _logices;
};

#endif //__DISTRIBUTION_H__
