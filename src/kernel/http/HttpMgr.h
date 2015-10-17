#ifndef __HTTP_H__
#define __HTTP_H__
#include "util.h"
#include "ThreadModule.h"
#include "singleton.h"
#include "IKernel.h"
using namespace core;

#define MAX_URL_SIZE 256
#define MAX_PARAM_SIZE 1024

class HttpBase;
class HttpMgr : public olib::ThreadModule<HttpBase>, public olib::IRunnerFactory<HttpBase>, public OSingleton<HttpMgr>{
	friend class OSingleton<HttpMgr>;
public:
    bool Ready();
    bool Initialize();
    bool Destroy();
	void Loop();

	void Get(const s64 threadId, IHttpHandler * handler, const char * uri);
	void Post(const s64 threadId, IHttpHandler * handler, const char * url, const char * field);
	void Stop(IHttpHandler * handler);

	virtual void Complete(HttpBase * command);

	virtual olib::IRunner<HttpBase> * Create();
};

#endif //__HTTP_H__

