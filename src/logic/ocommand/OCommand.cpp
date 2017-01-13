#include "OCommand.h"

bool OCommand::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool OCommand::Launched(IKernel * kernel) {
    return true;
}

bool OCommand::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

