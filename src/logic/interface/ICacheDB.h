/*
 * File: ICacheDB.h
 * Author: ooeyusea
 *
 * Created On November 25, 2016, 08:50 AM
 */

#ifndef __ICACHEDB_H__
#define __ICACHEDB_H__
 
#include "IModule.h"

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
	virtual ~ICacheDBContext() = 0;

	virtual void WriteInt8(const char * col, s8 val) = 0;
	virtual void WriteInt16(const char * col, s16 val) = 0;
	virtual void WriteInt32(const char * col, s32 val) = 0;
	virtual void WriteInt64(const char * col, s64 val) = 0;
	virtual void WriteFloat(const char * col, float val) = 0;
	virtual void WriteString(const char * col, const char * val) = 0;
	virtual void WriteBinary(const char * col, const void * context, const s32 size) = 0;

	virtual bool Update() = 0;
};

class ICacheDB : public IModule {
public:
	virtual ~ICacheDB() {}

	virtual bool Read(const char * table, const s64 id, const CacheDBReadFuncType& f, ...) = 0;
	virtual bool ReadWithParamArray(const char * table, const s64 id, const CacheDBReadFuncType& f, char ** params, s32 count) = 0;
	virtual bool ReadByIndex(const char * table, const s32 idx, const CacheDBReadFuncType& f, ...) = 0;

	virtual ICacheDBContext * PrepareWrite(const char * table, const s64 id) = 0;
	virtual void Destroy(const char * table, const s64 id) = 0;
};

#endif /*__ICACHEDB_H__ */
