#ifndef __CAPACITYSUBSCRIBER_H__
#define __CAPACITYSUBSCRIBER_H__
#include "util.h"
#include "IMonitor.h"
#include "IHarbor.h"
#include <unordered_map>

class CapacitySubscriber : public ICapacitySubscriber, public INodeListener {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual s32 Choose(const s32 nodeType);
	virtual bool CheckOverLoad(const s32 nodeType, const s32 overload);
	virtual s32 GetOverLoad(const s32 nodeType, const s32 nodeId);

	static void ReadLoad(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args);

	virtual void OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port) {}
	virtual void OnClose(IKernel * kernel, s32 nodeType, s32 nodeId);

	static CapacitySubscriber * Self() { return s_self; }

private:
	static CapacitySubscriber * s_self;
    static IKernel * s_kernel;
	static IHarbor * s_harbor;

	static std::unordered_map<s32, std::unordered_map<s32, s32>> s_servers;
};

#endif //__CAPACITYSUBSCRIBER_H__

