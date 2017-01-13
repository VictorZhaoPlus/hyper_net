#ifndef __CELL_H__
#define __CELL_H__
#include "util.h"
#include "IScene.h"
#include "singleton.h"

class Cell : public IModule, public OHolder<Cell> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

private:
    IKernel * _kernel;
};

#endif //__SCENE_H__

