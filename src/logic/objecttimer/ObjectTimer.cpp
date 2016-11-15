#include "ObjectTimer.h"

bool ObjectTimer::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool ObjectTimer::Launched(IKernel * kernel) {
    return true;
}

bool ObjectTimer::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

