#include "MapReduce.h"
#include "AsyncMapReduceBase.h"
#include "AsyncMapTask.h"

bool MapReduce::Initialize(IKernel * kernel) {
    _kernel = kernel;

	_nextTaskId = 0;
    return true;
}

bool MapReduce::Launched(IKernel * kernel) {
    return true;
}

bool MapReduce::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void MapReduce::StartAsyncTask(IAsyncMapReduceTask * task, const char * file, s32 line) {
	IKernel * kernel = _kernel;
	OASSERT(!task->GetBase(), "wtf");

	AsyncMapReduceBase * base = NEW AsyncMapReduceBase(task, file, line);
	std::vector<std::function<IMapReduceResult * (IKernel * kernel)>> functions = task->Split(_kernel);
	for (const auto& f : functions) {
		AsyncMapTask * mapTask = NEW AsyncMapTask(base, f);
		base->AddSubTask(mapTask);
		START_ASYNC(_nextTaskId++, mapTask);
	}

	_runningAsyncTasks.insert(base);
}

void MapReduce::StopAsyncTask(IAsyncMapReduceTask * task) {
	OASSERT(task->GetBase(), "wtf");
}

