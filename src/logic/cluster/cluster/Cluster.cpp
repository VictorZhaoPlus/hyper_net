#include "Cluster.h"
#include "IHarbor.h"
#include "tinyxml.h"
#include "CoreProtocol.h"
#include "NodeType.h"
#include "OBuffer.h"

bool Cluster::Initialize(IKernel * kernel) {
    _kernel = kernel;

    TiXmlDocument doc;
    std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/server_conf.xml";
    if (!doc.LoadFile(coreConfigPath.c_str())) {
        OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
        return false;
    }

    const TiXmlElement * root = doc.RootElement();
    OASSERT(root != nullptr, "core xml format error");

    const TiXmlElement * p = root->FirstChildElement("master");
    _ip = p->Attribute("ip");
    _port = tools::StringAsInt(p->Attribute("port"));

    return true;
}

bool Cluster::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);

	if (_harbor->GetNodeType() != node_type::MASTER) {
		_harbor->Connect(_ip.c_str(), _port);
		RGS_HABOR_HANDLER(core_proto::NEW_NODE, Cluster::NewNodeComming);
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
