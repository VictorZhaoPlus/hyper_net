#include "Distribution.h"

bool Distribution::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool Distribution::Launched(IKernel * kernel) {
    return true;
}

bool Distribution::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

