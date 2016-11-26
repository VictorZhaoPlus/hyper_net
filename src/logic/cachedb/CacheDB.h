#ifndef __CACHEDB_H__
#define __CACHEDB_H__
#include "util.h"
#include "ICacheDB.h"
#include "singleton.h"

class CacheDB : public ICacheDB, public OHolder<CacheDB> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

private:
    IKernel * _kernel;
};

#endif //__CACHEDB_H__

