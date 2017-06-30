#ifndef __COMMAND_H__
#define __COMMAND_H__
#include "util.h"
#include "IModule.h"
#include "IHarbor.h"
#include "singleton.h"
#include <unordered_map>

class IScriptArgumentReader;
class IScriptResultWriter;
class IScriptModule;
class OBuffer;
class Command : public IModule, public OHolder<Command>{
	struct ResponeInfo {
		s32 nodeType;
		s32 nodeId;
		s64 sender;
		s64 reciver;
		s32 sequenceId;
	};
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	void Call(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer);
	void CallNoRet(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer);
	void Respone(IKernel * kernel, const IScriptArgumentReader * reader, IScriptResultWriter * writer);

	void OnCall(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer & stream);
	void OnCallNoRet(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer & stream);
	void OnRespone(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer & stream);
	void OnCallForward(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer & stream);

private:
    IKernel * _kernel;
	IScriptModule * _module;

	s32 _sequenceId;
	std::unordered_map<s64, ResponeInfo> _respones;
};

#endif //__COMMAND_H__

