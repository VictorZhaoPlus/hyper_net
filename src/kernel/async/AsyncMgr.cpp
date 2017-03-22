#include "AsyncMgr.h"
#include "XmlReader.h"
#include "AsyncBase.h"
#include "kernel.h"
#include "ConfigMgr.h"
#include "AsyncThread.h"

bool AsyncMgr::Ready() {
    return true;
}

bool AsyncMgr::Initialize() {
	s32 threadCount = ConfigMgr::Instance()->GetAsyncThreadCount();
	if (threadCount > 0) {
		for (s32 i = 0; i < threadCount; ++i) {
			AsyncThread * t = NEW AsyncThread;
			t->Start();
			_threads.push_back(t);
		}
	}

    return true;
}

bool AsyncMgr::Destroy() {
	for (AsyncThread * t : _threads) {
		t->Terminate();
		DEL t;
	}
	_threads.clear();

    DEL this;
    return true;
}

void AsyncMgr::Loop(s64 overTime) {
	for (AsyncThread * t : _threads)
		t->Loop(overTime / _threads.size());
}

void AsyncMgr::Start(const s64 threadId, IAsyncHandler * handler, const char * debug) {
	OASSERT(!handler->GetBase(), "wtf");
	OASSERT(_threads.size() > 0, "wtf");
	AsyncBase * base = NEW AsyncBase(handler, debug);
	_threads[threadId % _threads.size()]->Add(base);
}

void AsyncMgr::Stop(IAsyncHandler * handler) {
	OASSERT(handler->GetBase(), "wtf");
	((AsyncBase*)handler->GetBase())->SetInvalid();
}
