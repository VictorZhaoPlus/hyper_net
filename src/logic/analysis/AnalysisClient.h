#ifndef __ANALYSIS_CLIENT_H__
#define __ANALYSIS_CLIENT_H__
#include "util.h"
#include "IAnalysis.h"

class IHarbor;
class OArgs;
class AnalysisClient : public IAnalysisClient {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	static void TestDelay(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args);

	static AnalysisClient * Self() { return s_self; }

private:
	static AnalysisClient * s_self;
    static IKernel * s_kernel;
	static IHarbor * s_harbor;
};

#endif //__ANALYSIS_CLIENT_H__

