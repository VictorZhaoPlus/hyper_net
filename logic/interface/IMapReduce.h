/*
 * File: IMapReduce.h
 * Author: ooeyusea
 *
 * Created On April 13, 2017, 11:56 PM
 */

#ifndef __IMAPREDUCE_H__
#define __IMAPREDUCE_H__
 
#include "IModule.h"
#include <vector>

class IAsyncMapReduceBase {
public:
	virtual ~IAsyncMapReduceBase() {}
};

class IMapReduceResult {
public:
	virtual ~IMapReduceResult() {}

	virtual void Release() = 0;

	virtual const void * Context() const = 0;
	virtual void Merge(IMapReduceResult * rhs) = 0;
};

class IAsyncMapReduceTask {
public:
	virtual ~IAsyncMapReduceTask() {}

	inline void SetBase(IAsyncMapReduceBase * base) { _base = base; }
	inline IAsyncMapReduceBase * GetBase() { return _base; }

	virtual std::vector<std::function<IMapReduceResult * (IKernel * kernel)>> Split(IKernel * kernel) = 0;
	virtual void OnComplete(IKernel * kernel, IMapReduceResult * result) = 0;

private:
	IAsyncMapReduceBase * _base;
};

class IMapReduce : public IModule {
public:
	virtual ~IMapReduce() {}

	virtual void StartAsyncTask(IAsyncMapReduceTask * task, const char * file , s32 line) = 0;
	virtual void StopAsyncTask(IAsyncMapReduceTask * task) = 0;
};

#endif /*__IMAPREDUCE_H__ */