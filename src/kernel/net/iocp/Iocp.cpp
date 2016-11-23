#include "Iocp.h"

LPFN_ACCEPTEX GetAcceptExFunc() {
	GUID acceptExGuild = WSAID_ACCEPTEX;
	DWORD bytes = 0;
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	LPFN_ACCEPTEX acceptFunc = nullptr;

	WSAIoctl(sock,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&acceptExGuild,
		sizeof(acceptExGuild),
		&acceptFunc,
		sizeof(acceptFunc),
		&bytes, nullptr, nullptr);

	if (nullptr == acceptFunc) {
		OASSERT(false, "Get AcceptEx fun error, error code : %d", WSAGetLastError());
	}

	return acceptFunc;
}

LPFN_CONNECTEX GetConnectExFunc() {
	GUID conectExFunc = WSAID_CONNECTEX;
	DWORD bytes = 0;
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	LPFN_CONNECTEX connectFunc = nullptr;

	WSAIoctl(sock,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&conectExFunc,
		sizeof(conectExFunc),
		&connectFunc,
		sizeof(connectFunc),
		&bytes, nullptr, nullptr);

	if (nullptr == connectFunc) {
		OASSERT(false, "Get ConnectEx fun error, error code : %d", WSAGetLastError());
	}

	return connectFunc;
}
