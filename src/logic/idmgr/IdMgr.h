#ifndef __IDMGR_H__
#define __IDMGR_H__
#include "util.h"
#include "IIdMgr.h"
#include "singleton.h"
#include <vector>

class OArgs;
class IHarbor;
class IdMgr : public IIdMgr, public OHolder<IdMgr>, public ITimer {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual s64 AllocId();

	virtual void OnStart(IKernel * kernel, s64 tick) {}
	virtual void OnTimer(IKernel * kernel, s64 tick);
	virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {}

	virtual void OnPause(IKernel * kernel, s64 tick) {}
	virtual void OnResume(IKernel * kernel, s64 tick) {}

	s64 GenerateId();

	void AskId(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);
	void GiveId(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);

private:
    IKernel * _kernel;
	IHarbor * _harbor;

	bool _multiProcess;
	s32 _nodeType;
	s32 _AskProtocolId;
	s32 _GiveProtocolId;
	u32 _areaId;
	s32 _poolSize;

	std::vector<s64> _ids;
};

#endif //__IDMGR_H__

