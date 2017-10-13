/*
 * File: ISave.h
 * Author: ooeyusea
 *
 * Created On July 03, 2017, 08:28 AM
 */

#ifndef __ISAVE_H__
#define __ISAVE_H__
 
#include "IModule.h"

class IObject;
class IBuffer;
class ISave : public IModule {
public:
	virtual ~ISave() {}

	virtual void RegObjectPartSavor(const char * type, const std::function<void(IKernel * kernel, IObject * object, IBuffer& buf)>& f, const char * debug) = 0;
};

#define RGS_OBJECT_PART_SAVOR(type, handler) OMODULE(Save)->RegObjectPartSavor(type, std::bind(&handler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), #handler)

#endif /*__ISAVE_H__ */