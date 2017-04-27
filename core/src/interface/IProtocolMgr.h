/*
 * File: IProtocolMgr.h
 * Author: ooeyusea
 *
 * Created On November 14, 2016, 02:37 AM
 */

#ifndef __IPROTOCOLMGR_H__
#define __IPROTOCOLMGR_H__
 
#include "IModule.h"

class IProtocolMgr : public IModule {
public:
	virtual ~IProtocolMgr() {}

	virtual s32 GetId(const char * group, const char * name) = 0;
	virtual const char * GetDesc(const char * group, const s32 id) = 0;
};

#endif /*__IPROTOCOLMGR_H__ */