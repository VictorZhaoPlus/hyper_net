#ifndef __NETENGINE_H__
#define __NETENGINE_H__
#include "util.h"
#include "singleton.h"
#include "NetLoop.h"
#include <thread>
#include "CycleQueue.h"

namespace core {
    class ISessionFactory;
    class ISession;
}

class NetEngine : public OSingleton<NetEngine> {
    friend class OSingleton<NetEngine>;

	struct NetEvent {
		NetBase * base;
		s32 event;
	};

	struct ThreadEvent {
		NetBase * base;
		s32 event;
	};

	struct Thread {
		NetLooper * looper;
		std::thread handler;
		s32 count;
		bool terminated;

		olib::CycleQueue<ThreadEvent> threadEvents;
		olib::CycleQueue<NetEvent> events;

		Thread() : threadEvents(8192), events(8192) {}
	};

public:
    bool Ready();
    bool Initialize();
    s32 Loop(s64 overtime);
    void Destroy();

    bool Listen(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, core::ISessionFactory * factory);
    bool Connect(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, core::ISession * session);

	void ProcessingOneThread(Thread * thread, s64 overtime);

	static s32 OnAccept(NetBase * accepter, struct NetBase * base);
	static s32 OnConnect(NetBase * connecter, const int code);

	static s32 OnRecv(NetBase * base, const s32 code, const char * buff, const s32 size);

	static s32 DoSend(NetBase * base);
	static s32 OnSend(NetBase * base);
	static s32 StartSend(NetBase * base);

	static void ThreadLoop(Thread * t);

	static void AddEvent(s32 threadId, const NetEvent & evt);
	static void AddSendEvent(s32 threadId, NetBase * base);

	static s32 SelectThread();
	static void DecThreadConnectionCount(s32 threadId);

private:
    NetEngine();
    virtual ~NetEngine();

	NetLooper * _looper;
	static Thread * s_threads;
};

#endif // __NETENGINE_H__
