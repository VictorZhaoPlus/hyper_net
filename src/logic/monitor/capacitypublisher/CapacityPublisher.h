#ifndef __CAPACITYPUBLISHER_H__
#define __CAPACITYPUBLISHER_H__
#include "util.h"
#include "IMonitor.h"
#include "IHarbor.h"

class CapacityPublisher : public ICapacityPublisher, public INodeListener {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void IncreaceLoad(const s32 value);
	virtual void DecreaceLoad(const s32 value);

	virtual void OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port);
	virtual void OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) {}

	static CapacityPublisher * Self() { return s_self; }

private:
	static CapacityPublisher * s_self;
    static IKernel * s_kernel;
	static IHarbor * s_harbor;

	static s32 s_load;
};

#endif //__CAPACITYPUBLISHER_H__

