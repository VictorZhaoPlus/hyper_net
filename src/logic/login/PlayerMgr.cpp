#include "PlayerMgr.h"

bool PlayerMgr::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool PlayerMgr::Launched(IKernel * kernel) {
    return true;
}

bool PlayerMgr::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

