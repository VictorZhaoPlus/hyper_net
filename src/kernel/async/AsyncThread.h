#ifndef __ASYNCTHREAD_H__
#define __ASYNCTHREAD_H__
#include "util.h"
#include "IKernel.h"
#include "CycleQueue.h"
#include <thread>

using namespace core;

class AsyncBase;
class AsyncThread {
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
	olib::CycleQueue<AsyncBase*> _runningQueue;
	olib::CycleQueue<AsyncBase*> _compeleQueue;

	bool _terminate;
	std::thread _thread;
};

#endif //__ASYNCTHREAD_H__
