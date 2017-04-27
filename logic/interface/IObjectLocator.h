/*
 * File: IObjectLocator.h
 * Author: ooeyusea
 *
 * Created On April 04, 2017, 06:57 AM
 */

#ifndef __IOBJECTLOCATOR_H__
#define __IOBJECTLOCATOR_H__
 
#include "IModule.h"

class IObjectLocator : public IModule {
public:
	virtual ~IObjectLocator() {}

	virtual void Report(s64 id, s32 gate, s32 logic) = 0;
	virtual void Erase(s64 id) = 0;

	virtual s32 FindObjectLogic(s64 id) = 0;
	virtual s32 FindObjectGate(s64 id) = 0;
};

#endif /*__IOBJECTLOCATOR_H__ */
