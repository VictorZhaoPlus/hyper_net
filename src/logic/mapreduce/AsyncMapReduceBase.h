#ifndef __ASYNCMAPREDUCEBASE_H__
#define __ASYNCMAPREDUCEBASE_H__
#include "IMapReduce.h"
#include <set>

class AsyncMapTask;
class AsyncMapReduceBase : public IAsyncMapReduceBase {
public:
	AsyncMapReduceBase(IAsyncMapReduceTask * task, const char * file, const s32 line);
	virtual ~AsyncMapReduceBase() {}

	void AddSubTask(AsyncMapTask * subTask) { _subTasks.insert(subTask); ++_totalTask; }
	void OnSubTaskComplete(IKernel * kernel, AsyncMapTask * subTask, IMapReduceResult * rst);

private:
	IAsyncMapReduceTask * _task;
	std::string _file;
	s32 _line;

	std::set<AsyncMapTask*> _subTasks;
	IMapReduceResult * _result;

	s32 _totalTask;
	s32 _successTask;
	s64 _startTick;
};

#endif //__ASYNCMAPREDUCEBASE_H__
