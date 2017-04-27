#include "Cluster.h"
#include "IHarbor.h"
#include "XmlReader.h"
#include "IProtocolMgr.h"
#include "NodeType.h"
#include "OBuffer.h"
#include "proto.h"

bool Cluster::Initialize(IKernel * kernel) {
    _kernel = kernel;

	olib::XmlReader reader;
    std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/server_conf.xml";
    if (!reader.LoadXml(coreConfigPath.c_str())) {
        OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
        return false;
    }

	_ip = reader.Root()["master"][0].GetAttributeString("ip");
    _port = reader.Root()["master"][0].GetAttributeInt32("port");

    return true;
}

bool Cluster::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);

	if (_harbor->GetNodeType() != node_type::MASTER) {
		FIND_MODULE(_protocolMgr, ProtocolMgr);

		_harbor->Connect(_ip.c_str(), _port);
		RGS_HABOR_HANDLER(_protocolMgr->GetId("proto_cluster", "new_node"), Cluster::NewNodeComming);
	}
    return true;
}

bool Cluster::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void Cluster::NewNodeComming(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& buff) {
    OASSERT(buff.GetSize() == sizeof(core_proto::NewNode), "invalid node size");
	core_proto::NewNode& info = *((core_proto::NewNode*)buff.GetContext());
    OASSERT(info.port > 0, "where is harbor port");

	s64 check = ((s64)info.nodeType << 32) | ((s64)info.nodeId);
    if (_openNode.find(check) != _openNode.end())
        return;

    if (info.nodeType == _harbor->GetNodeType() && info.nodeId <= _harbor->GetNodeId())
        return;

    _openNode.insert(check);
    _harbor->Connect(info.ip, info.port);
}
