#ifndef __ANALYSIS_H__
#define __ANALYSIS_H__
#include "util.h"
#include "IAnalysis.h"
#include "IHarbor.h"
#include <unordered_map>
#include "singleton.h"

class ICapacitySubscriber;
class IProtocolMgr;
class Analysis : public IAnalysis, public INodeListener, public ITimer, public OHolder<Analysis> {
	struct DataSample {

	};
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port);
	virtual void OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) {}

	void TestDelayRespone(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args);

	virtual void OnStart(IKernel * kernel, s64 tick) {}
	virtual void OnTimer(IKernel * kernel, s32 beatCount, s64 tick);
	virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {}

	virtual void OnPause(IKernel * kernel, s64 tick) {}
	virtual void OnResume(IKernel * kernel, s64 tick) {}

private:
    IKernel * _kernel;
	IHarbor * _harbor;
	ICapacitySubscriber * _capacitySubscriber;
	IProtocolMgr * _protocolMgr;

	std::unordered_map<s32, std::unordered_map<s32, DataSample>> _nodes;

	s32 _protoTestDelayRespone;
	s32 _protpTestDelay;
};

#endif //__ANALYSIS_H__

