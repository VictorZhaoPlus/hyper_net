#include "NetLoop.h"
#include <sys/resource.h>
#include "ORingBuffer.h"

#define BNEV_CONNECT 0x1
#define BNEV_ACCEPT 0x2
#define BNEV_IO 0x3

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	s32 SetNonBlocking(const s32 fd) {
		if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK) == -1) {
			printf("setnonblocking error %s\n", strerror(errno));
			return -1;
		}
		return 0;
	}
	
	s32 SetNonNegal(const s32 fd) {
		long val = 1l;
		return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&val, sizeof(val));
	}

	s32 SetSendBuf(const s32 fd, const s32 size) {
		return setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
	}

	s32 SetReuse(const s32 fd) {
		s32 val = 1;
		return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&val, sizeof(val));
	}

	s32 SetMaxOpenFile(const s32 size) {
		struct rlimit rt;
		rt.rlim_max = rt.rlim_cur = size;
		if (setrlimit(RLIMIT_NOFILE, &rt) == -1)
			return -1;
		return 0;
	}

	s32 SetStackSize(const s32 size) {
		struct rlimit rt;
		rt.rlim_max = rt.rlim_cur = size * 1024;
		if (setrlimit(RLIMIT_STACK, &rt) == -1)
			return -1;
		return 0;
	}

	struct NetLooper * MallocLooper(s32 size, FnAccept fnAccept, FnConnect fnConnect, FnRecv fnRecv, FnSend fnSend) {
		s32 fd;
		struct NetLooper * looper;
		OASSERT(fnRecv || fnSend || fnAccept || fnConnect, "must has a callback");

		if ((fd = epoll_create(size)) == -1) {
			printf("epoll create error, %s\n", strerror(errno));
			return NULL;
		}

		looper = (struct NetLooper *)MALLOC(sizeof(struct NetLooper));
		looper->fd = fd;
		looper->size = size;
		looper->fnAccept = fnAccept;
		looper->fnConnect = fnConnect;
		looper->fnRecv = fnRecv;
		looper->fnSend = fnSend;
		return looper;
	}

	void FreeLooper(struct NetLooper * looper) {
		close(looper->fd);
		FREE(looper);
	}

	struct NetBase * MallocAcceptor(const char * ip, const s32 port, const s32 backlog) {
		s32 fd;
		struct sockaddr_in addr;
		struct timeval tv;

		if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			printf("create listen socket error, %d\n", errno);
			return NULL;
		}

		if (0 != SetNonBlocking(fd) || 0 != SetReuse(fd)) {
			return NULL;
		}

		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		inet_pton(AF_INET, ip, &addr.sin_addr);

		if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
			printf("bind listen socket error, %d\n", errno);
			close(fd);
			return NULL;
		}

		if (listen(fd, backlog) == -1) {
			printf("listen socket error, %d\n", errno);
			close(fd);
			return NULL;
		}

		printf("listen socket create ok\n");
		struct NetBase * accepter = (struct NetBase *)MALLOC(sizeof(struct NetBase));
		accepter->type = BNEV_ACCEPT;
		accepter->fd = fd;
		accepter->looper = NULL;
		return accepter;
	}

	struct NetBase * MallocConnector(const char * ip, const int port) {
		s32 fd;
		struct timeval tv;
		struct sockaddr_in addr;
		struct NetBase * connecter;

		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
			printf("inet_pton error %s\n", ip);
			return NULL;
		}

		if (-1 == (fd = socket(AF_INET, SOCK_STREAM, 0))) {
			printf("socket create error %s\n", strerror(errno));
			return NULL;
		}

		if (0 != SetNonBlocking(fd) || 0 != SetReuse(fd) || 0 != SetNonNegal(fd)) {
			close(fd);
			return NULL;
		}

		s32 ret = connect(fd, (struct sockaddr *) &addr, sizeof(addr));
		if (ret < 0 && errno != EINPROGRESS) {
			printf("connect error %s\n", strerror(errno));
			close(fd);
			return NULL;
		}
		else {
			connecter = (struct NetBase *)MALLOC(sizeof(struct NetBase));
			connecter->fd = fd;
			connecter->type = BNEV_CONNECT;
			connecter->looper = NULL;
			return connecter;
		}
	}

	void FreeBase(struct NetBase * base) {
		struct linger _linger;
		_linger.l_onoff = 0;
		_linger.l_linger = 0;
		setsockopt(base->fd, SOL_SOCKET, SO_LINGER, (const char *)&_linger, sizeof(_linger));
		close(base->fd);
		FREE(base);
	}

	s32 BindLooper(struct NetLooper * looper, struct NetBase * base) {
		OASSERT(base->looper == NULL, "already has a looper");
		struct epoll_event ev;
		ev.data.ptr = base;
		switch (base->type) {
		case BNEV_ACCEPT: ev.events = EPOLLIN; break;
		case BNEV_CONNECT: ev.events = EPOLLOUT | EPOLLET; break;
		case BNEV_IO: ev.events = EPOLLIN | EPOLLOUT | EPOLLET; break;
		default:
			OASSERT(0, "wtf");
			return -1;
		}

		ev.events |= EPOLLERR | EPOLLHUP;

		if (epoll_ctl(looper->fd, EPOLL_CTL_ADD, base->fd, &ev) != 0) {
			printf("epoll add fd error %d\n", errno);
			return -1;
		}
		base->looper = looper;
		return 0;
	}

	s32 UnbindLooper(struct NetBase * base) {
		OASSERT(base->looper != NULL, "where is looper");
		if (base->looper != NULL) {
			if (epoll_ctl(base->looper->fd, EPOLL_CTL_DEL, base->fd, NULL) != 0) {
				printf("epoll add fd error %d\n", errno);
				return -1;
			}
			base->looper = NULL;
			return 0;
		}

		return -1;
	}

	s32 DispatchLooper(struct NetLooper * looper, const s32 millisecond) {
		struct epoll_event * events = (struct epoll_event *)alloca(sizeof(struct epoll_event) * looper->size);
		memset(events, 0, sizeof(struct epoll_event) * looper->size);
		s32 retCount = epoll_wait(looper->fd, events, looper->size, millisecond);
		if (retCount == -1) {
			if (errno != EINTR) {
				printf("epoll_wait error %d\n", errno);
			}
			return;
		}

		if (retCount == 0)
			return;

		s32 i = 0;
		for (i = 0; i < retCount; i++) {
			s32 code;
			s32 fd;
			struct sockaddr_in addr;
			socklen_t len = sizeof(addr);

			char type = ((struct NetBase *)events[i].data.ptr)->type;
			switch (type) {
			case BNEV_ACCEPT: {
				struct NetBase * accepter = events[i].data.ptr;
				if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
					code = -1;
					OASSERT(0, "wtf");
				} else if (events[i].events & EPOLLIN) {
					memset(&addr, 0, sizeof(addr));

					s32 i = 0;
					while (i++ < 30 && (fd = accept(accepter->fd, (struct sockaddr *)&addr, &len)) >= 0) {
						if (0 == SetNonBlocking(fd) && 0 == SetSendBuf(fd, 0) && 0 == SetNonNegal(fd)) {
							struct NetBase * base = (struct NetBase *)MALLOC(sizeof(struct NetBase));
							base->fd = fd;
							base->type = BNEV_IO;
							base->looper = NULL;
							base->maxRecvSize = accepter->maxRecvSize;
							base->maxSendSize = accepter->maxSendSize;
							if (0 != (*looper->fnAccept)(accepter, base)) {
								FreeBase(base);
							}
						}
						else {
							close(fd);
						}
					}
				}
				break;
			}
			case BNEV_CONNECT: {
				struct NetBase * connecter = events[i].data.ptr;
				if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
					code = -1;
				else if (events[i].events & EPOLLOUT)
					code = 0;

				if (UnbindLooper(connecter) != 0) {
					printf("epoll add fd error %s\n", strerror(errno));
					//return;
					OASSERT(0, "wtf");
				}
				connecter->type = BNEV_IO;

				(*looper->fnConnect)(connecter, code);

				if (-1 == code)
					FreeBase(connecter);
				break;
			}
			case BNEV_IO: {
				struct NetBase * base = events[i].data.ptr;
				if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
					UnbindLooper(base);
					(*looper->fnRecv)(base, -1, NULL, 0);
					FreeBase(base);
					break;
				}

				if (events[i].events & EPOLLIN) {
					s32 res = 0;
					while (1) {
						u32 size = 0;
						s32 len = 0;
						char * buf = RingBufferWrite(base->recvBuff, &size);
						if (buf && size > 0) {
							len = recv(base->fd, buf, size, 0);
							if (len < 0 && errno == EAGAIN)
								break;
						}
						else
							len = -1;

						if (len <= 0) {
							UnbindLooper(base);
							(*looper->fnRecv)(base, -1, NULL, 0);
							FreeBase(base);
							res = -1;
							break;
						}

						if (-1 == (*looper->fnRecv)(base, 0, buf, len)) {
							UnbindLooper(base);
							FreeBase(base);
							res = -1;
							break;
						}
					}

					if (-1 == res)
						break;
				}

				if (events[i].events & EPOLLOUT) {
					if (0 != (*looper->fnSend)(base)) {
						UnbindLooper(base);
						FreeBase(base);
					}
				}

				break;
			}
			default:
				OASSERT(0, "wtf");
				break;
			}
		}
	}

#ifdef __cplusplus
}
#endif //__cplusplus
