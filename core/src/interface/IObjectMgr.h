/* 
 * File:   IObjectMgr.h
 * Author: ooeyusea
 *
 * Created on March 3, 2015, 10:46 AM
 */

#ifndef __IOBJECTMGR_h__
#define __IOBJECTMGR_h__

#include "IModule.h"
#include <vector>

#define MAX_MODEL_NAME_LEN 64
enum {
    DTYPE_INT8 = 0,
    DTYPE_INT16,
    DTYPE_INT32,
    DTYPE_INT64,
    DTYPE_STRING,

    DTYPE_CANT_BE_KEY,
    DTYPE_FLOAT = DTYPE_CANT_BE_KEY,
    DTYPE_STRUCT,
    DTYPE_BLOB
};

class ITableControl;
class IRow;
typedef std::function<void (IKernel * kernel, ITableControl * table, IRow * row, const s32 col, const s8 type)> TableUpdateCallback;
typedef std::function<void (IKernel * kernel, ITableControl * table, IRow * row, const void * key, const s32 size, const s8 type)> TableAddCallback;
typedef std::function<void (IKernel * kernel, ITableControl * table, IRow * row)> TableDeleteCallback;
typedef std::function<void (IKernel * kernel, ITableControl * table, IRow * src, IRow * dst)> TableSwapCallback;

#define RGS_TABLE_UPDATE(table, call) table->RgsUpdate(std::bind(&call, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5), #call);
#define RGS_TABLE_ADD(table, call) table->RgsAdd(std::bind(&call, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6), #call);
#define RGS_TABLE_DELETE(table, call) table->RgsDelete(std::bind(&call, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), #call);
#define RGS_TABLE_CHANGE(table, call) table->RgsChange(std::bind(&call, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), #call);

class IRow {
public:
	virtual ~IRow() {}

	virtual s32 GetRowIndex() const = 0;

	virtual s8 GetDataInt8(const s32 col) const = 0;
	virtual s16 GetDataInt16(const s32 col) const = 0;
	virtual s32 GetDataInt32(const s32 col) const = 0;
	virtual s64 GetDataInt64(const s32 col) const = 0;
	virtual float GetDataFloat(const s32 col) const = 0;
	virtual const char * GetDataString(const s32 col) const = 0;
	virtual const void * GetDataStruct(const s32 col, const s32 size) const = 0;
	virtual const void * GetDataBlob(const s32 col, s32& size) const = 0;

	virtual void SetDataInt8(const s32 col, const s8 value) = 0;
	virtual void SetDataInt16(const s32 col, const s16 value) = 0;
	virtual void SetDataInt32(const s32 col, const s32 value) = 0;
	virtual void SetDataInt64(const s32 col, const s64 value) = 0;
	virtual void SetDataFloat(const s32 col, const float value) = 0;
	virtual void SetDataString(const s32 col, const char * value) = 0;
	virtual void SetDataStruct(const s32 col, const void * value, const s32 size) = 0;
	virtual void SetDataBlob(const s32 col, const void * value, const s32 size) = 0;
};

class IObject;
class ITableControl {
public:
    virtual ~ITableControl() {}

    virtual void RgsUpdate(TableUpdateCallback callback, const char * debug) = 0;
    virtual void RgsAdd(TableAddCallback callback, const char * debug) = 0;
    virtual void RgsDelete(TableDeleteCallback callback, const char * debug) = 0;
    virtual void RgsChange(TableSwapCallback callback, const char * debug) = 0;

    virtual IObject * GetHost() = 0;

	virtual s32 RowCount() const = 0;
    virtual void ClearRows() = 0;

    virtual IRow * FindRow(const s64 key) const = 0;
    virtual IRow * FindRow(const char * key) const = 0;
	virtual IRow * GetRow(const s32 index) const = 0;

    virtual IRow * AddRow() = 0;
    virtual IRow * AddRowKeyInt8(const s8 key) = 0;
    virtual IRow * AddRowKeyInt16(const s16 key) = 0;
    virtual IRow * AddRowKeyInt32(const s32 key) = 0;
    virtual IRow * AddRowKeyInt64(const s64 key) = 0;
    virtual IRow * AddRowKeyString(const char * key) = 0;

    virtual bool DelRow(const s32 index) = 0;
    virtual bool SwapRowIndex(const s32 src, const s32 dst) = 0;
};

#define DEL_TABLE_ROW(table, row) {\
	OASSERT(table && row, "table or row index is invalid");\
	if (table && row) {\
		s32 lastRow = table->RowCount() - 1; \
		if (lastRow != row->GetRowIndex()) \
			table->SwapRowIndex(row->GetRowIndex(), lastRow); \
		table->DelRow(lastRow);\
	}\
}

