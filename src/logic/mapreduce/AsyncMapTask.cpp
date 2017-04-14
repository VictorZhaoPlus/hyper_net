#include "AsyncMapTask.h"
#include "AsyncMapReduceBase.h"

bool AsyncMapTask::OnExecute(IKernel * kernel) {
	_rst = _f(kernel);
	return true;
}

void AsyncMapTask::OnSuccess(IKernel * kernel) {
	_base->OnSubTaskComplete(kernel, this, _rst);
}
