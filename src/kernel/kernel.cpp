#include "kernel.h"
#include "NetEngine.h"
#include "ConfigMgr.h"
#include "LogicMgr.h"
#include "tools.h"
#include "Logger.h"
#include "TimerMgr.h"
#include "AsyncMgr.h"
#include "Profile.h"

bool Kernel::Ready() {
    if (ConfigMgr::Instance() == nullptr)
        return false;
	if (Logger::Instance() == nullptr)
		return false;
    if (NetEngine::Instance() == nullptr)
        return false;
	if (TimerMgr::Instance() == nullptr)
		return false;
	if (AsyncMgr::Instance() == nullptr)
		return false;
	if (Profile::Instance() == nullptr)
		return false;
    if (LogicMgr::Instance() == nullptr)
        return false;
    return true;
}

bool Kernel::Initialize(int argc, char ** argv) {
    if (!ConfigMgr::Instance()->Initialize(argc, argv)) {
        OASSERT(false, "load config failed");
        return false;
    }

	if (!Logger::Instance()->Initialize()) {
		OASSERT(false, "initialize logger failed");
		return false;
	}

    if (!NetEngine::Instance()->Initialize()) {
        OASSERT(false, "initialize net failed");
        return false;
    }

	if (!TimerMgr::Instance()->Initialize()) {
		OASSERT(false, "initialize timer failed");
		return false;
	}

	if (!AsyncMgr::Instance()->Initialize()) {
		OASSERT(false, "initialize async failed");
		return false;
	}

    if (!LogicMgr::Instance()->Initialize()) {
        OASSERT(false, "initialize net failed");
        return false;
    }

	if (!Profile::Instance()->Initialize()) {
		OASSERT(false, "initialize async failed");
		return false;
	}
    return true;
}

void Kernel::Loop() {
    while (!_terminate) {
        s64 tick = tools::GetTimeMillisecond();

        NetEngine::Instance()->Loop(ConfigMgr::Instance()->GetNetFrameTick());
        LogicMgr::Instance()->Loop();
		TimerMgr::Instance()->Loop();
		AsyncMgr::Instance()->Loop(ConfigMgr::Instance()->GetAsyncTick());
		Profile::Instance()->Loop();

        s64 use = tools::GetTimeMillisecond() - tick;
        if (use > ConfigMgr::Instance()->GetFrameDuration()) {

        }
    }
}

void Kernel::Destroy() {
    LogicMgr::Instance()->Destroy();
	AsyncMgr::Instance()->Destroy();
	TimerMgr::Instance()->Destroy();
    NetEngine::Instance()->Destroy();
	Logger::Instance()->Destroy();
	Profile::Instance()->Destroy();
    ConfigMgr::Instance()->Destroy();
    DEL this;
}

bool Kernel::Listen(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, ISessionFactory * factory) {
    return NetEngine::Instance()->Listen(ip, port, sendSize, recvSize, factory);
}

bool Kernel::Connect(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, ISession * session) {
    return NetEngine::Instance()->Connect(ip, port, sendSize, recvSize, session);
}

void Kernel::StartTimer(ITimer * timer, s64 delay, s32 count, s64 interval) {
	TimerMgr::Instance()->StartTimer(timer, delay, count, interval);
}

void Kernel::KillTimer(ITimer * timer) {
	TimerMgr::Instance()->KillTimer(timer);
}

void Kernel::PauseTimer(ITimer * timer) {
	TimerMgr::Instance()->PauseTimer(timer);
}

void Kernel::ResumeTimer(ITimer * timer) {
	TimerMgr::Instance()->ResumeTimer(timer);
}

void Kernel::StartAsync(const s64 threadId, IAsyncHandler * handler, const char * debug) {
	AsyncMgr::Instance()->Start(threadId, handler, debug);
}

void Kernel::StopAsync(IAsyncHandler * handler) {
	AsyncMgr::Instance()->Stop(handler);
}

IModule * Kernel::FindModule(const char * name) {
    return LogicMgr::Instance()->FindModule(name);
}

const char * Kernel::GetCmdArg(const char * key) {
    return ConfigMgr::Instance()->GetCmdArg(key);
}

void Kernel::Log(const char * msg, bool sync) {
	return Logger::Instance()->Log(msg, sync);
}

