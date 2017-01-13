/*
 * File: IShadowMgr.h
 * Author: ooeyusea
 *
 * Created On January 13, 2017, 09:08 AM
 */

#ifndef __ISHADOWMGR_H__
#define __ISHADOWMGR_H__
 
#include "IModule.h"

class IObject;
class IShadowMgr : public IModule {
public:
	virtual ~IShadowMgr() {}

	virtual void Project(IObject * object, s32 logic) = 0;
	virtual void Unproject(IObject * object, s32 logic) = 0;
};

#endif /*__ISHADOWMGR_H__ */