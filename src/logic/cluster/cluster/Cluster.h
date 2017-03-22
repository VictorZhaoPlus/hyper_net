#ifndef __CLUSTER_H__
#define __CLUSTER_H__
#include "util.h"
#include "IModule.h"
#include <unordered_set>

class IHarbor;
class OBuffer;
class IProtocolMgr;
class Cluster : public IModule {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

private:
    void NewNodeComming(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& size);

private:
    IKernel * _kernel;
    IHarbor * _harbor;
	IProtocolMgr * _protocolMgr;

    std::unordered_set<s64> _openNode;
    std::string _ip;
    s32 _port;
};

#endif //__CLUSTER_H__

