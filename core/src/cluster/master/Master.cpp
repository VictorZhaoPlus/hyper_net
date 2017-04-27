#include "Master.h"
#include "IProtocolMgr.h"
#include "NodeType.h"

bool Master::Initialize(IKernel * kernel) {
    _kernel = kernel;
    return true;
}

bool Master::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);

	if (_harbor->GetNodeType() == node_type::MASTER) {
		_harbor->AddNodeListener(this, "Master");

		FIND_MODULE(_protocolMgr, ProtocolMgr);
		_protoNewNode = _protocolMgr->GetId("proto_cluster", "new_node");
	}
    return true;
}

bool Master::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void Master::OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port) {
    if (hide)
        return;

    s64 check = ((s64)nodeType << 32) | ((s64)nodeId);
#ifdef _DEBUG
    {
        auto itr = _nodes.find(check);
        if (itr != _nodes.end()) {
            OASSERT(strcmp(ip, itr->second.ip) == 0 && port == itr->second.port, "wtf");
        }
    }
#endif
    NodeInfo& info = _nodes[check];
    info.nodeType = nodeType;
    info.nodeId = nodeId;
    SafeSprintf(info.ip, sizeof(info.ip), ip);
    info.port = port;

    for (auto itr = _nodes.begin(); itr != _nodes.end(); ++itr) {
		if (itr->first != check) {
			SendNewNode(kernel, itr->second.nodeType, itr->second.nodeId, nodeType, nodeId, ip, port);
			SendNewNode(kernel, nodeType, nodeId, itr->second.nodeType, itr->second.nodeId, itr->second.ip, itr->second.port);
		}
    }
}

void Master::OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) {

}

void Master::SendNewNode(IKernel * kernel, s32 nodeType, s32 nodeId, s32 newNodeType, s32 newNodeId, const char * ip, s32 port) {
    core_proto::NewNode info;
    info.nodeType = newNodeType;
	info.nodeId = newNodeId;
    SafeSprintf(info.ip, sizeof(info.ip), ip);
    info.port = port;

    _harbor->PrepareSend(nodeType, nodeId, _protoNewNode, sizeof(info));
    _harbor->Send(nodeType, nodeId, &info, sizeof(info));
}
