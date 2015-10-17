#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "util.h"
#include "singleton.h"
#include <thread>
#include "spin_mutex.h"
#include <list>

#define MAX_LOG_SIZE 1024

struct LogUnit {
	char msg[MAX_LOG_SIZE];
};

class IModule;
class Logger : public OSingleton<Logger> {
	friend class OSingleton<Logger>;

	struct FileDesc {
		FILE * fp;
		s32 day;
	};

public:
	bool Ready();
	bool Initialize();
	void Loop();
	void Destroy();

	void Log(const char * msg, bool sync);

	void ThreadProc();

private:
	Logger() {}
	~Logger() {}

	void LogToFile(bool sync, const char * msg);
	void CheckAndOpen(FileDesc * desc, bool sync);

private:
	std::thread _thread;
	spin_mutex _runningMutex;
	bool _terminate;

	std::list<LogUnit*> _waiting;
	std::list<LogUnit*> _running;

	FileDesc _sync;
	FileDesc _async;

	char _path[MAX_PATH];
};

#endif //__LOGGER_H__
