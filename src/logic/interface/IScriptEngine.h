#ifndef __ISCRIPTENGINE_H__
#define __ISCRIPTENGINE_H__

#include "IKernel.h"
#include "IModule.h"

class IScriptArgumentReader {
public:
	virtual ~IScriptArgumentReader() {}

	virtual bool Read(const char * format, ...) const = 0;
};

class IScriptResultWriter {
public:
	virtual ~IScriptResultWriter() {}

	virtual void Write(const char * format, ...) = 0;
};

typedef std::function<void(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer)> ScriptFuncType;

class IScriptCallResult {
public:
	virtual ~IScriptCallResult() {}

	virtual bool Read(const char * format, ...) const = 0;
};

typedef std::function<void(IKernel * kernel, IScriptCallResult *)> ScriptResultReadFuncType;
class IScriptModule {
public:
	virtual ~IScriptModule() {}

	virtual void Release() = 0;
};

class IScriptEngine : public IModule {
public:
	virtual ~IScriptEngine() {}

	virtual IScriptModule * CreateModule(const char * name) = 0;
	virtual bool AddModuleFunction(const IScriptModule * module, const char * func, const ScriptFuncType& f) = 0;

	virtual bool Call(const IScriptModule * module, const char * func, const ScriptResultReadFuncType& f, const char * format, ...) = 0;
};

#define RGS_SCRIPT_FUNC(module, func, handler) _scriptEngine->AddModuleFunction(module, func, std::bind(&handler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))

#endif //__ISCRIPTENGINE_H__
