/*
 * File: ICacheDB.h
 * Author: ooeyusea
 *
 * Created On November 25, 2016, 08:50 AM
 */

#ifndef __ICACHEDB_H__
#define __ICACHEDB_H__
 
#include "IModule.h"

class ICacheDBReader {
public:
	virtual ~ICacheDBReader() {}

	virtual void ReadColumn(const char * column) = 0;
};

typedef std::function<void(IKernel * kernel, ICacheDBReader * reader)> CacheDBColumnFuncType;

class ICacheDBReadResult {
public:
	virtual ~ICacheDBReadResult() {}

	virtual s32 Count() = 0;

	virtual s8 GetDataInt8(s32 row, s32 col) const = 0;
	virtual s16 GetDataInt16(s32 row, s32 col) const = 0;
	virtual s32 GetDataInt32(s32 row, s32 col) const = 0;
	virtual s64 GetDataInt64(s32 row, s32 col) const = 0;
	virtual float GetDataFloat(s32 row, s32 col) const = 0;
	virtual const char * GetDataString(s32 row, s32 col) const = 0;
	virtual const void * GetBinary(s32 row, s32 col, s32& size) const = 0;
};

typedef std::function<void(IKernel * kernel, ICacheDBReadResult * result)> CacheDBReadFuncType;

class ICacheDBContext {
public:
	virtual ~ICacheDBContext() {}

	virtual void WriteInt8(const char * col, s8 val) = 0;
	virtual void WriteInt16(const char * col, s16 val) = 0;
	virtual void WriteInt32(const char * col, s32 val) = 0;
	virtual void WriteInt64(const char * col, s64 val) = 0;
	virtual void WriteFloat(const char * col, float val) = 0;
	virtual void WriteString(const char * col, const char * val) = 0;
};

typedef std::function<void(IKernel * kernel, ICacheDBContext * context)> CacheDBWriteFunc;

class ICacheDB : public IModule {
public:
	virtual ~ICacheDB() {}

	virtual bool Read(const char * table, const CacheDBColumnFuncType& cf, const CacheDBReadFuncType& f, s32 count, ...) = 0;
	virtual bool ReadByIndex(const char * table, const CacheDBColumnFuncType& cf, const CacheDBReadFuncType& f, const s64 index) = 0;
	virtual bool ReadByIndex(const char * table, const CacheDBColumnFuncType& cf, const CacheDBReadFuncType& f, const char * index) = 0;

	virtual bool Write(const char * table, const CacheDBWriteFunc& f, s32 count, ...) = 0;
	virtual bool WriteByIndex(const char * table, const CacheDBWriteFunc& f, const s64 index) = 0;
	virtual bool WriteByIndex(const char * table, const CacheDBWriteFunc& f, const char * index) = 0;

	virtual bool Destroy(const char * table, s32 count, ...) = 0;
	virtual bool DestroyByIndex(const char * table, const s64 index) = 0;
	virtual bool DestroyByIndex(const char * table, const char * index) = 0;
};

#endif /*__ICACHEDB_H__ */
