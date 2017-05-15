#ifndef __NETENGINE_H__
#define __NETENGINE_H__
#include "util.h"
#include "singleton.h"
#include <thread>
#include <list>
#include <vector>

namespace core {
    class ISessionFactory;
    class ISession;
}

class Connection;
class NetWorker;
class NetEngine : public OSingleton<NetEngine> {
    friend class OSingleton<NetEngine>;
	
	enum class ACDealerType {
		ACDT_ACCEPT,
		ACDT_CONNECT,
	};
	
	struct ACDealer {
		ACDealerType type;
		s32 fd;
		s32 sendSize;
		s32 recvSize;
		void * context;
	};
public:
    bool Ready();
    bool Initialize();
    s32 Loop(s64 overtime);
    void Destroy();

    bool Listen(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, core::ISessionFactory * factory);
    bool Connect(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, core::ISession * session);
	
	void Flush() {}
	
	void ProcessAC(s64 waitTime);
	void OnAccept(ACDealer * acceptor, s32 fd);
	void OnConnect(ACDealer * connecter, bool connectSuccess);
	
	bool AddToWorker(Connection * connection);
	
private:
    NetEngine();
    virtual ~NetEngine();

	s32 _acFd;
	s32 _acSize;

	std::vector<NetWorker*> _workers;
	bool _first;
};

#endif // __NETENGINE_H__
