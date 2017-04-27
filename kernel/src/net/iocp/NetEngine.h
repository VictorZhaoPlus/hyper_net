#ifndef __NETENGINE_H__
#define __NETENGINE_H__
#include "util.h"
#include "singleton.h"
#include <thread>
#include "CycleQueue.h"

namespace core {
    class ISessionFactory;
    class ISession;
}

struct IocpEvent;
class Connection;
class NetEngine : public OSingleton<NetEngine> {
    friend class OSingleton<NetEngine>;

	enum {
		CONNECTED,
		CONNECTFAILED,
		RECV,
		RECV_BREAK,
		SEND_DONE,
		SEND_BREAK,
	};

	struct NetEvent {
		s32 type;
		Connection * connection;
	};

	struct WorkerThread {
		std::thread handler;
		bool terminated;
		HANDLE completionPort;

		olib::CycleQueue<NetEvent> events;

		WorkerThread() : events(8192) {}
	};
public:
    bool Ready();
    bool Initialize();
    s32 Loop(s64 overtime);
    void Destroy();

    bool Listen(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, core::ISessionFactory * factory);
    bool Connect(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, core::ISession * session);

	void ProcessAccept();
	void ProcessThreadEvent(olib::CycleQueue<NetEvent> * events, s64 overtime);
	static void ThreadLoop(WorkerThread * t);

	static IocpEvent * GetQueueState(HANDLE completionPort);

	void Accept(IocpEvent * evt);
	bool DoAccept(IocpEvent * evt);

	static void Connect(WorkerThread * t, IocpEvent * evt);
	static void Recv(WorkerThread * t, IocpEvent * evt);
	static void Send(WorkerThread * t, IocpEvent * evt);

	HANDLE GetCompletionPort() const { return _completionPortRS; }

private:
    NetEngine();
    virtual ~NetEngine();

	HANDLE _completionPortAC; //Accept
	HANDLE _completionPortRS; //Recv & Send & Connect
	WorkerThread * _threads;
};

#endif // __NETENGINE_H__
