#ifndef __MYSQLBASE_H__
#define __MYSQLBASE_H__
#include "util.h"
#include "IKernel.h"
using namespace core;

class MysqlBase : public IMysqlResult {
public:
	MysqlBase(IMysqlHandler * handler) : _handler(handler), _errCode(0) {}
	virtual ~MysqlBase() {}

	inline void SetErrCode(const s32 errCode) { _errCode = errCode; }

	void Complete(IKernel * kernel);

	virtual void Release();

private:
	char * _sql;
	IMysqlHandler * _handler;

	s32 _errCode;
};

#endif //__MYSQLBASE_H__
