#ifndef __ASYNCTHREAD_H__
#define __ASYNCTHREAD_H__
#include "util.h"
#include "IKernel.h"
#include "spin_mutex.h"
#include "CycleQueue.h"
#include <thread>

using namespace core;

class AsyncBase;
class AsyncThread {
	struct AsyncBaseLink {
		AsyncBase * base;
		AsyncBaseLink * next;
	};

	struct AsyncBaseLinkChain {
		AsyncBaseLink * head;
		AsyncBaseLink * tail;
	};

public:
	AsyncThread();
	~AsyncThread() {}

	AsyncThread(const AsyncThread&) = delete;

	void Start();
	void Terminate();

	void Add(AsyncBase * base);

	void Loop(s64 overtime);
	void ThreadProc();

private:
	void AddChain(AsyncBaseLinkChain & chain, AsyncBase * base);
	void MergeChain(AsyncBaseLinkChain & to, AsyncBaseLinkChain & from);
	AsyncBase * RemoveFirst(AsyncBaseLinkChain & chain);

private:
	spin_mutex _lock;
	AsyncBaseLinkChain _waitQueue;
	AsyncBaseLinkChain _readyQueue;
	AsyncBaseLinkChain _runningQueue;
	olib::CycleQueue<AsyncBase*> _compeleQueue;

	bool _terminate;
	std::thread _thread;
};

#endif //__ASYNCTHREAD_H__
