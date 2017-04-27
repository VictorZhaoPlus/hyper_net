#ifndef __ANALYSIS_CLIENT_H__
#define __ANALYSIS_CLIENT_H__
#include "util.h"
#include "IAnalysis.h"
#include "singleton.h"

class IHarbor;
class OArgs;
class AnalysisClient : public IAnalysisClient, public OHolder<AnalysisClient>{
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	void TestDelay(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args);

private:
    IKernel * _kernel;
	IHarbor * _harbor;
};

#endif //__ANALYSIS_CLIENT_H__

