#include "Game.h"

bool Game::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool Game::Launched(IKernel * kernel) {
    return true;
}

bool Game::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

