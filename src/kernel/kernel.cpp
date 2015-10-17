#include "kernel.h"
#include "NetEngine.h"
#include "ConfigMgr.h"
#include "LogicMgr.h"
#include "tools.h"
#include "Logger.h"
#include "TimerMgr.h"
#include "HttpMgr.h"

bool Kernel::Ready() {
    if (ConfigMgr::Instance() == nullptr)
        return false;
	if (Logger::Instance() == nullptr)
		return false;
    if (NetEngine::Instance() == nullptr)
        return false;
	if (TimerMgr::Instance() == nullptr)
		return false;
	if (HttpMgr::Instance() == nullptr)
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

	if (!HttpMgr::Instance()->Initialize()) {
		OASSERT(false, "initialize timer failed");
		return false;
	}

    if (!LogicMgr::Instance()->Initialize()) {
        OASSERT(false, "initialize net failed");
        return false;
    }
    return true;
}

void Kernel::Loop() {
    while (!_terminate) {
        s64 tick = tools::GetTimeMillisecond();

        NetEngine::Instance()->Loop();
        LogicMgr::Instance()->Loop();
		TimerMgr::Instance()->Loop();
		HttpMgr::Instance()->Loop();

        s64 use = tools::GetTimeMillisecond() - tick;
        if (use > ConfigMgr::Instance()->GetFrameDuration()) {

        }
    }
}

void Kernel::Destroy() {
    LogicMgr::Instance()->Destroy();
	HttpMgr::Instance()->Destroy();
	TimerMgr::Instance()->Destroy();
    NetEngine::Instance()->Destroy();
	Logger::Instance()->Destroy();
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

void Kernel::Get(const s64 threadId, IHttpHandler * handler, const char * uri) {
	HttpMgr::Instance()->Get(threadId, handler, uri);
}

void Kernel::Post(const s64 threadId, IHttpHandler * handler, const char * url, const char * field) {
	HttpMgr::Instance()->Post(threadId, handler, url, field);
}

void Kernel::Stop(IHttpHandler * handler) {
	HttpMgr::Instance()->Stop(handler);
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
