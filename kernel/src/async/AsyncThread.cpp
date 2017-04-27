#include "AsyncThread.h"
#include "AsyncBase.h"
#include "tools.h"
#include <mutex>

#define ASYNC_QUEUE_SIZE 8196

AsyncThread::AsyncThread() 
	: _compeleQueue(ASYNC_QUEUE_SIZE)
	, _terminate(false) {
	_waitQueue = { nullptr, nullptr };
	_readyQueue = { nullptr, nullptr };
	_runningQueue = { nullptr, nullptr };
}

void AsyncThread::Start() {
	_thread = std::thread(&AsyncThread::ThreadProc, this);
}

void AsyncThread::Terminate() {
	_terminate = true;
	_thread.join();
}

void AsyncThread::Add(AsyncBase * base) {
	AddChain(_waitQueue, base);
}

void AsyncThread::Loop(s64 overtime) {
	MergeChain(_readyQueue, _waitQueue);
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
		AsyncBase * base = RemoveFirst(_runningQueue);
		if (!base) {
			MergeChain(_runningQueue, _readyQueue);
			base = RemoveFirst(_runningQueue);
		}

		if (base) {
			base->OnExecute();

			_compeleQueue.Push(base);
		}
		else {
			CSLEEP(1);
		}
	}
}

void AsyncThread::AddChain(AsyncBaseLinkChain & chain, AsyncBase * base) {
	AsyncBaseLink * link = (AsyncBaseLink*)MALLOC(sizeof(AsyncBaseLink));
	link->base = base;
	link->next = nullptr;

	if (chain.head == nullptr) {
		OASSERT(chain.tail == nullptr, "wtf");

		chain.head = link;
		chain.tail = link;
	}
	else {
		OASSERT(chain.tail != nullptr, "wtf");

		chain.tail->next = link;
		chain.tail = link;
	}
}

void AsyncThread::MergeChain(AsyncBaseLinkChain & to, AsyncBaseLinkChain & from) {
	if (from.head == nullptr)
		return;
	else {
		OASSERT(from.tail != nullptr, "wtf");
		std::unique_lock<spin_mutex> guard(_lock);
		if (to.head == nullptr) {
			to.head = from.head;
			to.tail = from.tail;
		}
		else {
			to.tail->next = from.head;
			to.tail = from.tail;
		}
		from.head = from.tail = nullptr;
	}
}

AsyncBase * AsyncThread::RemoveFirst(AsyncBaseLinkChain & chain) {
	if (chain.head == nullptr)
		return nullptr;

	AsyncBaseLink * first = chain.head;
	chain.head = first->next;
	if (chain.head == nullptr)
		chain.tail = nullptr;

	AsyncBase * ret = first->base;
	FREE(first);
	return ret;
}
