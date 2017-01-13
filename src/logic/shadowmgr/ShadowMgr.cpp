#include "ShadowMgr.h"

bool ShadowMgr::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool ShadowMgr::Launched(IKernel * kernel) {
    return true;
}

bool ShadowMgr::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

