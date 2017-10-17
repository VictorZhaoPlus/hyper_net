#include "Login.h"
#include "OArgs.h"

#define TOKEN_LEN 512

bool Login::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool Login::Launched(IKernel * kernel) {
	if (OMODULE(Harbor)->GetNodeType() == OID("node_type", "account")) {
		OMODULE(Harbor)->AddNodeListener(this, "Login");
		RGS_HABOR_ARGS_HANDLER(OID("login", "bind_account_req"), Login::OnRecvBindAccount);
		RGS_HABOR_ARGS_HANDLER(OID("login", "unbind_account_req"), Login::OnRecvUnbindAccount);
	}

    return true;
}

bool Login::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void Login::OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) {
	if (nodeType == OID("node_type", "gate")) { 
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
		args << agentId << accountId << OID("err_mmo", "token_check_failed");
		args.Fix();
		OMODULE(Harbor)->Send(nodeType, nodeId, OID("login", "bind_account_ack"), args.Out());
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
			OMODULE(Harbor)->Send(OID("node_type", "gate"), account.gateId, OID("login", "kick_from_account"), args.Out());
		}
		break;
	case ST_SWITCH: {
			IArgs<3, 128> args;
			args << account.switchAgentId << accountId << OID("err_mmo", "authen_failed");
			args.Fix();
			OMODULE(Harbor)->Send(OID("node_type", "gate"), account.switchGateId, OID("login", "bind_account_ack"), args.Out());

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
	args << account.agentId << account.accountId << OID("error", "no_error") << account.tokenCount;
	args.Fix();
	OMODULE(Harbor)->Send(OID("node_type", "gate"), account.gateId, OID("login", "bind_account_ack"), args.Out());
}
