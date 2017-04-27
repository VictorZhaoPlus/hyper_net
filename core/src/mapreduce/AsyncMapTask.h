#ifndef __ASYNCMAPTASk_H__
#define __ASYNCMAPTASk_H__
#include "IMapReduce.h"

class AsyncMapReduceBase;
class AsyncMapTask : public IAsyncHandler {
public:
	AsyncMapTask(AsyncMapReduceBase * base, const std::function<IMapReduceResult * (IKernel * kernel)>& f) : _base(base), _f(f) {}
	virtual ~AsyncMapTask() {}

	virtual bool OnExecute(IKernel * kernel);
	virtual void OnSuccess(IKernel * kernel);
	virtual void OnFailed(IKernel * kernel, bool nonviolent) {}
	virtual void OnRelease(IKernel * kernel) { DEL this; }

private:
	AsyncMapReduceBase * _base;
	std::function<IMapReduceResult * (IKernel * kernel)> _f;

	IMapReduceResult * _rst;
};

#endif //__ASYNCMAPTASk_H__
