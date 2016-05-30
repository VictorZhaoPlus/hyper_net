#include "AsyncThread.h"
#include "AsyncBase.h"
#include "tools.h"

#define ASYNC_QUEUE_SIZE 8196

AsyncThread::AsyncThread() 
	: _runningQueue(ASYNC_QUEUE_SIZE)
	, _compeleQueue(ASYNC_QUEUE_SIZE)
	, _terminate(false) {

}

void AsyncThread::Start() {
	_thread = std::thread(&AsyncThread::ThreadProc, this);
}

void AsyncThread::Terminate() {
	_terminate = true;
	_thread.join();
}

void AsyncThread::Add(AsyncBase * base) {
	_runningQueue.Push(base);
}

void AsyncThread::Loop(s64 overtime) {
	s64 tick = tools::GetTimeMillisecond();
	AsyncBase * base = nullptr;
	while (_compeleQueue.Read(base)) {
		base->OnComplete();
		base->Release();
		
		if (tools::GetTimeMillisecond() - tick >= overtime)
			break;
	}
}

void AsyncThread::ThreadProc() {
	while (!_terminate) {
		AsyncBase * base = nullptr;
		if (_runningQueue.Read(base)) {
			base->OnExecute();

			_compeleQueue.Push(base);
		}
		else {
			CSLEEP(1);
		}
	}
}
