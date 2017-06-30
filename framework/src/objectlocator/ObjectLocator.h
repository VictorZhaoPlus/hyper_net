#ifndef __OBJECTLOCATOR_H__
#define __OBJECTLOCATOR_H__
#include "util.h"
#include "IObjectLocator.h"
#include "singleton.h"
#include <unordered_map>

class OArgs;
class ObjectLocator : public IObjectLocator, public OHolder<ObjectLocator> {
	struct Locator {
		s32 gate;
		s32 logic;
	};
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void Report(s64 id, s32 gate, s32 logic);
	virtual void Erase(s64 id);
	void OnReport(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args);

	virtual s32 FindObjectLogic(s64 id);
	virtual s32 FindObjectGate(s64 id);

private:
    IKernel * _kernel;

	std::unordered_map<s64, Locator> _objects;
};

#endif //__OBJECTLOCATOR_H__

