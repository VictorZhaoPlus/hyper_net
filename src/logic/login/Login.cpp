#include "Login.h"

bool Login::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool Login::Launched(IKernel * kernel) {
    return true;
}

bool Login::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

