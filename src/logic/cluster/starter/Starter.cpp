#include "Starter.h"
#include "IHarbor.h"
#include "XmlReader.h"
#include "NodeType.h"
#include "ICapacitySubscriber.h"
#include "IProtocolMgr.h"
#include "StartNodeTimer.h"
#include "OArgs.h"

bool Starter::Initialize(IKernel * kernel) {
    _kernel = kernel;

	olib::XmlReader conf;
	std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/server_conf.xml";
	if (!conf.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "wtf");
		return false;
	}

	const olib::IXmlObject& starter = conf.Root()["starter"][0];
	_checkInterval = starter.GetAttributeInt32("check");
	_deadInterval = starter.GetAttributeInt32("dead");

	if (starter.IsExist("node")) {
		const olib::IXmlObject& nodes = starter["node"];
		for (s32 i = 0; i < nodes.Count(); ++i) {
			Execute info;
			info.type = nodes[i].GetAttributeInt32("nodeType");
			info.min = nodes[i].GetAttributeInt32("min");
			info.max = nodes[i].GetAttributeInt32("max");
			info.score = nodes[i].GetAttributeInt32("score");
			info.delay = nodes[i].GetAttributeInt32("delay");
			info.timer = nullptr;

			_executes[info.type] = info;
		}
	}

	SetStrategy(this);
    return true;
}

bool Starter::Launched(IKernel * kernel) {
	_harbor = (IHarbor*)kernel->FindModule("Harbor");
	OASSERT(_harbor, "where is harbor");

	if (_harbor->GetNodeType() == node_type::MASTER) {
		_harbor->AddNodeListener(this, "Starter");

		_capacitySubscriber = (ICapacitySubscriber *)kernel->FindModule("CapacitySubscriber");
		OASSERT(_capacitySubscriber, "where is CapacitySubscriber");

		for (auto itr = _executes.begin(); itr != _executes.end(); ++itr) {
			itr->second.timer = StartNodeTimer::Create(itr->first);
			kernel->StartTimer(itr->second.timer, itr->second.delay, TIMER_BEAT_FOREVER, _checkInterval);
		}

		FIND_MODULE(_protocolMgr, ProtocolMgr);
		_protoStartNode = _protocolMgr->GetId("proto_cluster", "new_node");
	}

    return true;
}

bool Starter::Destroy(IKernel * kernel) {
	for (auto itr = _executes.begin(); itr != _executes.end(); ++itr) {
		kernel->KillTimer(itr->second.timer);
	}

    DEL this;
    return true;
}

s32 Starter::ChooseNode(const s32 nodeType) {
	if (_maxSlaveId > 0)
		return rand() % _maxSlaveId + 1;
	return 0;
}

void Starter::OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port) {
	OASSERT(_strategy, "miss start strategy");
	if (nodeType == node_type::SLAVE) 
		_strategy->AddSlave(nodeId);
	else {
		if (_executes.find(nodeType) != _executes.end()) {
			_nodes[nodeType].nodes[nodeId].online = true;
			_nodes[nodeType].nodes[nodeId].closeTick = 0;
		}
	}
}

void Starter::OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) {
	if (nodeType != node_type::SLAVE) {
		_nodes[nodeType].nodes[nodeId].online = false;
		_nodes[nodeType].nodes[nodeId].closeTick = tools::GetTimeMillisecond();
	}
}

void Starter::OnNodeTimerStart(IKernel * kernel, s32 type, s64 tick) {
	auto itr = _executes.find(type);
	OASSERT(itr != _executes.end(), "wtf");

	if (itr != _executes.end()) {
		for (s32 i = 1; i <= itr->second.min; ++i)
			StartNode(kernel, itr->second.type, i);
		_nodes[itr->first].max = itr->second.min;
	}
}

void Starter::OnNodeTimer(IKernel * kernel, s32 type, s64 tick) {
	auto itr = _nodes.find(type);
	OASSERT(itr != _nodes.end(), "wtf");

	if (itr != _nodes.end()) {
		for (auto itrServer = itr->second.nodes.begin(); itrServer != itr->second.nodes.end(); ++itrServer) {
			if (!itrServer->second.online && tick > itrServer->second.closeTick + _deadInterval)
				StartNode(kernel, itr->first, itrServer->first);
		}

		if (_executes[itr->first].max > itr->second.max && _capacitySubscriber->CheckOverLoad(itr->first, _executes[itr->first].rate * itr->second.max)) {
			StartNode(kernel, itr->first, itr->second.max + 1);
			++itr->second.max;
		}
	}
}

void Starter::OnNodeTimerEnd(IKernel * kernel, s32 type, s64 tick) {
	auto itr = _executes.find(type);
	OASSERT(itr != _executes.end(), "wtf");

	if (itr != _executes.end()) {
		OASSERT(itr->second.timer, "wtf");
		if (itr->second.timer) {
			itr->second.timer->Release();
			itr->second.timer = nullptr;
		}
	}
}

void Starter::StartNode(IKernel * kernel, s32 nodeType, s32 nodeId) {
	OASSERT(_strategy, "miss start strategy");
	s32 slave = _strategy->ChooseNode(nodeType);
	OASSERT(slave != IStartStrategy::INVALID_NODE, "where is slave");

	IArgs<2, 64> args;
	args << nodeType << nodeId;
	args.Fix();

	_harbor->Send(node_type::SLAVE, slave, _protoStartNode, args.Out());
}
