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

class IColumn {
public:
	virtual ~IColumn() {};

	virtual const char * GetRealName() const = 0;
};

class ITable {
public:
	virtual ~ITable() {};

	virtual const char * GetRealName() const = 0;

	virtual const IColumn * GetCol(const char *) const = 0;
};


class ITableControl;
class IRow;
typedef std::function<void (IKernel * kernel, ITableControl * table, IRow * row, const IColumn * col, const s8 type)> TableUpdateCallback;
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

	virtual s8 GetDataInt8(const IColumn * col) const = 0;
	virtual s16 GetDataInt16(const IColumn * col) const = 0;
	virtual s32 GetDataInt32(const IColumn * col) const = 0;
	virtual s64 GetDataInt64(const IColumn * col) const = 0;
	virtual float GetDataFloat(const IColumn * col) const = 0;
	virtual const char * GetDataString(const IColumn * col) const = 0;
	virtual const void * GetDataStruct(const IColumn * col, const s32 size) const = 0;
	virtual const void * GetDataBlob(const IColumn * col, s32& size) const = 0;

	virtual void SetDataInt8(const IColumn * col, const s8 value) = 0;
	virtual void SetDataInt16(const IColumn * col, const s16 value) = 0;
	virtual void SetDataInt32(const IColumn * col, const s32 value) = 0;
	virtual void SetDataInt64(const IColumn * col, const s64 value) = 0;
	virtual void SetDataFloat(const IColumn * col, const float value) = 0;
	virtual void SetDataString(const IColumn * col, const char * value) = 0;
	virtual void SetDataStruct(const IColumn * col, const void * value, const s32 size) = 0;
	virtual void SetDataBlob(const IColumn * col, const void * value, const s32 size) = 0;
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

	virtual const s64 GetName() const = 0;
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
	
	template <typename T>
	T& GetPropT(const IProp * prop) { return *(T*)GetPropStruct(prop, sizeof(T)); }

    virtual void RgsPropChangeCB(const IProp * prop, const PropCallback& cb, const char * debug_info) = 0;

    virtual ITableControl * FindTable(const ITable * table) const = 0;
};

#define CREATE_OBJECT(...)  OMODULE(ObjectMgr)->Create(__FILE__, __LINE__, __VA_ARGS__)
#define CREATE_OBJECT_BYID(...) OMODULE(ObjectMgr)->CreateObjectByID(__FILE__, __LINE__, __VA_ARGS__)
#define CREATE_STATIC_TABLE(name, model) OMODULE(ObjectMgr)->CreateStaticTable(name, model, __FILE__, __LINE__)

typedef std::function<void(void * p, s32 size)> PropFunc;
typedef std::function<void(IKernel * kernel, IObject*)> ObjectCRCB;
class IObjectMgr : public IModule {
public:
    virtual ~IObjectMgr() {}

    virtual IObject * Create(const char * file, const s32 line, const char * name) = 0;
    virtual IObject * CreateObjectByID(const char * file, const s32 line, const char * name, const s64 id) = 0;
    virtual IObject * FindObject(const s64 id) = 0;
    virtual void Recove(IObject * pObject) = 0;

	virtual const IProp * CalcProp(const char * name, const char * module = "") = 0;
	virtual const IProp * CalcProp(const s64 name) = 0;
	virtual s32 CalcPropSetting(const char * setting) = 0;
	virtual const ITable * CalcTable(const char * name, const char * module = "") = 0;

	inline void ExtendInt8(const char * type, const char * name, const char * module = "") { ExtendData(type, module, name, DTYPE_INT8, sizeof(s8)); }
	inline void ExtendInt16(const char * type, const char * name, const char * module = "") { ExtendData(type, module, name, DTYPE_INT16, sizeof(s16)); }
	inline void ExtendInt32(const char * type, const char * name, const char * module = "") { ExtendData(type, module, name, DTYPE_INT32, sizeof(s32)); }
	inline void ExtendInt64(const char * type, const char * name, const char * module = "") { ExtendData(type, module, name, DTYPE_INT64, sizeof(s64)); }
	inline void ExtendFloat(const char * type, const char * name, const char * module = "") { ExtendData(type, module, name, DTYPE_FLOAT, sizeof(float)); }
	inline void ExtendString(const char * type, const char * name, const s32 size, const char * module = "") { ExtendData(type, module, name, DTYPE_STRING, size); }
	inline void ExtendBlob(const char * type, const char * name, const s32 size, const char * module = "") { ExtendData(type, module, name, DTYPE_BLOB, size); }

	template <typename T>
	inline void ExtendT(const char * type, const char * name, const char * module = "") {
		ExtendData(type, module, name, DTYPE_STRUCT, sizeof(T), [](void * p, s32 size) {
			new (p) T;
		}, [](void * p, s32 size) {
			T * t = (T*)p;
			t->clear();
		}, [](void * p, s32 size) {
			T * t = (T*)p;
			t->~T();
		});
	}

	//virtual void ExtendSetting(const char * type, const char * name, const char * module, s32 count, ...) = 0;

    virtual const std::vector<const IProp*>* GetPropsInfo(const char * type, bool noFather = false) const = 0;

    virtual ITableControl * CreateStaticTable(const char * name, const ITable * model, const char * file, const s32 line) = 0;
    virtual void RecoverStaticTable(ITableControl * table) = 0;

	virtual void RgsObjectCRCB(const char * type, const ObjectCRCB& init, const ObjectCRCB& uninit) = 0;

protected:
	virtual void ExtendData(const char * type, const char * module, const char * name, const s32 dataType, const s32 size, const PropFunc& init = nullptr, const PropFunc& reset = nullptr, const PropFunc& uninit = nullptr) = 0;
};

template <s64, s64>
struct PropGetter {
	inline static const IProp * Get(const char * module, const char * name) {
		static const IProp * prop = nullptr;
		if (!prop) {
			prop = OMODULE(ObjectMgr)->CalcProp(name, module);
			OASSERT(prop, "wtf");
		}
		return prop;
	}
};

#define OMPROP(module, name) (PropGetter<CalcUniqueId(0, module), CalcUniqueId(0, name)>::Get(module, name))
#define OPROP(name) OMPROP("", name)

template <s64>
struct SettingGetter {
	inline static s32 Get(const char * name) {
		static const s32 prop = OMODULE(ObjectMgr)->CalcPropSetting(name);
		return prop;
	}
};
#define OSETTING(name) (SettingGetter<CalcUniqueId(0, name)>::Get(name))

template <s64, s64> 
struct TableGetter{
	inline static const ITable * Get(const char * module, const char * name) {
		static const ITable * table = nullptr;
		if (!table) {
			table = OMODULE(ObjectMgr)->CalcTable(name, module);
			OASSERT(table, "wtf");
		}
		return table;
	}

	template <s64>
	struct ColumnGetter {
		inline static const IColumn * Get(const char * module, const char * name, const char * column) {
			static const IColumn * col = nullptr;
			if (!col) {
				col = TableGetter::Get(name, module)->GetCol(column);
				OASSERT(col, "wtf");
			}
			return col;
		}
	};
};

#define OMTABLE(module, name) (TableGetter<CalcUniqueId(0, module), CalcUniqueId(0, name)>::Get(module, name))
#define OTABLE(name) OMTABLE("", name)
#define OMCOLUMN(module, name, col) (TableGetter<CalcUniqueId(0, module), CalcUniqueId(0, name)>::ColumnGetter<CalcUniqueId(0, col)>::Get(module, name, col))
#define OCOLUMN(name, col) OMCOLUMN("", name, col)

#endif //define __IOBJECTMGR_h__
