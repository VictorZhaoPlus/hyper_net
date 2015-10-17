#ifndef __MYSQLMGR_H__
#define __MYSQLMGR_H__
#include "util.h"
#include "ThreadModule.h"
#include "singleton.h"
#include "IKernel.h"
using namespace core;

#define MAX_URL_SIZE 256
#define MAX_PARAM_SIZE 1024

class MysqlBase;
class MysqlMgr : public olib::ThreadModule<MysqlBase>, public olib::IRunnerFactory<MysqlBase>, public OSingleton<MysqlMgr>{
	friend class OSingleton<MysqlMgr>;
public:
    bool Ready();
    bool Initialize();
    bool Destroy();
	void Loop();

	void Execute(const s64 threadId, const char * mysql, IHttpHandler * handler);

	virtual void Complete(MysqlBase * command);

	virtual olib::IRunner<MysqlBase> * Create();
};

#endif //__MYSQLMGR_H__

