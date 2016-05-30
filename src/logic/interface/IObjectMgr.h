/* 
 * File:   IObjectMgr.h
 * Author: ooeyusea
 *
 * Created on March 3, 2015, 10:46 AM
 */

#ifndef __IOBJECTMGR_h__
#define __IOBJECTMGR_h__

#include "IModule.h"
#include <unordered_map>
#include <vector>

#define INVALID_ROW_INDEX -1
#define OUT 

#define MAX_MODEL_NAME_LEN 64
#define MAX_MODEL_PATH_LEN 260
#define MAX_TABLE_NAME_LEN 64

typedef s32 RowIndex, ColumnIndex;
class IObject;
class IObjectMgr;
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
typedef void(*table_update)(IKernel * kernel, ITableControl * table, bool copy, const s32 row, const s32 column, const void * value, const s32 size, const s8 type);
typedef void(*table_add)(IKernel * kernel, ITableControl * table, bool copy, const s32 row, const void * key, const s32 size, const s8 type);
typedef void(*table_delete)(IKernel * kernel, ITableControl * table, bool copy, const s32 row);
typedef void(*table_change)(IKernel * kernel, ITableControl * table, bool copy, const s32 src, const s32 dst);

#define RGS_TABLE_UPDATE(table, call) table->RgsUpdate(call, #call);
#define RGS_TABLE_ADD(table, call) table->RgsAdd(call, #call);
#define RGS_TABLE_DELETE(table, call) table->RgsDelete(call, #call);
#define RGS_TABLE_CHANGE(table, call) table->RgsChange(call, #call);

class ITableControl {
public:
    virtual ~ITableControl() {}

    virtual void RgsUpdate(table_update callback, const char * debug) = 0;
    virtual void RgsAdd(table_add callback, const char * debug) = 0;
    virtual void RgsDelete(table_delete callback, const char * debug) = 0;
    virtual void RgsChange(table_change callback, const char * debug) = 0;

    //是否为同步到影子
    virtual bool IsShadow() const = 0;

    //获取表主对象,如果是静态表则返回NULL
    virtual IObject * GetHost() = 0;

	//获取表名字
	virtual const char * GetTableName() const = 0;

    //初始化表结构(尽量在创建对象时初始化), 调用Forming生成表结构
    virtual bool AddColumnInt8(const ColumnIndex index, bool isKey = false) = 0;
    virtual bool AddColumnInt16(const ColumnIndex index, bool isKey = false) = 0;
    virtual bool AddColumnInt32(const ColumnIndex index, bool isKey = false) = 0;
    virtual bool AddColumnInt64(const ColumnIndex index, bool isKey = false) = 0;
    virtual bool AddColumnString(const ColumnIndex index, const s32 maxlen, bool isKey = false) = 0;
    virtual bool AddColumnFloat(const ColumnIndex index) = 0;
    virtual bool AddColumnStruct(const ColumnIndex index, const s32 size) = 0;
    virtual bool Forming() = 0;

    //清空表
    virtual void ClearRows() = 0;

    //数据控制接口
    //通过key值查找行
    virtual RowIndex FindRow(const s64 key) const = 0;
    virtual RowIndex FindRow(const char * key) const = 0;

    //获取行数
    virtual s32 RowCount() const = 0;
    //增加一行数据,失败返回-1,否则返回行index
    virtual RowIndex AddRow() = 0;
    virtual RowIndex AddRowKeyInt8(const s8 key) = 0;
    virtual RowIndex AddRowKeyInt16(const s16 key) = 0;
    virtual RowIndex AddRowKeyInt32(const s32 key) = 0;
    virtual RowIndex AddRowKeyInt64(const s64 key) = 0;
    virtual RowIndex AddRowKeyString(const char * key) = 0;

    virtual bool GetRowData(const s32 row, const void * & pData, s32 & size) = 0;
    virtual RowIndex AddRowData(const void * pData, const s32 size) = 0;

    virtual bool InsertRow(const RowIndex index) = 0;
    virtual bool InsertRowKeyInt8(const s8 key, const RowIndex index) = 0;
    virtual bool InsertRowKeyInt16(const s16 key, const RowIndex index) = 0;
    virtual bool InsertRowKeyInt32(const s32 key, const RowIndex index) = 0;
    virtual bool InsertRowKeyInt64(const s64 key, const RowIndex index) = 0;
    virtual bool InsertRowKeyString(const char * key, const RowIndex index) = 0;

