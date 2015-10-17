#include "Logger.h"
#include "kernel.h"
#include "tools.h"
#include <mutex>
#include <time.h>
#include "ConfigMgr.h"

bool Logger::Ready() {
	return true;
}

bool Logger::Initialize() {
	_terminate = false;
	_thread = std::thread(&Logger::ThreadProc, this);

	memset(&_sync, 0, sizeof(_sync));
	memset(&_async, 0, sizeof(_async));

	const char * name = ConfigMgr::Instance()->GetCmdArg("name");
	const char * id = ConfigMgr::Instance()->GetCmdArg("node");
	const char * path = ConfigMgr::Instance()->GetLoggerPath();
	SafeSprintf(_path, sizeof(_path), "%s/%s_%s", path, name, id);
	return true;
}

void Logger::Destroy() {
	_terminate = true;
	_thread.join();
	DEL this;
}

void Logger::Log(const char * msg, bool sync) {
	if (sync)
		LogToFile(true, msg);
	else {
		LogUnit * unit = NEW LogUnit;
		SafeSprintf(unit->msg, sizeof(unit->msg), msg);

		std::unique_lock<spin_mutex> guard(_runningMutex);
		_waiting.push_back(unit);
	}

	if (ConfigMgr::Instance()->IsLogToConsole())
		printf("%s\n", msg);
}

void Logger::ThreadProc(){
	while (!_terminate) {
		if (_running.empty()) {
			std::unique_lock<spin_mutex> guard(_runningMutex);
			_running.swap(_waiting);
		}

		if (_running.empty())
			CSLEEP(1);
		else {
			for (auto * unit : _running) {
				LogToFile(false, unit->msg);
				DEL unit;
			}
			_running.clear();
		}
	}
}

void Logger::LogToFile(bool sync, const char * msg) {
	FileDesc * desc = (sync ? &_sync : &_async);

	CheckAndOpen(desc, sync);
	OASSERT(desc->fp, "where is logger file");

	time_t now = time(NULL);
	tm nowTm = *localtime(&now);
	fprintf(desc->fp, "[%02d:%02d:%02d]%s\n", nowTm.tm_hour, nowTm.tm_min, nowTm.tm_sec, msg);
	fflush(desc->fp);
}

void Logger::CheckAndOpen(FileDesc * desc, bool sync) {
	bool recreate = false;
	if (desc->fp) {
		time_t now = time(NULL);
		tm nowTm = *localtime(&now);
		if (nowTm.tm_wday != desc->day)
			recreate = true;
	}
	else
		recreate = true;

	if (recreate) {
		if (desc->fp)
			fclose(desc->fp);
		
		time_t now = time(NULL);
		tm nowTm = *localtime(&now);
		char file[MAX_PATH];
		if (sync)
			SafeSprintf(file, sizeof(file), "%s_sync_%d_%d_%d.log", _path, nowTm.tm_year + 1900, nowTm.tm_mon + 1, nowTm.tm_mday);
		else
			SafeSprintf(file, sizeof(file), "%s_async_%d_%d_%d.log", _path, nowTm.tm_year + 1900, nowTm.tm_mon + 1, nowTm.tm_mday);
		desc->fp = fopen(file, "a");
	}
}
