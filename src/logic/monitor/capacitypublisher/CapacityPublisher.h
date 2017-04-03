#ifndef __CAPACITYPUBLISHER_H__
#define __CAPACITYPUBLISHER_H__
#include "util.h"
#include "IMonitor.h"
#include "IHarbor.h"

class IProtocolMgr;
class CapacityPublisher : public ICapacityPublisher, public ITimer, public INodeListener {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void IncreaceLoad(const s32 value);
	virtual void DecreaceLoad(const s32 value);

	virtual void OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port);
	virtual void OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) {}

	virtual void OnStart(IKernel * kernel, s64 tick) {}
	virtual void OnTimer(IKernel * kernel, s32 beatCount, s64 tick);
	virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {}

private:
    IKernel * _kernel;
	IHarbor * _harbor;
	IProtocolMgr * _protocolMgr;

	s32 _load;
	bool _changed;

	s32 _protoOverLoad;
};

#endif //__CAPACITYPUBLISHER_H__

