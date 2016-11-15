#ifndef __PROTOCOLMGR_H__
#define __PROTOCOLMGR_H__
#include "util.h"
#include "IProtocolMgr.h"
#include "singleton.h"
#include <unordered_map>
#include <OString.h>

#define MAX_PROTOCOL_NAME_LEN 64
#define MAX_PROTOCOL_DESC_LEN 1024
class ProtocolMgr : public IProtocolMgr, public OHolder<ProtocolMgr> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual s32 GetId(const char * group, const char * name);
	virtual const char * GetDesc(const char * group, const s32 id);

private:
    IKernel * _kernel;

	std::unordered_map<olib::OString<MAX_PROTOCOL_NAME_LEN>, std::unordered_map<olib::OString<MAX_PROTOCOL_NAME_LEN>, s32, olib::OStringHash<MAX_PROTOCOL_NAME_LEN>>, olib::OStringHash<MAX_PROTOCOL_NAME_LEN>> _protos;
	std::unordered_map<olib::OString<MAX_PROTOCOL_NAME_LEN>, std::unordered_map<s32, olib::OString<MAX_PROTOCOL_DESC_LEN>>, olib::OStringHash<MAX_PROTOCOL_NAME_LEN>> _descs;
};

#endif //__PROTOCOLMGR_H__
