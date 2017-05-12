#include "TestClient.h"

#define ONE_BATCH 1000

class TestCSession : public ISession, public ITimer {
public:
	TestCSession() : _index(0) {}
	virtual ~TestCSession() {}

	virtual void OnConnected(IKernel * kernel) { START_TIMER(this, 0, TIMER_BEAT_FOREVER, 200); }
	virtual s32 OnRecv(IKernel * kernel, const void * context, const s32 size) {
		if (size < sizeof(s16))
			return 0;

		s16 len = *(s16*)context;
		if (len > size)
			return 0;

		s64 tick = tools::GetTimeMillisecond();
		if (tick - *(s64*)((const char*)context + sizeof(s16)) > 100) {
			DBG_INFO("connection %x over 100ms:%lld[%d, %d]", (s64)this, tick - *(s64*)((const char*)context + sizeof(s16)), len, *(s32*)((const char*)context + sizeof(s16) + sizeof(s64)));
		}

		return len;
	}
	virtual void OnDisconnected(IKernel * kernel) { kernel->KillTimer(this); DEL this; }
	virtual void OnConnectFailed(IKernel * kernel) { DEL this; }

	virtual void OnStart(IKernel * kernel, s64 tick) {}
	virtual void OnTimer(IKernel * kernel, s32 beatCount, s64 tick) {
		s16 size = 0;
		const void * context = TestClient::Instance()->GetBuffer(size);
		*(s64*)((const char*)context + sizeof(s16)) = tools::GetTimeMillisecond();
		*(s32*)((const char*)context + sizeof(s16) + sizeof(s64)) = _index++;

		Send(context, size);
	}
	virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {}

private:
	s32 _index;
};

bool TestClient::Initialize(IKernel * kernel) {
    _kernel = kernel;

	s16 size[6] = { 16, 32, 64, 256, 512, 1024 };
	for (s32 i = 0; i < 1024; ++i) {
		_buf[i].size = size[rand() % 6];
		_buf[i].data = MALLOC(_buf[i].size);
		for (s32 j = 0; j < _buf[i].size; ++j) {
			*((char*)_buf[i].data + j) = (char)(rand() % 128);
		}
		*(s16*)_buf[i].data = _buf[i].size;
	}

    return true;
}

bool TestClient::Launched(IKernel * kernel) {
	_ip = kernel->GetCmdArg("ip");
	_port = tools::StringAsInt(kernel->GetCmdArg("port"));
	_count = tools::StringAsInt(kernel->GetCmdArg("count"));
	START_TIMER(this, 0, (_count / ONE_BATCH + 1), ONE_BATCH);

    return true;
}

bool TestClient::Destroy(IKernel * kernel) {
	for (s32 i = 0; i < 1024; ++i) {
		FREE(_buf[i].data);
	}
    DEL this;
    return true;
}

void TestClient::OnTimer(IKernel * kernel, s32 beatCount, s64 tick) {
	s32 count = _count > ONE_BATCH ? ONE_BATCH : _count;
	for (s32 i = 0; i < count; ++i)
		kernel->Connect(_ip.c_str(), _port, 8192, 8192, NEW TestCSession);
	_count -= count;
}
