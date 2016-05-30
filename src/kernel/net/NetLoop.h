#ifndef __NETLOOP_H__
#define __NETLOOP_H__
#include "util.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus 

	s32 SetMaxOpenFile(const s32 size);
	s32 SetStackSize(const s32 size);

	struct NetLooper;

	struct NetBase {
		char type;
		s32 fd;
		struct NetLooper * looper;
		void * context;
		s32 maxSendSize;
		s32 maxRecvSize;
	};

	typedef int(*FnRecv)(struct NetBase * base, const s32 code, const char * buff, const s32 size);
	typedef int(*FnSend)(struct NetBase * base);
	typedef int(*FnAccept)(struct NetBase * accepter, struct NetBase * base);
	typedef int(*FnConnect)(struct NetBase * connecter, const s32 code);

	struct NetLooper {
		s32 fd;
		s32 size;
		FnRecv fnRecv;
		FnSend fnSend;
		FnAccept fnAccept;
		FnConnect fnConnect;
	};

	struct NetLooper * MallocLooper(s32 size, FnAccept fnAccept, FnConnect fnConnect, FnRecv fnRecv, FnSend fnSend);
	void FreeLooper(struct NetLooper * looper);

	struct NetBase * MallocAcceptor(const char * ip, const int port, const int backlog);
	struct NetBase * MallocConnector(const char * ip, const int port);
	void FreeBase(struct NetBase * base);

	s32 BindLooper(struct NetLooper * looper, struct NetBase * base);
	s32 UnbindLooper(struct NetBase * base);

	s32 DispatchLooper(struct NetLooper * looper, const s32 millisecond);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__NETLOOP_H__
