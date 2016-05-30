/*
 * File: ICapacitySubscriber.h
 * Author: ooeyusea
 *
 * Created On February 03, 2016, 08:11 AM
 */

#ifndef __ICAPACITYSUBSCRIBER_H__
#define __ICAPACITYSUBSCRIBER_H__
 
#include "IModule.h"

class ICapacitySubscriber : public IModule {
public:
	virtual ~ICapacitySubscriber() {}

	virtual s32 Choose(const s32 nodeType) = 0;
	virtual bool CheckOverLoad(const s32 nodeType, const s32 overload) = 0;
	virtual s32 GetOverLoad(const s32 nodeType, const s32 nodeId) = 0;
};

#endif /*__ICAPACITYSUBSCRIBER_H__ */