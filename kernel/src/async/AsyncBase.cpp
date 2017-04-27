#include "AsyncBase.h"
#include "kernel.h"

void AsyncBase::OnExecute() {
	if (_valid) {
		_successed = _handler->OnExecute(Kernel::Instance());
		_executed = true;
	}
}

void AsyncBase::OnComplete() {
	if (_successed)
		_handler->OnSuccess(Kernel::Instance());
	else
		_handler->OnFailed(Kernel::Instance(), _executed);
	
	_handler->OnRelease(Kernel::Instance());
}

void AsyncBase::Release() {
	DEL this;
}
