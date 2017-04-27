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

class Connection;
class NetEngine : public OSingleton<NetEngine> {
    friend class OSingleton<NetEngine>;
public:
    bool Ready();
    bool Initialize();
    s32 Loop(s64 overtime);
    void Destroy();

    bool Listen(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, core::ISessionFactory * factory);
    bool Connect(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, core::ISession * session);
	
	void Flush();

	static s32 OnAccept(NetBase * accepter, struct NetBase * base);
	static s32 OnConnect(NetBase * connecter, const int code);

	static s32 OnRecv(NetBase * base, const s32 code, const char * buff, const s32 size);
	static s32 OnSend(NetBase * base);
	
	void InsertIntoChain(Connection * connection);
	void RemoveFromChain(Connection * connection);

	bool IsDirectSend() const { return _directSend; }
	
private:
    NetEngine();
    virtual ~NetEngine();

	NetLooper * _looper;
	Connection * _head;
	bool _directSend;
};

#endif // __NETENGINE_H__
