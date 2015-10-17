#ifndef __IKERNEL_H__
#define __IKERNEL_H__
#include "util.h"

class IModule;
namespace core {
    class IKernel;

	class IHttpBase {
	public:
		virtual ~IHttpBase() {}
	};

	class IHttpHandler {
	public:
		virtual ~IHttpHandler() {}

		inline void SetBase(IHttpBase * base) { _base = base; }
		inline IHttpBase * GetBase() { return _base; }

		virtual void OnSuccess(IKernel * kernel, const void * context, const s32 size) = 0;
		virtual void OnFail(IKernel * kernel, const s32 errCode) = 0;
		virtual void OnRelease() = 0;

	private:
		IHttpBase * _base;
	};

	class ITimerBase {
	public:
		virtual ~ITimerBase() {}
	};

	class ITimer {
	public:
		ITimer() : _base(nullptr) {}
		virtual ~ITimer() {}

		inline void SetBase(ITimerBase * base) { _base = base; }
		inline ITimerBase * GetBase() { return _base; }

		virtual void OnStart(IKernel * kernel, s64 tick) = 0;
		virtual void OnTimer(IKernel * kernel, s64 tick) = 0;
		virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick) = 0;

		virtual void OnPause(IKernel * kernel, s64 tick) = 0;
		virtual void OnResume(IKernel * kernel, s64 tick) = 0;

	private:
		ITimerBase * _base;
	};

    class IPipe {
    public:
        virtual ~IPipe() {}

        virtual void Send(const void * context, const s32 size) = 0;
        virtual void Close() = 0;

        virtual const char * GetLocalIp() const = 0;
        virtual s32 GetLocalPort() const = 0;

        virtual const char * GetRemoteIp() const = 0;
        virtual s32 GetRemotePort() const = 0;
    };

    class ISession;
    class ISessionFactory {
    public:
        virtual ~ISessionFactory() {}

        virtual ISession * Create() = 0;
        virtual void Recover(ISession *) = 0;
    };

    class ISession {
    public:
        virtual ~ISession() {}

        virtual void OnConnected(IKernel * kernel) = 0;
#define ON_RECV_FAILED -1
        virtual s32 OnRecv(IKernel * kernel, const void * context, const s32 size) = 0;
        virtual void OnError(IKernel * kernel, const s32 error) = 0;
        virtual void OnDisconnected(IKernel * kernel) = 0;
        virtual void OnConnectFailed(IKernel * kernel) = 0;

        void OnRelease() {
            if (_factory)
                _factory->Recover(this);
        }

        inline void Send(const void * context, const s32 size) {
            if (_pipe)
                _pipe->Send(context, size);
        }

        inline void Close() {
            if (_pipe)
                _pipe->Close();
        }

        inline const char * GetLocalIp() const { return _pipe ? _pipe->GetLocalIp() : nullptr; }
        inline s32 GetLocalPort() const { return _pipe ? _pipe->GetLocalPort() : 0; }

        inline const char * GetRemoteIp() const { return _pipe ? _pipe->GetRemoteIp() : nullptr; }
        inline s32 GetRemotePort() const { return _pipe ? _pipe->GetRemotePort() : 0; }

        void SetPipe(IPipe * pipe) { _pipe = pipe; }
        void SetFactory(ISessionFactory * factory) { _factory = factory; }

    private:
        IPipe * _pipe;
        ISessionFactory * _factory;
    };

    class IKernel {
    public:
        virtual ~IKernel() {}

        virtual bool Listen(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, ISessionFactory * factory) = 0;
        virtual bool Connect(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, ISession * session) = 0;

#define TIMER_BEAT_FOREVER -1
		virtual void StartTimer(ITimer * timer, s64 delay, s32 count, s64 interval) = 0;
		virtual void KillTimer(ITimer * timer) = 0;
		virtual void PauseTimer(ITimer * timer) = 0;
		virtual void ResumeTimer(ITimer * timer) = 0;

		virtual void Get(const s64 threadId, IHttpHandler * handler, const char * uri) = 0;
		virtual void Post(const s64 threadId, IHttpHandler * handler, const char * url, const char * field) = 0;
		virtual void Stop(IHttpHandler * handler) = 0;

        virtual IModule * FindModule(const char * name) = 0;

        virtual const char * GetCmdArg(const char * key) = 0;

		virtual void Log(const char * msg, bool sync = false) = 0;
    };
}

#define DBG_INFO(format, a...) { \
    char debug[4096] = {0}; \
    SafeSprintf(debug, sizeof(debug), format, ##a); \
    printf("%s\n", debug); \
}

#endif // __IKERNEL_H__
