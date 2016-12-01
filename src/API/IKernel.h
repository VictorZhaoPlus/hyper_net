#ifndef __IKERNEL_H__
#define __IKERNEL_H__
#include "util.h"

#define KERNEL_VERSION 20160609

class IModule;
namespace core {
    class IKernel;

	class IAsyncBase {
	public:
		virtual ~IAsyncBase() {}
	};

	class IAsyncHandler {
	public:
		virtual ~IAsyncHandler() {}

		inline void SetBase(IAsyncBase * base) { _base = base; }
		inline IAsyncBase * GetBase() { return _base; }

		virtual bool OnExecute(IKernel * kernel) = 0;
		virtual void OnSuccess(IKernel * kernel) = 0;
		virtual void OnFailed(IKernel * kernel, bool nonviolent) = 0;
		virtual void OnRelease(IKernel * kernel) = 0;

	private:
		IAsyncBase * _base;
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

        inline const char * GetRemoteIp() const { return _pipe ? _pipe->GetRemoteIp() : nullptr; }
        inline s32 GetRemotePort() const { return _pipe ? _pipe->GetRemotePort() : 0; }

		inline void SetPipe(IPipe * pipe) { _pipe = pipe; }
		inline IPipe * GetPipe() const { return _pipe; }
		inline void SetFactory(ISessionFactory * factory) { _factory = factory; }

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

		virtual void StartAsync(const s64 threadId, IAsyncHandler * handler, const char * debug) = 0;
		virtual void StopAsync(IAsyncHandler * handler) = 0;

        virtual IModule * FindModule(const char * name) = 0;

        virtual const char * GetCmdArg(const char * key) = 0;
    };
}

#define DBG_INFO(format, ...) { \
    char debug[4096] = {0}; \
    SafeSprintf(debug, sizeof(debug), format, __VA_ARGS__); \
    printf("%s\n", debug); \
}

#define START_TIMER(timer, delay, count, interval) {\
	kernel->StartTimer(timer, delay, count, interval);\
}

#define FIND_MODULE(m, name) {\
	m = (I##name *)kernel->FindModule(#name);\
	OASSERT(m, "where is #name");\
}

#endif // __IKERNEL_H__
