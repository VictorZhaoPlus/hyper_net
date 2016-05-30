/*
 * File: IStarter.h
 * Author: ooeyusea
 *
 * Created On January 29, 2016, 07:29 AM
 */

#ifndef __ISTARTER_H__
#define __ISTARTER_H__
 
#include "IModule.h"

class IStartStrategy {
public:
	virtual ~IStartStrategy() {}

	virtual s32 ChooseNode(const s32 nodeType) = 0;
	virtual void AddSlave(const s32 nodeId) = 0;
};

class IStarter : public IModule {
public:
	virtual ~IStarter() {}

	virtual void SetStrategy(IStartStrategy * strategy) = 0;
};

#endif /*__ISTARTER_H__ */