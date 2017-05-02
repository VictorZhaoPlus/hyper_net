#ifndef __NETWORKER_H__
#define __NETWORKER_H__
#include "IKernel.h"
#include <thread>
#include "CycleQueue.h"
#include "spin_mutex.h"
#include <mutex>

class Connection;
class NetWorker {
	enum {
		NWET_SEND,
		NWET_CLOSING,
		
		NEWT_RECV,
		NEWT_ERROR,
		NEWT_DONE,
	};
	
	struct NetEvent {
		s8 type;
		Connection * connection;
	};
public:
	NetWorker();
	~NetWorker() {}
	
	void Start();
	void Terminate();

	void Process(s64 overtime);
	
	void ThreadProc();
	void ProcessRS(s64 waitTime);
	
	bool Add(Connection * connection);
	void Remove(Connection * connection);
	
	inline void PostSend(Connection * connection) {
		std::unique_lock<spin_mutex> guard(_lock);
		_waitCompleteQueue.push_back({NWET_SEND, connection});
	}
	
	inline void PostClosing(Connection * connection) {
		std::unique_lock<spin_mutex> guard(_lock);
		_waitCompleteQueue.push_back({NWET_CLOSING, connection});
	}
	
	inline void PostRecv(Connection * connection) { _runQueue.Push({NEWT_RECV, connection}); }
	inline void PostError(Connection * connection) { _runQueue.Push({NEWT_ERROR, connection}); }
	inline void PostDone(Connection * connection) { _runQueue.Push({NEWT_DONE, connection}); }

private:
	s32 _fd;
	s32 _count;
	
	olib::CycleQueue<NetEvent> _runQueue;
	
	spin_mutex _lock;
	std::list<NetEvent> _waitCompleteQueue;
	std::list<NetEvent> _completeQueue;
	
	bool _terminate;
	std::thread _thread;
};

#endif //__CONNECTION_H__
