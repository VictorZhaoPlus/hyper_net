#include "SceneMgr.h"

bool SceneMgr::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool SceneMgr::Launched(IKernel * kernel) {
    return true;
}

bool SceneMgr::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

