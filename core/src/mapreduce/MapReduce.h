#ifndef __MAPREDUCE_H__
#define __MAPREDUCE_H__
#include "util.h"
#include "IMapReduce.h"
#include "singleton.h"
#include <set>

class AsyncMapReduceBase;
class MapReduce : public IMapReduce, public OHolder<MapReduce> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void StartAsyncTask(IAsyncMapReduceTask * task, const char * file, s32 line);
	virtual void StopAsyncTask(IAsyncMapReduceTask * task);
	void RemoveAsyncTask(AsyncMapReduceBase * base) { _runningAsyncTasks.erase(base); }

private:
    IKernel * _kernel;
	s32 _nextTaskId;

	std::set<AsyncMapReduceBase*> _runningAsyncTasks;
};

#endif //__MAPREDUCE_H__

