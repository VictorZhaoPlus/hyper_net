#ifndef __GAME_H__
#define __GAME_H__
#include "util.h"
#include "IGame.h"
#include "singleton.h"

class Game : public IGame, public OHolder<Game> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

private:
    IKernel * _kernel;
};

#endif //__GAME_H__

