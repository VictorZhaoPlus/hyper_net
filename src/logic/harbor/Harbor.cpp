#include "Harbor.h"
#include "NodeSession.h"
#include "XmlReader.h"
#include "tools.h"
#include <algorithm>
#include "IMaster.h"

Harbor * Harbor::s_harbor = nullptr;
IKernel * Harbor::s_kernel = nullptr;

bool Harbor::Initialize(IKernel * kernel) {
    s_harbor = this;
    s_kernel = kernel;

    const char * name = kernel->GetCmdArg("name");
    OASSERT(name, "invalid command args, there is no name");

	olib::XmlReader reader;
	std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/server_conf.xml";
	if (!reader.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
		return false;
	}

	olib::IXmlObject& harbor = reader.Root()["harbor"][0];
	_sendBuffSize = harbor.GetAttributeInt32("send");
	_recvBuffSize = harbor.GetAttributeInt32("recv");
	_reconnectTick = harbor.GetAttributeInt32("reconnect");

	_hide = false;
	olib::IXmlObject& nodes = harbor["node"];
	for (s32 i = 0; i < nodes.Count(); ++i) {
		s32 type = nodes[i].GetAttributeInt32("type");
		const char * nodeName = nodes[i].GetAttributeString("name");

		if (strcmp(nodeName, name) == 0) {
			_nodeType = type;
			_hide = nodes[i].GetAttributeBoolean("hide");
		}
		_nodeNames[type] = nodeName;
	}

    OASSERT(_nodeType > 0, "invalid node type");

    _nodeId = tools::StringAsInt(kernel->GetCmdArg("node"));
    const char * port = kernel->GetCmdArg("port");
    _port = port ? tools::StringAsInt(port) : 0;

    return true;
}

bool Harbor::Launched(IKernel * kernel) {
    IMaster * master =(IMaster*)kernel->FindModule("Master");
    if (master) {
        _port = master->GetPort();
    }

    if (_port) {
        if (!kernel->Listen("0.0.0.0", _port, _sendBuffSize, _recvBuffSize, this)) {
            OASSERT(false, "listen failed");
            return false;
        }
    }

    return true;
}

bool Harbor::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

ISession * Harbor::Create() {
    return NEW NodeSession;
}

void Harbor::Recover(ISession * session) {
    DEL session;
}

void Harbor::Connect(const char * ip, const s32 port) {
    core::ISession * session = Create();
    OASSERT(session != nullptr, "create session failed");

    ((NodeSession*)session)->SetConnect(ip, port);

    s_kernel->Connect(ip, port, _sendBuffSize, _recvBuffSize, session);
}

void Harbor::AddNodeListener(INodeListener * listener, const char * debug) {
#ifdef _DEBUG
    auto itr = std::find_if(_listeners.begin(), _listeners.end(), [listener](const NodeListenerUnit& unit) -> bool {
        return unit.listener == listener;
    });
    OASSERT(itr == _listeners.end(), "node listener is already add");
#endif
    NodeListenerUnit unit;
    unit.listener = listener;
    SafeSprintf(unit.debug, sizeof(unit.debug), debug);

    _listeners.push_back(unit);
}

bool Harbor::PrepareSend(s32 nodeType, s32 nodeId, const s32 messageId, const s32 size) {
    auto itr = _nodes[nodeType].find(nodeId);
    if (itr != _nodes[nodeType].end()) {
        if (!itr->second->PrepareSendNodeMessage(size + sizeof(s32)))
            return false;
        return itr->second->SendNodeMessage(&messageId, sizeof(s32));
    }
    return false;
}

bool Harbor::Send(s32 nodeType, s32 nodeId, const void * context, const s32 size) {
    auto itr = _nodes[nodeType].find(nodeId);
    if (itr != _nodes[nodeType].end())
        return itr->second->SendNodeMessage(context, size);
    return false;
}

void Harbor::PrepareBrocast(s32 nodeType, const s32 messageId, const s32 size) {
    for (auto itr = _nodes[nodeType].begin(); itr != _nodes[nodeType].end(); ++itr) {
        itr->second->PrepareSendNodeMessage(size + sizeof(s32));
        itr->second->SendNodeMessage(&messageId, sizeof(s32));
    }
}

void Harbor::Brocast(s32 nodeType, const void * context, const s32 size) {
    for (auto itr = _nodes[nodeType].begin(); itr != _nodes[nodeType].end(); ++itr)
        itr->second->SendNodeMessage(context, size);
}

void Harbor::PrepareBrocast(const s32 messageId, const s32 size) {
	for (auto itrType = _nodes.begin(); itrType != _nodes.end(); ++itrType) {
		for (auto itr = itrType->second.begin(); itr != itrType->second.end(); ++itr) {
			itr->second->PrepareSendNodeMessage(size + sizeof(s32));
			itr->second->SendNodeMessage(&messageId, sizeof(s32));
		}
	}
}

void Harbor::Brocast(const void * context, const s32 size) {
	for (auto itrType = _nodes.begin(); itrType != _nodes.end(); ++itrType) {
		for (auto itr = itrType->second.begin(); itr != itrType->second.end(); ++itr) {
			itr->second->SendNodeMessage(context, size);
		}
	}
}

void Harbor::RegProtocolHandler(s32 nodeType, s32 messageId, const NodeProtocolHandlerType& handler, const char * debug) {
    Handler unit;
    unit.func = handler;
    SafeSprintf(unit.debug, sizeof(unit.debug), debug);

    _handlers[nodeType][messageId] = unit;
}

void Harbor::OnNodeOpen(IKernel * kernel, const s32 nodeType, const s32 nodeId, const char * ip, const s32 port, const bool hide, NodeSession * session) {
    _nodes[nodeType][nodeId] = session;

    DBG_INFO("node %s:%d opened", _nodeNames[nodeType].c_str(), nodeId);

    for (auto& unit : _listeners) {
        unit.listener->OnOpen(kernel, nodeType, nodeId, hide, ip, port);
    }
}

void Harbor::OnNodeClose(IKernel * kernel, const s32 nodeType, const s32 nodeId) {
    _nodes[nodeType].erase(nodeId);

    DBG_INFO("node %s:%d closed", _nodeNames[nodeType].c_str(), nodeId);

    for (auto& unit : _listeners) {
        unit.listener->OnClose(kernel, nodeType, nodeId);
    }
}

void Harbor::OnNodeMessage(IKernel * kernel, const s32 nodeType, const s32 nodeId, const void * context, const s32 size) {
    OASSERT(size >= sizeof(s32), "message size invalid");

    s32 messageId = *(s32*)context;
    NodeProtocolHandlerType handler = nullptr;
    const char * debug = nullptr;
    auto itr = _handlers[nodeType].find(messageId);
    if (itr == _handlers[nodeType].end()) {
        itr = _handlers[ANY_NODE].find(messageId);
        if (itr != _handlers[ANY_NODE].end()) {
            handler = itr->second.func;
            debug = itr->second.debug;
        }
    }
    else {
        handler = itr->second.func;
        debug = itr->second.debug;
    }

    if (handler != nullptr) {
        handler(kernel, nodeType, nodeId, (char*)context + sizeof(s32), size - sizeof(s32));
    }
    else {

    }
}

void Harbor::Reconnect(NodeSession * session) {
	s_kernel->Connect(session->GetConnectIp(), session->GetConnectPort(), _sendBuffSize, _recvBuffSize, session);
}
