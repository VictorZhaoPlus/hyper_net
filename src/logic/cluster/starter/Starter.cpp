#include "Starter.h"
#include "IHarbor.h"
#include "XmlReader.h"
#include "NodeType.h"
#include "ICapacitySubscriber.h"
#include "CoreProtocol.h"
#include "StartNodeTimer.h"

Starter * Starter::s_self = nullptr;
IKernel * Starter::s_kernel = nullptr;
IHarbor * Starter::s_harbor = nullptr;
IStartStrategy * Starter::s_strategy = nullptr;
ICapacitySubscriber * Starter::s_capacitySubscriber = nullptr;

s32 Starter::s_checkInterval = 0;
s32 Starter::s_deadInterval = 0;

std::unordered_map<s32, Starter::Execute> Starter::s_executes;
std::unordered_map<s32, Starter::NodeGroup> Starter::s_nodes;

bool Starter::Initialize(IKernel * kernel) {
    s_self = this;
    s_kernel = kernel;

	olib::XmlReader conf;
	std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/server_conf.xml";
	if (!conf.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "wtf");
		return false;
	}

	const olib::IXmlObject& starter = conf.Root()["starter"][0];
	s_checkInterval = starter.GetAttributeInt32("check");
	s_deadInterval = starter.GetAttributeInt32("dead");

	const olib::IXmlObject& nodes = starter["node"];
	for (s32 i = 0; i < nodes.Count(); ++i) {
		Execute info;
		info.type = nodes[i].GetAttributeInt32("nodeType");
		info.min = nodes[i].GetAttributeInt32("min");
		info.max = nodes[i].GetAttributeInt32("max");
		info.score = nodes[i].GetAttributeInt32("score");
		info.delay = nodes[i].GetAttributeInt32("delay");
		info.timer = nullptr;

		s_executes[info.type] = info;
	}

    return true;
}

bool Starter::Launched(IKernel * kernel) {
	s_harbor = (IHarbor*)kernel->FindModule("Harbor");
	OASSERT(s_harbor, "where is harbor");

	if (s_harbor->GetNodeType() == node_type::MASTER) {
		s_harbor->AddNodeListener(this, "Starter");

		s_capacitySubscriber = (ICapacitySubscriber *)kernel->FindModule("CapacitySubscriber");
		OASSERT(s_capacitySubscriber, "where is CapacitySubscriber");

		for (auto itr = s_executes.begin(); itr != s_executes.end(); ++itr) {
			itr->second.timer = StartNodeTimer::Create(itr->first);
			kernel->StartTimer(itr->second.timer, itr->second.delay, TIMER_BEAT_FOREVER, s_checkInterval);
		}
	}

    return true;
}

bool Starter::Destroy(IKernel * kernel) {
	for (auto itr = s_executes.begin(); itr != s_executes.end(); ++itr) {
		kernel->KillTimer(itr->second.timer);
	}

    DEL this;
    return true;
}

void Starter::OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port) {
	OASSERT(s_strategy, "miss start strategy");
	if (nodeType == node_type::SLAVE) 
		s_strategy->AddSlave(nodeId);
	else {
		if (s_executes.find(nodeType) != s_executes.end()) {
			s_nodes[nodeType].nodes[nodeId].online = true;
			s_nodes[nodeType].nodes[nodeId].closeTick = 0;
		}
	}
}

void Starter::OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) {
	if (nodeType != node_type::SLAVE) {
		s_nodes[nodeType].nodes[nodeId].online = false;
		s_nodes[nodeType].nodes[nodeId].closeTick = tools::GetTimeMillisecond();
	}
}

void Starter::OnNodeTimerStart(IKernel * kernel, s32 type, s64 tick) {
	auto itr = s_executes.find(type);
	OASSERT(itr != s_executes.end(), "wtf");

	if (itr != s_executes.end()) {
		for (s32 i = 1; i <= itr->second.min; ++i)
			StartNode(kernel, itr->second.type, i);
		s_nodes[itr->first].max = itr->second.min;
	}
}

void Starter::OnNodeTimer(IKernel * kernel, s32 type, s64 tick) {
	auto itr = s_nodes.find(type);
	OASSERT(itr != s_nodes.end(), "wtf");

	if (itr != s_nodes.end()) {
		for (auto itrServer = itr->second.nodes.begin(); itrServer != itr->second.nodes.end(); ++itrServer) {
			if (!itrServer->second.online && tick > itrServer->second.closeTick + s_deadInterval)
				StartNode(kernel, itr->first, itrServer->first);
		}

		if (s_executes[itr->first].max > itr->second.max && s_capacitySubscriber->CheckOverLoad(itr->first, s_executes[itr->first].rate * itr->second.max)) {
			StartNode(kernel, itr->first, itr->second.max + 1);
			++itr->second.max;
		}
	}
}

void Starter::OnNodeTimerEnd(IKernel * kernel, s32 type, s64 tick) {
	auto itr = s_executes.find(type);
	OASSERT(itr != s_executes.end(), "wtf");

	if (itr != s_executes.end()) {
		OASSERT(itr->second.timer, "wtf");
		if (itr->second.timer) {
			itr->second.timer->Release();
			itr->second.timer = nullptr;
		}
	}
}

void Starter::StartNode(IKernel * kernel, s32 nodeType, s32 nodeId) {
	OASSERT(s_strategy, "miss start strategy");
	s32 slave = s_strategy->ChooseNode(nodeType);
	OASSERT(slave != INVALID_NODE_ID, "where is slave");

	IArgs<2, 64> args;
	args << nodeType << nodeId;
	args.Fix();

	s_harbor->Send(node_type::SLAVE, slave, core_proto::START_NODE, args.Out());
}
