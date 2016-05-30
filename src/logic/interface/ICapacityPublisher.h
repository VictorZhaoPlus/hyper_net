/*
 * File: ICapacityPublisher.h
 * Author: ooeyusea
 *
 * Created On February 03, 2016, 08:11 AM
 */

#ifndef __ICAPACITYPUBLISHER_H__
#define __ICAPACITYPUBLISHER_H__
 
#include "IModule.h"

class ICapacityPublisher : public IModule {
public:
	virtual ~ICapacityPublisher() {}

	virtual void IncreaceLoad(const s32 value) = 0;
	virtual void DecreaceLoad(const s32 value) = 0;
};

#endif /*__ICAPACITYPUBLISHER_H__ */