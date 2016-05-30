#ifndef __STARTNODESTRATEGY_H__
#define __STARTNODESTRATEGY_H__
#include "util.h"
#include "IStartNodeStrategy.h"
#include "IStarter.h"
#include <unordered_map>

class StartNodeStrategy : public IStartNodeStrategy, public IStartStrategy {
	enum {
		OVERLOAD = 0,
		BANDWIDTH,

		DATA_COUNT,
	};
	struct Score {
		s32 value[DATA_COUNT];
	};
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual s32 ChooseNode(const s32 nodeType);
	virtual void AddSlave(const s32 nodeId);

	static s32 Choose(IKernel * kernel, s32 first, s32 second);

	static StartNodeStrategy * Self() { return s_self; }

private:
	static StartNodeStrategy * s_self;
    static IKernel * s_kernel;
	static IStarter * s_starter;

	static std::unordered_map<s32, Score> s_scores;
	static std::unordered_map<s32, Score> s_slaves;
};

#endif //__STARTNODESTRATEGY_H__

