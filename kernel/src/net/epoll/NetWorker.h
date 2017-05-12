#ifndef __NETWORKER_H__
#define __NETWORKER_H__
#include "IKernel.h"
#include <thread>
#include "CycleQueue.h"
#include "spin_mutex.h"
#include <mutex>
#include <list>

class Connection;
class NetWorker {
	enum {
		NWET_SEND,
		NWET_CLOSING,
		
		NWET_RECV,
		NWET_ERROR,
		NWET_DONE,
	};
	
	struct NetEvent {
		s8 type;
		Connection * connection;
	};
public:
	NetWorker();
	~NetWorker() {}
	
	bool Start();
	void Terminate();

	void Process(s64 overtime);
	
	void ThreadProc();
	void ProcessRS(s64 waitTime);
	
	bool Add(Connection * connection);
	void Remove(Connection * connection);
	
	inline void PostSend(Connection * connection) { _runQueue.Push({NWET_SEND, connection}); }
	inline void PostClosing(Connection * connection) { _runQueue.Push({NWET_CLOSING, connection}); }
	
	inline void PostRecv(Connection * connection) { 
		std::unique_lock<spin_mutex> guard(_lock);
		_waitCompleteQueue.push_back({NWET_RECV, connection});
	}
	
	inline void PostError(Connection * connection) { 
		std::unique_lock<spin_mutex> guard(_lock);
		_waitCompleteQueue.push_back({NWET_ERROR, connection});
	}
	
	inline void PostDone(Connection * connection) { 
		std::unique_lock<spin_mutex> guard(_lock);
		_waitCompleteQueue.push_back({NWET_DONE, connection});
	}

	inline s32 Count() const { return _count; }

private:
	s32 _fd;
	s32 _count;
	
	olib::CycleQueue<NetEvent> _runQueue;
	
	spin_mutex _lock;
	std::list<NetEvent> _waitCompleteQueue;
	std::list<NetEvent> _completeQueue;
	s32 _waitTime;
	
	bool _terminate;
	std::thread _thread;
};

#endif //__CONNECTION_H__
