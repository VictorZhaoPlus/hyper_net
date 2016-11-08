#ifndef __MASTER_H__
#define __MASTER_H__
#include "util.h"
#include "ICluster.h"
#include "IHarbor.h"
#include <unordered_map>
#include "Define.h"

class Master : public IModule, public INodeListener {
    struct NodeInfo {
        s32 nodeType;
        s32 nodeId;
        char ip[MAX_IP_SIZE];
        s32 port;
    };

public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

    virtual void OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port);
    virtual void OnClose(IKernel * kernel, s32 nodeType, s32 nodeId);

private:
    void SendNewNode(IKernel * kernel, s32 nodeType, s32 nodeId, s32 newNodeType, s32 newNodeId, const char * ip, s32 port);

private:
    IKernel * _kernel;
    IHarbor * _harbor;

    std::unordered_map<s64, NodeInfo> _nodes;
};

#endif //__MASTER_H__

