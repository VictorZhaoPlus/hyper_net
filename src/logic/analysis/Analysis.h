#ifndef __ANALYSIS_H__
#define __ANALYSIS_H__
#include "util.h"
#include "IAnalysis.h"
#include "IHarbor.h"
#include <unordered_map>

class ICapacitySubscriber;
class Analysis : public IAnalysis, public INodeListener, public ITimer {
	struct DataSample {

	};
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port);
	virtual void OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) {}

	static void TestDelayRespone(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args);

	virtual void OnStart(IKernel * kernel, s64 tick) {}
	virtual void OnTimer(IKernel * kernel, s64 tick);
	virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {}

	virtual void OnPause(IKernel * kernel, s64 tick) {}
	virtual void OnResume(IKernel * kernel, s64 tick) {}

	static Analysis * Self() { return s_self; }

private:
	static Analysis * s_self;
    static IKernel * s_kernel;
	static IHarbor * s_harbor;
	static ICapacitySubscriber * s_capacitySubscriber;

	static std::unordered_map<s32, std::unordered_map<s32, DataSample>> s_nodes;
};

#endif //__ANALYSIS_H__

