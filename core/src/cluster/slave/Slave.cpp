#include "Slave.h"
#include "IHarbor.h"
#include "OArgs.h"
#include "IProtocolMgr.h"
#include "XmlReader.h"
#include "NodeType.h"
#include "proto.h"
#ifdef WIN32
#include <process.h>
#else
#include <sys/wait.h>
#endif

#define MAX_CMD_ARGS_COUNT 256

#define EXECUTE_CMD_PORT "$port$"
#define EXECUTE_CMD_PORT_SIZE 6
#define EXECUTE_CMD_OUT_PORT "$out_port$"
#define EXECUTE_CMD_OUT_PORT_SIZE 10
#define EXECUTE_CMD_ID "$id$"
#define EXECUTE_CMD_ID_SIZE 4
#define EXECUTE_NAME "serverd"

bool Slave::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool Slave::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);

	if (_harbor->GetNodeType() == node_type::SLAVE) {
		FIND_MODULE(_protocolMgr, ProtocolMgr);

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
			if (ports[i].GetAttributeInt32("node") == _harbor->GetNodeId()) {
				_startPort = ports[i].GetAttributeInt32("start");
				_endPort = ports[i].GetAttributeInt32("end");
				find = true;
			}
		}
		OASSERT(find, "wtf");

		const olib::IXmlObject& outPorts = starter["out_port"];
		for (s32 i = 0; i < outPorts.Count(); ++i) {
			if (outPorts[i].GetAttributeInt32("node") == _harbor->GetNodeId()) {
				_startOutPort = outPorts[i].GetAttributeInt32("start");
				_endOutPort = outPorts[i].GetAttributeInt32("end");
				find = true;
			}
		}
		OASSERT(find, "wtf");

		std::unordered_map<std::string, std::string> defines;
		const olib::IXmlObject& defs = starter["define"];
		for (s32 i = 0; i < defs.Count(); ++i)
			defines[defs[i].GetAttributeString("name")] = defs[i].GetAttributeString("value");

		if (starter.IsExist("node")) {
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

				_executes[info.type] = info;
			}
		}

		RGS_HABOR_ARGS_HANDLER(_protocolMgr->GetId("proto_cluster", "start_node"), Slave::OpenNewNode);
	}
    return true;
}

bool Slave::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void Slave::OpenNewNode(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s32 newNodeType = args.GetDataInt32(0);
	s32 newNodeId = args.GetDataInt32(1);

	OASSERT(_executes.find(newNodeType) != _executes.end(), "unknown nodeType %d", newNodeType);

	s64 node = (((s64)newNodeType) << 32) | newNodeId;
	auto itr = _cmds.find(node);
	if (itr != _cmds.end())
		StartNode(kernel, itr->second.cmd);
	else
		StartNewNode(kernel, _executes[newNodeType].name, _executes[newNodeType].cmd, newNodeType, newNodeId);
}

void Slave::StartNode(IKernel * kernel, const char * cmd) {
	char process[MAX_CMD_LEN];
#ifdef WIN32
	SafeSprintf(process, sizeof(process), "%s/%s.exe", tools::GetAppPath(), EXECUTE_NAME);

	STARTUPINFO si = { sizeof(si) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = TRUE;
	si.lpTitle = (char*)cmd;

	PROCESS_INFORMATION pi;
	BOOL ret = CreateProcess(process, (char*)cmd, nullptr, nullptr, false, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi);
	OASSERT(ret, "create process failed");
	::CloseHandle(pi.hThread);
	::CloseHandle(pi.hProcess);
#else
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
#endif
}

void Slave::StartNewNode(IKernel * kernel, const char * name, const char * cmd, const s32 nodeType, const s32 nodeId) {
	std::string tmp(cmd);
	std::string::size_type pos = tmp.find(EXECUTE_CMD_PORT);
	while (pos != std::string::npos) {
		OASSERT(_startPort < _endPort, "port is not enough");

		char portStr[64];
		SafeSprintf(portStr, sizeof(portStr), "%d", _startPort++);
		tmp.replace(pos, EXECUTE_CMD_PORT_SIZE, portStr);
	
		pos = tmp.find(EXECUTE_CMD_PORT);
	}

	pos = tmp.find(EXECUTE_CMD_OUT_PORT);
	while (pos != std::string::npos) {
		OASSERT(_startOutPort < _endOutPort, "out port is not enough");

		char portStr[64];
		SafeSprintf(portStr, sizeof(portStr), "%d", _startOutPort++);
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
	SafeSprintf(_cmds[node].cmd, MAX_CMD_LEN, tmp.c_str());

	StartNode(kernel, _cmds[node].cmd);
}
