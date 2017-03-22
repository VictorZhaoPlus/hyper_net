#ifndef __ICLUSTER_H__
#define __ICLUSTER_H__

#include "IModule.h"

class IStartStrategy {
public:
	enum {
		INVALID_NODE = 0,
	};

	virtual ~IStartStrategy() {}

	virtual s32 ChooseNode(const s32 nodeType) = 0;
	virtual void AddSlave(const s32 nodeId) = 0;
};

class IStarter : public IModule {
public:
	virtual ~IStarter() {}

	virtual void SetStrategy(IStartStrategy * strategy) = 0;
};

#endif //__ICLUSTER_H__
