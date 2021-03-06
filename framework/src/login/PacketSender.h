#ifndef __PACKETSENDER_H__
#define __PACKETSENDER_H__
#include "util.h"
#include "ILogin.h"
#include "singleton.h"

class PacketSender : public IPacketSender, public OHolder<PacketSender> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void Send(const s32 gate, const s64 actorId, const s32 msgId, const OBuffer& buf, s8 delay = 0);
	virtual void Brocast(const std::unordered_map<s32, std::vector<s64>>& actors, const s32 msgId, const OBuffer& buf, s8 delay = 0);
	virtual void Brocast(const s32 msgId, const OBuffer& buf, s8 delay = 0);

private:
    IKernel * _kernel;
};

#endif //__PACKETSENDER_H__
