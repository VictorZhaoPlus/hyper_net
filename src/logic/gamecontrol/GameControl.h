#ifndef __GAMECONTROL_H__
#define __GAMECONTROL_H__
#include "util.h"
#include "IGameControl.h"

class GameControl : public IGameControl {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	static GameControl * Self() { return s_self; }

private:
	static GameControl * s_self;
    static IKernel * s_kernel;
};

#endif //__GAMECONTROL_H__

