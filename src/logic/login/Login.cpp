#include "Login.h"
#include "UserNodeType.h"
#include "IProtocolMgr.h"
#include "OArgs.h"
#include "FrameworkProtocol.h"

#define TOKEN_LEN 512

bool Login::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool Login::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);
	if (_harbor->GetNodeType() == user_node_type::ACCOUNT) {
		FIND_MODULE(_protocolMgr, ProtocolMgr);

		_harbor->AddNodeListener(this, "Login");
		RGS_HABOR_ARGS_HANDLER(framework_proto::BIND_ACCOUNT_REQ, Login::OnRecvBindAccount);
		RGS_HABOR_ARGS_HANDLER(framework_proto::UNBIND_ACCOUNT, Login::OnRecvUnbindAccount);

		_noError = _protocolMgr->GetId("error", "no_error");
		_errorTokenCheckFailed = _protocolMgr->GetId("error", "token_check_failed");
		_errorAuthenFailed = _protocolMgr->GetId("error", "authen_failed");
	}

    return true;
}

bool Login::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void Login::OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) {
	if (nodeType == user_node_type::GATE) { 
		{
			for (auto accountId : _switchGateAccounts[nodeId]) {
				OASSERT(_accounts.find(accountId) == _accounts.end(), "wtf");
				Account& account = _accounts[accountId];
				OASSERT(account.state == ST_SWITCH, "wtf");

				account.switchGateId = 0;
				account.switchAgentId = 0;
				account.state = ST_ONLINE;
			}
			_switchGateAccounts[nodeId].clear();
		}

		{
			std::unordered_set<s64> tmp(_gateAccounts[nodeId]);
			for (auto accountId : tmp) {
				OASSERT(_accounts.find(accountId) == _accounts.end(), "wtf");
				Account& account = _accounts[accountId];
				OASSERT(account.state != ST_OFFLINE, "wtf");

				if (account.state == ST_ONLINE) {
					account.state = ST_OFFLINE;
					account.agentId = 0;
					account.gateId = 0;
					_gateAccounts[nodeId].erase(accountId);
				}
				else if (account.state == ST_SWITCH) {
					OASSERT(account.switchGateId != nodeId, "wtf");
					_gateAccounts[nodeId].erase(accountId);
					_switchGateAccounts[account.switchGateId].erase(accountId);
					account.agentId = account.switchAgentId;
					account.gateId = account.switchGateId;
					account.switchGateId = 0;
					account.switchAgentId = 0;
					account.state = ST_ONLINE;
					++account.tokenCount;
					if (account.tokenCount == 0)
						account.tokenCount = 1;
					_gateAccounts[account.gateId].insert(accountId);

					BindAccountSuccess(kernel, account);
				}
			}
		}
	}
}

void Login::OnRecvBindAccount(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s64 agentId = args.GetDataInt64(0);
	s64 accountId = args.GetDataInt64(1);
	s32 tokenCount = args.GetDataInt32(2);

	auto itr = _accounts.find(accountId);
	if (itr == _accounts.end())
		_accounts[accountId] = { accountId, ST_OFFLINE, 0, 0, 0, 0, 0 };

	Account& account = _accounts[accountId];
	if (tokenCount > 0 && account.tokenCount != tokenCount) {
		IArgs<3, 128> args;
		args << agentId << accountId << _errorTokenCheckFailed;
		args.Fix();
		_harbor->Send(nodeType, nodeId, framework_proto::BIND_ACCOUNT_ACK, args.Out());
		return;
	}

	switch (account.state) {
	case ST_OFFLINE: {
			account.gateId = nodeId;
			account.agentId = agentId;
			account.state = ST_ONLINE;
			++account.tokenCount;
			if (account.tokenCount == 0)
				account.tokenCount = 1;
			_gateAccounts[nodeId].insert(accountId);

			BindAccountSuccess(kernel, account);
		}
		break;
	case ST_ONLINE: {
			account.state = ST_SWITCH;
			account.switchGateId = nodeId;
			account.switchAgentId = agentId;
			_switchGateAccounts[nodeId].insert(accountId);

			IArgs<1, 128> args;
			args << account.agentId;
			args.Fix();
			_harbor->Send(user_node_type::GATE, account.gateId, framework_proto::KICK_FROM_ACCOUNT, args.Out());
		}
		break;
	case ST_SWITCH: {
			IArgs<3, 128> args;
			args << account.switchAgentId << accountId << _errorAuthenFailed;
			args.Fix();
			_harbor->Send(user_node_type::GATE, account.switchGateId, framework_proto::BIND_ACCOUNT_ACK, args.Out());

			_switchGateAccounts[account.switchGateId].erase(accountId);

			account.switchGateId = nodeId;
			account.switchAgentId = agentId;
			_switchGateAccounts[nodeId].insert(accountId);
		}
		break;
	}
}

void Login::OnRecvUnbindAccount(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	s64 agentId = args.GetDataInt64(0);
	s64 accountId = args.GetDataInt64(1);

	auto itr = _accounts.find(accountId);
	if (itr == _accounts.end())
		_accounts[accountId] = { accountId, ST_OFFLINE, 0, 0, 0, 0, 0 };

	Account& account = _accounts[accountId];
	switch (account.state) {
	case ST_ONLINE: {
			if (account.gateId == nodeId && account.agentId == agentId) {
				account.state = ST_OFFLINE;
				account.agentId = 0;
				account.gateId = 0;
				_gateAccounts[nodeId].erase(accountId);
			}
		}
		break;
	case ST_SWITCH: {
			if (account.gateId == nodeId && account.agentId == agentId) {
				_gateAccounts[nodeId].erase(accountId);
				_switchGateAccounts[account.switchGateId].erase(accountId);
				account.agentId = account.switchAgentId;
				account.gateId = account.switchGateId;
				account.switchGateId = 0;
				account.switchAgentId = 0;
				account.state = ST_ONLINE;
				++account.tokenCount;
				if (account.tokenCount == 0)
					account.tokenCount = 1;
				_gateAccounts[account.gateId].insert(accountId);

				BindAccountSuccess(kernel, account);
			}
			else if (account.switchGateId == nodeId && account.switchAgentId == agentId) {
				_switchGateAccounts[account.switchGateId].erase(accountId);
				account.switchGateId = 0;
				account.switchAgentId = 0;
				account.state = ST_ONLINE;
			}
		}
		break;
	}
}

void Login::BindAccountSuccess(IKernel * kernel, const Account& account) {
	IArgs<4, 1024> args;
	args << account.agentId << account.accountId << _noError << account.tokenCount;
	args.Fix();
	_harbor->Send(user_node_type::GATE, account.gateId, framework_proto::BIND_ACCOUNT_ACK, args.Out());
}
