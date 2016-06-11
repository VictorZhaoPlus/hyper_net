#include "GameControl.h"

GameControl * GameControl::s_self = nullptr;
IKernel * GameControl::s_kernel = nullptr;

bool GameControl::Initialize(IKernel * kernel) {
    s_self = this;
    s_kernel = kernel;

    return true;
}

bool GameControl::Launched(IKernel * kernel) {
    return true;
}

bool GameControl::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

