#include "&.h"

bool &::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool &::Launched(IKernel * kernel) {
    return true;
}

bool &::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

