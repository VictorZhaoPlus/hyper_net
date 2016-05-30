#ifndef __ASYNCMGR_H__
#define __ASYNCMGR_H__
#include "util.h"
#include "singleton.h"
#include "IKernel.h"
#include <vector>
using namespace core;

#define MAX_URL_SIZE 256
#define MAX_PARAM_SIZE 1024

class AsyncThread;
class AsyncMgr : public OSingleton<AsyncMgr>{
	friend class OSingleton<AsyncMgr>;
public:
    bool Ready();
    bool Initialize();
    bool Destroy();
	void Loop(s64 overTime);

	void Start(const s64 threadId, IAsyncHandler * handler, const char * debug);
	void Stop(IAsyncHandler * handler);

private:
	std::vector<AsyncThread*> _threads;
};

#endif //__ASYNCMGR_H__