class IProp {
public:
	virtual ~IProp() {}

	virtual const s32 GetName() const = 0;
	virtual const char * GetRealName() const = 0;
	virtual const s8 GetType(IObject * object) const = 0;
	virtual const s32 GetSetting(IObject * object) const = 0;
};

typedef std::function<void (IKernel * kernel, IObject * object, const char * name, const IProp * prop, const bool sync)> PropCallback;
#define RGS_PROP_CHANGER(obj, prop, cb) obj->RgsPropChangeCB(prop, std::bind(&cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5), #cb)
class IObject {
public:
    virtual ~IObject() {}

    virtual const s64 GetID() const = 0;
    virtual const char * GetTypeName() const = 0;

    virtual const std::vector<const IProp*>& GetPropsInfo(bool noFather = false) const = 0;

    virtual bool SetPropInt8(const IProp * prop, const s8 value, const bool sync = true) = 0;
    virtual bool SetPropInt16(const IProp * prop, const s16 value, const bool sync = true) = 0;
    virtual bool SetPropInt32(const IProp * prop, const s32 value, const bool sync = true) = 0;
    virtual bool SetPropInt64(const IProp * prop, const s64 value, const bool sync = true) = 0;
    virtual bool SetPropFloat(const IProp * prop, const float value, const bool sync = true) = 0;
    virtual bool SetPropString(const IProp * prop, const char * value, const bool sync = true) = 0;
    virtual bool SetPropStruct(const IProp * prop, const void * value, const s32 len, const bool sync = true) = 0;
	virtual bool SetPropBlob(const IProp * prop, const void * value, const s32 len, const bool sync = true) = 0;

    virtual s8 GetPropInt8(const IProp * prop) const = 0;
    virtual s16 GetPropInt16(const IProp * prop) const = 0;
    virtual s32 GetPropInt32(const IProp * prop) const = 0;
    virtual s64 GetPropInt64(const IProp * prop) const = 0;
    virtual float GetPropFloat(const IProp * prop) const = 0;
    virtual const char * GetPropString(const IProp * prop) const = 0;
    virtual const void * GetPropStruct(const IProp * prop, const s32 len) const = 0;
	virtual const void * GetPropBlob(const IProp * prop, s32& len) const = 0;

    virtual void RgsPropChangeCB(const IProp * prop, const PropCallback& cb, const char * debug_info) = 0;

    virtual ITableControl * FindTable(const s32 name) const = 0;
};

#define CREATE_OBJECT(mgr, ...)  mgr->Create(__FILE__, __LINE__, __VA_ARGS__)
#define CREATE_OBJECT_BYID(mgr, ...) mgr->CreateObjectByID(__FILE__, __LINE__, __VA_ARGS__)
#define CREATE_STATIC_TABLE(mgr, name, model) mgr->CreateStaticTable(name, model, __FILE__, __LINE__)

class IObjectMgr : public IModule {
public:
    virtual ~IObjectMgr() {}

    virtual IObject * Create(const char * file, const s32 line, const char * name) = 0;
    virtual IObject * CreateObjectByID(const char * file, const s32 line, const char * name, const s64 id) = 0;
    virtual IObject * FindObject(const s64 id) = 0;
    virtual void Recove(IObject * pObject) = 0;

	virtual const IProp * CalcProp(const char * name) = 0;
	virtual const IProp * CalcProp(const s32 name) = 0;
	virtual s32 CalcPropSetting(const char * setting) = 0;
	virtual s32 CalcTableName(const char * table) = 0;

    virtual const std::vector<const IProp*>* GetPropsInfo(const char * type, bool noFather = false) const = 0;

    virtual ITableControl * CreateStaticTable(const char * name, const char * model, const char * file, const s32 line) = 0;
    virtual void RecoverStaticTable(ITableControl * table) = 0;
};

constexpr s32 CalcUniqueId(s32 hash, const char * str) {
	return *str ? CalcUniqueId(hash * 131 + (*str), str + 1) : hash;
}

template <s32>
struct PropGetter {
	inline static const IProp * Get(const char * name) {
		const IProp * prop = nullptr;
		if (!prop) {
			prop = OMODULE(ObjectMgr)->CalcProp(name);
			OASSERT(prop, "wtf");
		}
		return prop;
	}
};

#define OPROP(name) (PropGetter<CalcUniqueId(0, name)>::Get(name))
#endif //define __IOBJECTMGR_h__
