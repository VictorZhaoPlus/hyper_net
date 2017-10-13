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
};

namespace protocol_mgr {
	template <s64, s64>
	struct IdGetter {
		inline static const s32 Get(const char * group, const char * name) {
			static const s32 ret = OMODULE(ProtocolMgr)->GetId(group, name);
			return ret;
		}
	};
}

#define PROTOCOL_ID(module, name) (protocol_mgr::IdGetter<CalcUniqueId(0, module), CalcUniqueId(0, name)>::Get(module, name))

#endif /*__IPROTOCOLMGR_H__ */