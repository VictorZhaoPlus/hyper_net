#ifndef __kernel_h__
#define __kernel_h__
#include "util.h"
#include "singleton.h"
#include "IKernel.h"
using namespace core;

class Kernel : public OSingleton<Kernel>, public IKernel {
    friend class OSingleton<Kernel>;
public:

    bool Ready();
    bool Initialize(int argc, char ** argv);
    void Loop();
    void Destroy();

    virtual bool Listen(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, ISessionFactory * factory);
    virtual bool Connect(const char * ip, const s32 port, const s32 sendSize, const s32 recvSize, ISession * session);

	virtual void StartTimer(ITimer * timer, s64 delay, s32 count, s64 interval);
	virtual void KillTimer(ITimer * timer);
	virtual void PauseTimer(ITimer * timer);
	virtual void ResumeTimer(ITimer * timer);

	virtual void Get(const s64 threadId, IHttpHandler * handler, const char * uri);
	virtual void Post(const s64 threadId, IHttpHandler * handler, const char * url, const char * field);
	virtual void Stop(IHttpHandler * handler);

    virtual IModule * FindModule(const char * name);
    virtual const char * GetCmdArg(const char * key);

	virtual void Log(const char * msg, bool sync);

    void Terminate() { _terminate = true; }

private:
    Kernel() : _terminate(false) {}
    virtual ~Kernel() {}

private:
    bool _terminate;
};

#endif //__kernel_h__
