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
		virtual void OnTimer(IKernel * kernel, s32 beatCount, s64 tick) = 0;
		virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick) = 0;

		virtual void OnPause(IKernel * kernel, s64 tick) {}
		virtual void OnResume(IKernel * kernel, s64 tick) {}

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

		virtual s32 GetLocalPort() const = 0;

		virtual void AdjustSendBuffSize(u32 size) = 0;
		virtual void AdjustRecvBuffSize(u32 size) = 0;
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
		ISession() : _pipe(nullptr), _factory(nullptr) {}
        virtual ~ISession() {}

        virtual void OnConnected(IKernel * kernel) = 0;
#define ON_RECV_FAILED -1
        virtual s32 OnRecv(IKernel * kernel, const void * context, const s32 size) = 0;
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

		inline s32 GetLocalPort() const { return _pipe ? _pipe->GetLocalPort() : 0; }

		inline void AdjustSendBuffSize(u32 size) {
			if (_pipe)
				_pipe->AdjustSendBuffSize(size);
		}

		inline void AdjustRecvBuffSize(u32 size) {
			if (_pipe)
				_pipe->AdjustRecvBuffSize(size);
		}

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
		virtual void StartTimer(ITimer * timer, s64 delay, s32 count, s64 interval, const char * file, const s32 line) = 0;
		virtual void KillTimer(ITimer * timer) = 0;
		virtual void PauseTimer(ITimer * timer) = 0;
		virtual void ResumeTimer(ITimer * timer) = 0;

		virtual void StartAsync(const s64 threadId, IAsyncHandler * handler, const char * file, const s32 line) = 0;
		virtual void StopAsync(IAsyncHandler * handler) = 0;

		virtual s32 GetId(const char * group, const char * name) = 0;

        virtual IModule * FindModule(const char * name) = 0;

        virtual const char * GetCmdArg(const char * key) = 0;
    };
}

KERNEL_API core::IKernel * GetCore();

#define DBG_INFO(format, ...) { \
    char debug[4096] = {0}; \
    SafeSprintf(debug, sizeof(debug), format, ##__VA_ARGS__); \
    printf("%s\n", debug); \
}

#define START_TIMER(timer, delay, count, interval) GetCore()->StartTimer(timer, delay, count, interval, __FILE__, __LINE__)
#define START_ASYNC(threadId, handler) GetCore()->StartAsync(threadId, handler, __FILE__, __LINE__)

template <typename T>
struct OModule {
	inline static T * Instance(const char * module) {
		static T * instance = nullptr;
		if (instance == nullptr) {
			instance = (T *)GetCore()->FindModule(module);
			OASSERT(instance, "where is %s", module);
		}
		return instance;
	}
};

#define OMODULE(name) (OModule<I##name>::Instance(#name))

template <s64, s64>
struct OId {
	inline static const s32 Get(const char * group, const char * name) {
		static const s32 ret = GetCore()->GetId(group, name);
		return ret;
	}
};

#define OID(group, name) (OId<CalcUniqueId(0, group), CalcUniqueId(0, name)>::Get(group, name))

#endif // __IKERNEL_H__
