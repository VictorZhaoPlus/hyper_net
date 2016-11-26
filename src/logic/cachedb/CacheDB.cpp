#include "CacheDB.h"

bool CacheDB::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool CacheDB::Launched(IKernel * kernel) {
    return true;
}

bool CacheDB::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

