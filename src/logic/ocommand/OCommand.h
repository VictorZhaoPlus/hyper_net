#ifndef __OCOMMAND_H__
#define __OCOMMAND_H__
#include "util.h"
#include "IOCommand.h"
#include "singleton.h"

class OCommand : public IOCommand, public OHolder<OCommand> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

private:
    IKernel * _kernel;
};

#endif //__OCOMMAND_H__

