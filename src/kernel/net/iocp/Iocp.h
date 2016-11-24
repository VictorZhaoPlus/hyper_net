#ifndef __IOCP_H__
#define __IOCP_H__
#include "util.h"
#include <mswsock.h> 
#include "IKernel.h"

LPFN_ACCEPTEX GetAcceptExFunc();
LPFN_CONNECTEX GetConnectExFunc();

enum {
	IOCP_OPT_CONNECT = 0,
	IOCP_OPT_ACCEPT,
	IOCP_OPT_RECV,
	IOCP_OPT_SEND,
};

struct IocpEvent {
	OVERLAPPED ol;
	s8 opt;
	s32 code;
	WSABUF buf;
	SOCKET socket;
	DWORD bytes;
	void * context;
};

struct IocpAcceptor {
	IocpEvent accept;
	SOCKET socket;
	s32 sendSize;
	s32 recvSize;
	core::ISessionFactory * factory;
	char buf[256];
};

#endif //__IOCP_H__
