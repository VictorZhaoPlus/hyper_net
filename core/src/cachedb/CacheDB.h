#ifndef __CACHEDB_H__
#define __CACHEDB_H__
#include "util.h"
#include "ICacheDB.h"
#include "singleton.h"
#include <unordered_map>
#include <vector>

class IRedis;
class CacheDB : public ICacheDB, public OHolder<CacheDB> {
	enum {
		CDB_TYPE_NONE = 0,
		CDB_TYPE_INT8,
		CDB_TYPE_INT16,
		CDB_TYPE_INT32,
		CDB_TYPE_INT64,
		CDB_TYPE_STRING,

		CDB_TYPE_CANT_BE_KEY,
		CDB_TYPE_FLOAT = CDB_TYPE_CANT_BE_KEY,
		CDB_TYPE_STRUCT,
		CDB_TYPE_BLOB
	};

public:
	struct CacheTableKey {
		s8 type;
		std::string name;
	};

	struct CacheTable {
		std::string name;
		std::unordered_map<std::string, s8> columns;
		std::string key;
		CacheTableKey index;
		bool del;
	};

    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual bool Read(const char * table, const CacheDBColumnFuncType& cf, const CacheDBReadFuncType& f, s32 count, ...);
	virtual bool ReadByIndex(const char * table, const CacheDBColumnFuncType& cf, const CacheDBReadFuncType& f, const s64 index);
	virtual bool ReadByIndex(const char * table,  const CacheDBColumnFuncType& cf, const CacheDBReadFuncType& f, const char * index);

	virtual bool Write(const char * table, const CacheDBWriteFunc& f, s32 count, ...);
	virtual bool WriteByIndex(const char * table, const CacheDBWriteFunc& f, const s64 index);
	virtual bool WriteByIndex(const char * table, const CacheDBWriteFunc& f, const char * index);

	virtual bool Destroy(const char * table, s32 count, ...);
	virtual bool DestroyByIndex(const char * table, const s64 index);
	virtual bool DestroyByIndex(const char * table, const char * index);

private:
    IKernel * _kernel;

	std::unordered_map<std::string, CacheTable> _tables;
};

#endif //__CACHEDB_H__

