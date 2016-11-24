#include "Harbor.h"
#include "NodeSession.h"
#include "XmlReader.h"
#include "tools.h"
#include <algorithm>
#include "NodeType.h"

Harbor * Harbor::s_harbor = nullptr;
IKernel * Harbor::s_kernel = nullptr;

class NodeCBProtoHandler : public IHarborProtoHandler {
public:
	NodeCBProtoHandler(const NodeCB cb) : _cb(cb) {}
	virtual ~NodeCBProtoHandler() {}

	virtual void DealNodeProto(IKernel * kernel, const s32 nodeType, const s32 nodeId, const void * context, const s32 size) {
		_cb(kernel, nodeType, nodeId, context, size);
	}

private:
	NodeCB _cb;
};

class NodeArgsCBProtoHandler : public IHarborProtoHandler {
public:
	NodeArgsCBProtoHandler(const NodeArgsCB cb) : _cb(cb) {}
	virtual ~NodeArgsCBProtoHandler() {}

	virtual void DealNodeProto(IKernel * kernel, const s32 nodeType, const s32 nodeId, const void * context, const s32 size) {
		OArgs args(context, size);
		_cb(kernel, nodeType, nodeId, args);
	}

private:
	NodeArgsCB _cb;
};

bool Harbor::Initialize(IKernel * kernel) {
    s_harbor = this;
    s_kernel = kernel;
	_nodeType = node_type::INVALID;

    const char * name = kernel->GetCmdArg("name");
    OASSERT(name, "invalid command args, there is no name");

	olib::XmlReader reader;
	std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/server_conf.xml";
	if (!reader.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
		return false;
	}

	const olib::IXmlObject& harbor = reader.Root()["harbor"][0];
	_sendBuffSize = harbor.GetAttributeInt32("send");
	_recvBuffSize = harbor.GetAttributeInt32("recv");
	_reconnectTick = harbor.GetAttributeInt32("reconnect");

	_hide = false;
	const olib::IXmlObject& nodes = harbor["node"];
	for (s32 i = 0; i < nodes.Count(); ++i) {
		s32 type = nodes[i].GetAttributeInt32("type");
		const char * nodeName = nodes[i].GetAttributeString("name");

		if (strcmp(nodeName, name) == 0) {
			_nodeType = type;
			_hide = nodes[i].GetAttributeBoolean("hide");
		}
		_nodeNames[type] = nodeName;
	}

    OASSERT(_nodeType != node_type::INVALID, "invalid node type");

    _nodeId = tools::StringAsInt(kernel->GetCmdArg("node"));
    const char * port = kernel->GetCmdArg("port");
    _port = port ? tools::StringAsInt(port) : 0;

    return true;
}

bool Harbor::Launched(IKernel * kernel) {
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

void Harbor::Send(s32 nodeType, s32 nodeId, const s32 messageId, const OArgs& args) {
	auto itr = _nodes[nodeType].find(nodeId);
	if (itr != _nodes[nodeType].end()) {
		if (!itr->second->PrepareSendNodeMessage(args.GetSize() + sizeof(s32))) {
			OASSERT(false, "send failed");
			return;
		}

		if (!itr->second->SendNodeMessage(&messageId, sizeof(s32))) {
			OASSERT(false, "send failed");
			return;
		}

		if (!itr->second->SendNodeMessage(args.GetContext(), args.GetSize())) {
			OASSERT(false, "send failed");
			return;
		}
	}
}

void Harbor::Brocast(s32 nodeType, const s32 messageId, const OArgs& args) {
	for (auto itr = _nodes[nodeType].begin(); itr != _nodes[nodeType].end(); ++itr) {
		if (!itr->second->PrepareSendNodeMessage(args.GetSize() + sizeof(s32))) {
			OASSERT(false, "send failed");
		}

		if (!itr->second->SendNodeMessage(&messageId, sizeof(s32))) {
			OASSERT(false, "send failed");
			return;
		}

		if (!itr->second->SendNodeMessage(args.GetContext(), args.GetSize())) {
			OASSERT(false, "send failed");
		}
	}
}

void Harbor::Brocast(const s32 messageId, const OArgs& args) {
	for (auto itrNode = _nodes.begin(); itrNode != _nodes.end(); ++itrNode) {
		for (auto itr = itrNode->second.begin(); itr != itrNode->second.end(); ++itr) {
			if (!itr->second->PrepareSendNodeMessage(args.GetSize() + sizeof(s32))) {
				OASSERT(false, "send failed");
			}

			if (!itr->second->SendNodeMessage(&messageId, sizeof(s32))) {
				OASSERT(false, "send failed");
				return;
			}

			if (!itr->second->SendNodeMessage(args.GetContext(), args.GetSize())) {
				OASSERT(false, "send failed");
			}
		}
	}
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

void Harbor::RegProtocolHandler(s32 messageId, const NodeCB& handler, const char * debug) {
	NodeCBProtoHandler * unit = NEW NodeCBProtoHandler(handler);
	unit->SetDebug(debug);

    _handlers[messageId].push_back(unit);
}

void Harbor::RegProtocolArgsHandler(s32 messageId, const NodeArgsCB& handler, const char * debug) {
	NodeArgsCBProtoHandler * unit = NEW NodeArgsCBProtoHandler(handler);
	unit->SetDebug(debug);

	_handlers[messageId].push_back(unit);
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
	auto itr = _handlers.find(messageId);
	if (itr != _handlers.end()) {
		for (auto * handler : itr->second) {
			const char * debug = handler->GetDebug();

			handler->DealNodeProto(kernel, nodeType, nodeId, (const char*)context + sizeof(s32), size - sizeof(s32));
		}
    }
    else {

    }
}

void Harbor::Reconnect(NodeSession * session) {
	s_kernel->Connect(session->GetConnectIp(), session->GetConnectPort(), _sendBuffSize, _recvBuffSize, session);
}
