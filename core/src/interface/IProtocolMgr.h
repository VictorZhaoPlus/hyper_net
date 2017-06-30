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

namespace protocol_mgr {
	constexpr s64 CalcUniqueId(s64 hash, const char * str) {
		return *str ? CalcUniqueId((hash * 131 + (*str)) % 4294967295, str + 1) : hash;
	}

	template <s64, s64>
	struct IdGetter {
		inline static const s32 Get(const char * group, const char * name) {
			static const s32 ret = OMODULE(ProtocolMgr)->GetId(group, name);
			return ret;
		}
	};

	template <s64, s64>
	struct DescGetter {
		inline static const char * Get(const char * group, const char * name) {
			static const char * ret = OMODULE(ProtocolMgr)->GetDesc(group, name);
			return ret;
		}
	};
}

#define PROTOCOL_ID(module, name) (protocol_mgr::IdGetter<protocol_mgr::CalcUniqueId(0, module), protocol_mgr::CalcUniqueId(0, name)>::Get(module, name))
#define PROTOCOL_DESC(module, name) (protocol_mgr::DescGetter<protocol_mgr::CalcUniqueId(0, module), protocol_mgr::CalcUniqueId(0, name)>::Get(module, name))

#endif /*__IPROTOCOLMGR_H__ */