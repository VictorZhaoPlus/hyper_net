#include "Slave.h"
#include "IHarbor.h"
#include "OArgs.h"
#include "NodeProtocol.h"
#include "XmlReader.h"
#include <sys/wait.h>

#define EXECUTE_CMD_PORT "$port$"
#define EXECUTE_CMD_PORT_SIZE 6
#define EXECUTE_CMD_OUT_PORT "$out_port$"
#define EXECUTE_CMD_OUT_PORT_SIZE 10
#define EXECUTE_CMD_ID "$id$"
#define EXECUTE_CMD_ID_SIZE 4
#define EXECUTE_NAME "serverd"

Slave * Slave::s_self = nullptr;
IKernel * Slave::s_kernel = nullptr;
IHarbor * Slave::s_harbor = nullptr;

s32 Slave::s_startPort = 0;
s32 Slave::s_endPort = 0;
s32 Slave::s_startOutPort = 0;
s32 Slave::s_endOutPort = 0;
std::unordered_map<s32, Slave::Execute> Slave::s_executes;
std::unordered_map<s64, Slave::Node> Slave::s_cmds;

bool Slave::Initialize(IKernel * kernel) {
    s_self = this;
    s_kernel = kernel;

	olib::XmlReader conf;
	std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/server_conf.xml";
	if (!conf.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "wtf");
		return false;
	}

	const olib::IXmlObject& starter = conf.Root()["starter"][0];

	bool find = false;
	const olib::IXmlObject& ports = starter["port"];
	for (s32 i = 0; i < ports.Count(); ++i) {
		if (ports[i].GetAttributeInt32("node") == s_harbor->GetNodeId()) {
			s_startPort = ports[i].GetAttributeInt32("start");
			s_endPort = ports[i].GetAttributeInt32("end");
			find = true;
		}
	}
	OASSERT(find, "wtf");

	const olib::IXmlObject& outPorts = starter["out_port"];
	for (s32 i = 0; i < outPorts.Count(); ++i) {
		if (outPorts[i].GetAttributeInt32("node") == s_harbor->GetNodeId()) {
			s_startOutPort = outPorts[i].GetAttributeInt32("start");
			s_endOutPort = outPorts[i].GetAttributeInt32("end");
			find = true;
		}
	}
	OASSERT(find, "wtf");

	std::unordered_map<std::string, std::string> defines;
	const olib::IXmlObject& defs = starter["define"];
	for (s32 i = 0; i < defs.Count(); ++i)
		defines[defs[i].GetAttributeString("name")] = defs[i].GetAttributeString("value");

	const olib::IXmlObject& nodes = starter["node"];
	for (s32 i = 0; i < nodes.Count(); ++i) {
		Execute info;
		info.type = nodes[i].GetAttributeInt32("nodeType");
		SafeSprintf(info.name, sizeof(info.name), nodes[i].GetAttributeString("name"));

		std::string cmd = nodes[i].GetAttributeString("cmd");
		for (auto itr = defines.begin(); itr != defines.end(); ++itr) {
			std::string::size_type pos = cmd.find(itr->first);
			while (pos != std::string::npos) {
				cmd.replace(pos, itr->first.size(), itr->second.c_str());
				pos = cmd.find(itr->first);
			}
		}
		SafeSprintf(info.cmd, sizeof(info.cmd), cmd.c_str());

		s_executes[info.type] = info;
	}

    return true;
}

bool Slave::Launched(IKernel * kernel) {
	s_harbor = (IHarbor*)kernel->FindModule("Harbor");
	OASSERT(s_harbor, "where is harbor");

	REGPROTOCOL(node_proto::START_NODE, Slave::OpenNewNode);
    return true;
}

bool Slave::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void Slave::OpenNewNode(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s32 newNodeType = args.GetDataInt32(0);
	s32 newNodeId = args.GetDataInt32(1);

	OASSERT(s_executes.find(newNodeType) != s_executes.end(), "unknown nodeType %d", newNodeType);

	s64 node = (((s64)newNodeType) << 32) | newNodeId;
	auto itr = s_cmds.find(node);
	if (itr != s_cmds.end())
		StartNode(kernel, itr->second.cmd);
	else
		StartNewNode(kernel, s_executes[newNodeType].name, s_executes[newNodeType].cmd, newNodeType, newNodeId);
}

void Slave::StartNode(IKernel * kernel, const char * cmd) {
	char process[MAX_CMD_LEN];
	SafeSprintf(process, sizeof(process), "%s/%s", tools::GetAppPath(), EXECUTE_NAME);

	char args[MAX_CMD_LEN];
	SafeSprintf(args, sizeof(args), cmd);

	char * p[MAX_CMD_ARGS_COUNT];
	SafeMemset(p, sizeof(p), 0, sizeof(p));
	p[0] = EXECUTE_NAME;
	s32 idx = 1;
	char * checkPtr = args;
	char * innderPtr = nullptr;
	while ((p[idx] = strtok_r(checkPtr, " ", &innderPtr)) != nullptr) {
		++idx;
		checkPtr = nullptr;
	}

	pid_t pid;
	pid = fork();
	if (pid < 0) {
		OASSERT(false, "start process failed");
	}
	else if (pid == 0) {
		pid = fork();
		if (pid == 0)
			execv(process, p);
		else
			exit(0);
	}
	else {
		s32 status;
		waitpid(pid, &status, 0);
	}
}

void Slave::StartNewNode(IKernel * kernel, const char * name, const char * cmd, const s32 nodeType, const s32 nodeId) {
	std::string tmp(cmd);
	std::string::size_type pos = tmp.find(EXECUTE_CMD_PORT);
	while (pos != std::string::npos) {
		OASSERT(s_startPort < s_endPort, "port is not enough");

		char portStr[64];
		SafeSprintf(portStr, sizeof(portStr), "%d", s_startPort++);
		tmp.replace(pos, EXECUTE_CMD_PORT_SIZE, portStr);
	
		pos = tmp.find(EXECUTE_CMD_PORT);
	}

	pos = tmp.find(EXECUTE_CMD_OUT_PORT);
	while (pos != std::string::npos) {
		OASSERT(s_startOutPort < s_endOutPort, "out port is not enough");

		char portStr[64];
		SafeSprintf(portStr, sizeof(portStr), "%d", s_startOutPort++);
		tmp.replace(pos, EXECUTE_CMD_OUT_PORT_SIZE, portStr);

		pos = tmp.find(EXECUTE_CMD_OUT_PORT);
	}

	pos = tmp.find(EXECUTE_CMD_ID);
	if (pos != std::string::npos) {
		char idStr[64];
		SafeSprintf(idStr, sizeof(idStr), "%d", nodeId);
		tmp.replace(pos, EXECUTE_CMD_ID_SIZE, idStr);
	}

	s64 node = (((s64)nodeType) << 32) | nodeId;
	SafeSprintf(s_cmds[node].cmd, MAX_CMD_LEN, tmp.c_str());

	StartNode(kernel, s_cmds[node].cmd);
}
