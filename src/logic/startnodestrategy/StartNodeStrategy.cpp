#include "StartNodeStrategy.h"
#include "Define.h"
#include "UserNodeType.h"
#include "XmlReader.h"

StartNodeStrategy * StartNodeStrategy::s_self = nullptr;
IKernel * StartNodeStrategy::s_kernel = nullptr;
IStarter * StartNodeStrategy::s_starter = nullptr;

std::unordered_map<s32, StartNodeStrategy::Score> StartNodeStrategy::s_scores;
std::unordered_map<s32, StartNodeStrategy::Score> StartNodeStrategy::s_slaves;

bool StartNodeStrategy::Initialize(IKernel * kernel) {
    s_self = this;
    s_kernel = kernel;

	olib::XmlReader conf;
	std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/server_conf.xml";
	if (!conf.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "wtf");
		return false;
	}

	const olib::IXmlObject& scores = conf.Root()["start_startegy"][0]["score"];
	for (s32 i = 0; i < scores.Count(); ++i) {
		Score info;
		info.value[BANDWIDTH] = scores[i].GetAttributeInt32("bandwidth");
		info.value[OVERLOAD] = scores[i].GetAttributeInt32("overload");

		s_scores[scores[i].GetAttributeInt32("nodeType")] = info;
	}

    return true;
}

bool StartNodeStrategy::Launched(IKernel * kernel) {
	s_starter = (IStarter *)kernel->FindModule("Starter");
	OASSERT(s_starter, "where is Starter");
	s_starter->SetStrategy(this);

    return true;
}

bool StartNodeStrategy::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

s32 StartNodeStrategy::ChooseNode(const s32 nodeType) {
	OASSERT(!s_slaves.empty(), "where is slave");

	if (s_slaves.empty())
		return INVALID_NODE_ID;

	if (nodeType == user_node_type::GATE)
		return Choose(s_kernel, BANDWIDTH, OVERLOAD);
	else
		return Choose(s_kernel, OVERLOAD, BANDWIDTH);
}

void StartNodeStrategy::AddSlave(const s32 nodeId) {
	if (s_slaves.find(nodeId) == s_slaves.end()) {
		SafeMemset(s_slaves[nodeId].value, sizeof(s_slaves[nodeId].value), 0, sizeof(s_slaves[nodeId].value));
	}
}

s32 StartNodeStrategy::Choose(IKernel * kernel, s32 first, s32 second) {
	OASSERT(first >= 0 && first < DATA_COUNT && second >= 0 && second < DATA_COUNT, "wtf");
	s32 node = INVALID_NODE_ID;
	s32 value[DATA_COUNT];
	SafeMemset(value, sizeof(value), -1, sizeof(value));

	for (auto itr = s_slaves.begin(); itr != s_slaves.end(); ++itr) {
		if (value[first] == -1 || value[first] > itr->second.value[first] 
			|| (value[first] == itr->second.value[first] && value[second] > itr->second.value[second])) {
			node = itr->first;
			value[first] = itr->second.value[first];
			value[second] = itr->second.value[second];
		}
	}

	if (node != INVALID_NODE_ID) {
		for (s32 i = 0; i < DATA_COUNT; ++i)
			s_slaves[node].value[i] += s_scores[node].value[i];
	}

	return node;
}