    //删除一行(删除行后,该行后面的所有行index自减1
    virtual bool DelRow(const RowIndex index) = 0;
    //复制行
    virtual bool SwapRowIndex(const RowIndex src, const RowIndex dst) = 0;

    virtual s8 GetDataInt8(const RowIndex rowIndex, const ColumnIndex clmIndex) const = 0;
    virtual s16 GetDataInt16(const RowIndex rowIndex, const ColumnIndex clmIndex) const = 0;
    virtual s32 GetDataInt32(const RowIndex rowIndex, const ColumnIndex clmIndex) const = 0;
    virtual s64 GetDataInt64(const RowIndex rowIndex, const ColumnIndex clmIndex) const = 0;
    virtual float GetDataFloat(const RowIndex rowIndex, const ColumnIndex clmIndex) const = 0;
    virtual const char * GetDataString(const RowIndex rowIndex, const ColumnIndex clmIndex) const = 0;
    virtual const void * GetDataStruct(const RowIndex rowIndex, const ColumnIndex clmIndex, const s32 size) const = 0;

    virtual bool SetDataInt8(const RowIndex rowIndex, const ColumnIndex clmIndex, const s8 value) = 0;
    virtual bool SetDataInt16(const RowIndex rowIndex, const ColumnIndex clmIndex, const s16 value) = 0;
    virtual bool SetDataInt32(const RowIndex rowIndex, const ColumnIndex clmIndex, const s32 value) = 0;
    virtual bool SetDataInt64(const RowIndex rowIndex, const ColumnIndex clmIndex, const s64 value) = 0;
    virtual bool SetDataFloat(const RowIndex rowIndex, const ColumnIndex clmIndex, const float value) = 0;
    virtual bool SetDataString(const RowIndex rowIndex, const ColumnIndex clmIndex, const char * value) = 0;
    virtual bool SetDataStruct(const RowIndex rowIndex, const ColumnIndex clmIndex, const void * value, const s32 size) = 0;
};

#define DEL_TABLE_ROW(table, row) {\
	OASSERT(table && row != INVALID_ROW_INDEX, "table or row index is invalid");\
	if (table && row != INVALID_ROW_INDEX) {\
		RowIndex lastRow = table->RowCount() - 1; \
		if (lastRow != row) \
			table->SwapRowIndex(row, lastRow); \
		table->DelRow(lastRow);\
	}\
}

struct PropSetting {
    const bool visable;
    const bool share;
    const bool save;
    const bool signficant;
    const bool copy;
    PropSetting(const bool _visable, const bool _share, const bool _save, const bool _signficant, const bool _copy)
        : visable(_visable), share(_share), save(_save), signficant(_signficant), copy(_copy) {}
};

struct PropInfo{
    PropInfo(const s32 _index, const s32 _offset, const s32 _size, const s8 _mask,
    const bool _visable, const bool _share, const bool _save, const bool _signficant, const bool _copy)
    : index(_index), offset(_offset), size(_size), mask(_mask),
    setting(_visable, _share, _save, _signficant, _copy){}
    const s32 index;
    const s32 offset;
    const s32 size;
    const s8 mask;
    const PropSetting setting;
};
typedef std::vector<PropInfo> PROP_INDEX;

#define RGS_ENTRY_STATUS(obj, status, cb) obj->RgsEntryCB(status, cb, #cb)
#define RGS_LEAVE_STATUS(obj, status, cb) obj->RgsLeaveCB(status, cb, #cb)
typedef void(*status_cb)(IKernel *, IObject *);

#define RGS_ENTRY_JUDEG(obj, status, cb) obj->RgsEntryJudegCB(status, cb, #cb)
#define RGS_LEAVE_JUDEG(obj, status, cb) obj->RgsLeaveJudegCB(status, cb, #cb)
typedef bool(*status_judeg_cb)(IKernel *, IObject *);

