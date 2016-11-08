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

private:
    IKernel * _kernel;
	IHarbor * _harbor;

	s32 _load;
};

#endif //__CAPACITYPUBLISHER_H__

