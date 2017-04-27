/*
 * File: IIdMgr.h
 * Author: ooeyusea
 *
 * Created On November 08, 2016, 03:10 AM
 */

#ifndef __IIDMGR_H__
#define __IIDMGR_H__
 
#include "IModule.h"

class IIdMgr : public IModule {
public:
	virtual ~IIdMgr() {}

	virtual s64 AllocId() = 0;
};

#endif /*__IIDMGR_H__ */