#define ANY_CALL NULL
typedef void(*prop_cb)(IKernel * kernel, IObject * pObject, const char * name, const char * prop, const PropInfo * setting, const bool sync);
#define RGS_PROP_CHANGER(obj, prop, cb) obj->RgsPropChangeCB(prop, cb, #cb)


#define CREATE_OBJECT_TABLE(obj, name) obj->CreateTable(name, __FILE__, __LINE__)
class IObject {
public:
    virtual ~IObject() {}

    virtual const s64 GetID() const = 0;
    virtual const char * GetTypeName() const = 0;
    virtual const bool IsShadow() const = 0;

    virtual const PROP_INDEX & GetPropsInfo(bool noFather = false) const = 0;
    virtual bool SetPropInt8(const s32 prop, const s8 value, const bool sync = true) = 0;
    virtual bool SetPropInt16(const s32 prop, const s16 value, const bool sync = true) = 0;
    virtual bool SetPropInt32(const s32 prop, const s32 value, const bool sync = true) = 0;
    virtual bool SetPropInt64(const s32 prop, const s64 value, const bool sync = true) = 0;
    virtual bool SetPropFloat(const s32 prop, const float value, const bool sync = true) = 0;
    virtual bool SetPropString(const s32 prop, const char * value, const bool sync = true) = 0;
    virtual bool SetPropStruct(const s32 prop, const void * value, const s32 len, const bool sync = true) = 0;

    virtual s8 GetPropInt8(const s32 prop) const = 0;
    virtual s16 GetPropInt16(const s32 prop) const = 0;
    virtual s32 GetPropInt32(const s32 prop) const = 0;
    virtual s64 GetPropInt64(const s32 prop) const = 0;
    virtual float GetPropFloat(const s32 prop) const = 0;
    virtual const char * GetPropString(const s32 prop) const = 0;
    virtual const void * GetPropStruct(const s32 prop, s32& len) const = 0;

    virtual bool RgsPropChangeCB(const s32 prop, const prop_cb cb, const char * debug_info) = 0;

    //创建对象所有表
    virtual ITableControl * CreateTable(const char * name, const char * file, const s32 line) = 0;
    //通过名称获取表
    virtual ITableControl * FindTable(const char * name) const = 0;
    //删除表
    virtual bool RemoveTable(const char * name) = 0;

    //状态机
    virtual bool RgsEntryCB(const s32 status, const status_cb cb, const char * debuginfo) = 0;
    virtual bool RgsLeaveCB(const s32 status, const status_cb cb, const char * debuginfo) = 0;
    virtual bool RgsEntryJudegCB(const s32 status, const status_judeg_cb cb, const char * debuginfo) = 0;
    virtual bool RgsLeaveJudegCB(const s32 status, const status_judeg_cb cb, const char * debuginfo) = 0;
    virtual bool EntryStatus(const s32 status, const bool b = false) = 0;
    virtual const s32 GetStatus() const = 0;
};

#define CREATE_OBJECT(mgr, ...)  mgr->Create(__FILE__, __LINE__, __VA_ARGS__)
#define CREATE_OBJECT_BYID(mgr, ...) mgr->CreateObjectByID(__FILE__, __LINE__, __VA_ARGS__)
#define CREATE_STATIC_TABLE(mgr, name) mgr->CreateStaticTable(name, __FILE__, __LINE__)

class IObjectMgr : public IModule {
public:
    virtual ~IObjectMgr() {}

    virtual s64 AllotID() = 0;
    virtual IObject * Create(const char * file, const s32 line, const char * name, bool shadow = false) = 0;
    virtual IObject * CreateObjectByID(const char * file, const s32 line, const char * name, const s64 id, bool shadow = false) = 0;
    virtual IObject * FindObject(const s64 id) = 0;
    virtual void Recove(IObject * pObject) = 0;

    virtual const PROP_INDEX * GetPropsInfo(const char * type, bool noFather = false) const = 0;

    //创建对象类型静态表
    virtual ITableControl * CreateStaticTable(const char * name, const char * file, const s32 line) = 0;
    virtual void RecoverStaticTable(ITableControl * table) = 0;
    //通过名称获取静态表
    virtual ITableControl * FindStaticTable(const char * name) = 0;
};

#endif //define __IOBJECTMGR_h__
