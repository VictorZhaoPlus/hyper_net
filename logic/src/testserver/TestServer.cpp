#include "TestServer.h"
#include "tools.h"

bool TestServer::Initialize(IKernel * kernel) {
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

bool TestServer::Launched(IKernel * kernel) {
	s32 port = tools::StringAsInt(kernel->GetCmdArg("port"));
	kernel->Listen("0.0.0.0", port, 8192, 8192, this);

	START_TIMER(this, 0, TIMER_BEAT_FOREVER, 2000);
    return true;
}

bool TestServer::Destroy(IKernel * kernel) {
	for (s32 i = 0; i < 1024; ++i) {
		FREE(_buf[i].data);
	}
    DEL this;
    return true;
}

class TestSSession : public ISession {
public:
	TestSSession() {}
	virtual ~TestSSession() {}

	virtual void OnConnected(IKernel * kernel) {}
	virtual s32 OnRecv(IKernel * kernel, const void * context, const s32 size) {
		if (size < sizeof(s16))
			return 0;

		s16 len = *(s16*)context;
		if (len > size)
			return 0;

		Send(context, size);
		TestServer::Instance()->Active(this);
		return len;
	}
	virtual void OnDisconnected(IKernel * kernel) {}
	virtual void OnConnectFailed(IKernel * kernel) {}
};

ISession * TestServer::Create() {
	++_count;
	return NEW TestSSession;
}

void TestServer::Recover(ISession * session) {
	--_count;
	DEL session;
}

void TestServer::OnTimer(IKernel * kernel, s32 beatCount, s64 tick) {
	DBG_INFO("connection: %d active %d", _count, (s32)_actives.size());
	_actives.clear();
}
