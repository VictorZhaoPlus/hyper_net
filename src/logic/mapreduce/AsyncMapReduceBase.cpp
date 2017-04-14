#include "AsyncMapReduceBase.h"
#include "tools.h"
#include "MapReduce.h"

AsyncMapReduceBase::AsyncMapReduceBase(IAsyncMapReduceTask * task, const char * file, const s32 line)
	: _task(task), _file(file), _line(line), _result(nullptr), _totalTask(0), _successTask(0) {
	task->SetBase(this);
	_startTick = tools::GetTimeMillisecond();
}

void AsyncMapReduceBase::OnSubTaskComplete(IKernel * kernel, AsyncMapTask * subTask, IMapReduceResult * rst) {
	OASSERT(!_subTasks.empty(), "wtf");
	
	if (rst != nullptr) {
		if (_result == nullptr)
			_result = rst;
		else {
			_result->Merge(rst);
			rst->Release();
		}
		++_successTask;
	}

	_subTasks.erase(subTask);

	if (_subTasks.empty()) {
		_task->SetBase(nullptr);
		_task->OnComplete(kernel, _result);

		MapReduce::Instance()->RemoveAsyncTask(this);
		if (_result)
			_result->Release();
		DEL this;
	}
}
