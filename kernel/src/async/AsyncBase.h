#ifndef __ASYNCBASE_H__
#define __ASYNCBASE_H__
#include "util.h"
#include "IKernel.h"
using namespace core;

#define DEBUG_SIZE 256

class AsyncBase : public IAsyncBase {
public:
	AsyncBase(IAsyncHandler * handler, const char * file, s32 line) : _handler(handler) {
		SafeSprintf(_debug, sizeof(_debug), "%d:%s", line, file);
		handler->SetBase(this);

		_valid = true;
		_executed = false;
		_successed = false;
	}
	virtual ~AsyncBase() {}

	inline void SetInvalid() { _valid = false; }

	void OnExecute();
	void OnComplete();
	void Release();

private:
	char _debug[DEBUG_SIZE];
	IAsyncHandler * _handler;

	bool _valid;
	bool _executed;
	bool _successed;
};

#endif //__ASYNCBASE_H__
