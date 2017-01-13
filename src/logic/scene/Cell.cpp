#include "Cell.h"

bool Cell::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool Cell::Launched(IKernel * kernel) {
    return true;
}

bool Cell::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

