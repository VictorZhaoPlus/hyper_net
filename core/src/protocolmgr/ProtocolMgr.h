#ifndef __PROTOCOLMGR_H__
#define __PROTOCOLMGR_H__
#include "util.h"
#include "IProtocolMgr.h"
#include "singleton.h"
#include <unordered_map>

#define MAX_PROTOCOL_NAME_LEN 64
class ProtocolMgr : public IProtocolMgr, public OHolder<ProtocolMgr> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual s32 GetId(const char * group, const char * name);

private:
    IKernel * _kernel;

	std::unordered_map<std::string, std::unordered_map<std::string, s32>> _protos;
	std::unordered_map<std::string, s32> _nextProtos;
};

#endif //__PROTOCOLMGR_H__

