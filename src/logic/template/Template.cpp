#include "&.h"

& * &::s_self = nullptr;
IKernel * &::s_kernel = nullptr;

bool &::Initialize(IKernel * kernel) {
    s_self = this;
    s_kernel = kernel;

    return true;
}

bool &::Launched(IKernel * kernel) {
    return true;
}

bool &::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

