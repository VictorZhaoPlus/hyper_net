/*
 * File: IOCommand.h
 * Author: ooeyusea
 *
 * Created On January 13, 2017, 07:22 AM
 */

#ifndef __IOCOMMAND_H__
#define __IOCOMMAND_H__
 
#include "IModule.h"

class IObject;
class OArgs;

typedef std::function<void(IKernel * kernel, const s64 sender, IObject * reciever, const OArgs& args)> CommandCB;
typedef std::function<void(IKernel * kernel, const s32 cmd, const s64 sender, const s64 reciver)> CommandFailCB;
class IOCommand : public IModule {
public:
	virtual ~IOCommand() {}

	virtual void RegisterCmd(s32 cmd, const CommandCB& handler, const char * debug) = 0;
	virtual void Command(s32 cmd, const IObject * sender, const s64 reciever, const OArgs& args, const CommandFailCB& fail = nullptr) = 0;

	//can't use in logic
	virtual bool CommandTo(const s64 reciver, const s32 cmd, const OArgs & args) = 0;
};

#define RGS_COMMAND_CB(messageId, handler) _command->RegCommand(messageId, std::bind(&handler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), #handler)

#endif /*__IOCOMMAND_H__ */