#ifndef __ID_H__
#define __ID_H__
#include "util.h"
#include "singleton.h"
#include "IKernel.h"
#include <vector>
using namespace core;

class Id : public OSingleton<Id>{
	friend class OSingleton<Id>;
public:
    bool Ready();
    bool Initialize();
    bool Destroy();

	inline s32 GetId(const char * group, const char * name) {
		auto itr = _protos[group].find(name);
		OASSERT(itr != _protos[group].end(), "where is %s/%s", group, name);
		if (itr != _protos[group].end())
			return itr->second;
		return 0;
	}

private:

	std::unordered_map<std::string, std::unordered_map<std::string, s32>> _protos;
	std::unordered_map<std::string, s32> _nextProtos;
};

#endif //__ASYNCMGR_H__

