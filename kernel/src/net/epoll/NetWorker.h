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
		
		NWET_CHANGING,
		NWET_CHANGED,
	};
	
	struct NetEvent {
		s8 type;
		Connection * connection;
		s64 tick;
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
	inline void DelCounter(Connection * connection) { --_count; }
	
	inline void PostSend(Connection * connection, s64 tick) { _runQueue.Push({NWET_SEND, connection, tick}); }
	inline void PostClosing(Connection * connection, s64 tick) { _runQueue.Push({NWET_CLOSING, connection, tick}); }
	inline void PostChange(Connection * connection, s64 tick) { _runQueue.Push({NWET_CHANGING, connection, tick}); }
	
	inline void PostRecv(Connection * connection, s64 tick) { 
		std::unique_lock<spin_mutex> guard(_lock);
		_waitCompleteQueue.push_back({NWET_RECV, connection, tick});
	}
	
	inline void PostError(Connection * connection, s64 tick) { 
		std::unique_lock<spin_mutex> guard(_lock);
		_waitCompleteQueue.push_back({NWET_ERROR, connection, tick});
	}
	
	inline void PostChanged(Connection * connection, s64 tick) { 
		std::unique_lock<spin_mutex> guard(_lock);
		_waitCompleteQueue.push_back({NWET_CHANGED, connection, tick});
	}
	
	inline void PostDone(Connection * connection, s64 tick) { 
		std::unique_lock<spin_mutex> guard(_lock);
		_waitCompleteQueue.push_back({NWET_DONE, connection, tick});
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
