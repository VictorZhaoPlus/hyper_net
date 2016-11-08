/* 
 * File:   TableControl.h
 * Author: Alax
 *
 * Created on March 3, 2015, 10:46 AM
 */
#ifndef __TableControl_h__
#define __TableControl_h__

#include "TableStruct.h"
#include "IObjectMgr.h"
#include "OPool.h"
#include "TableRowPool.h"
#include <vector>
#include <map>
#include <list>
#include "OString.h"

#define MAX_STRING_KEY_LEN 64
#define MAX
class TableControl : public ITableControl{
    typedef std::vector<TableRow *> TABLE_ROWS;
    typedef std::unordered_map<s64, RowIndex> KEY_INT_MAP;
    typedef std::unordered_map<olib::OString<MAX_STRING_KEY_LEN>, RowIndex, olib::OStringHash<MAX_STRING_KEY_LEN>> KEY_STRING_MAP;
    typedef olib::Pool<TableControl> TABLE_POOL;

    typedef std::list<TableUpdateCallback> UPDATE_CB_POOL;
    typedef std::list<TableAddCallback> ADD_CB_POOL;
    typedef std::list<TableDeleteCallback> DELETE_CB_POOL;
    typedef std::list<TableSwapCallback> SWP_CB_POOL;

    struct TableCreateInfo {
        TableControl * table;
        const olib::OString<128> file;
        const s32 line;
    };

public:
    static TableControl * Create(const s32 name, IObject * host, const char * file, const s32 line) {
		TableControl * table = CREATE_FROM_POOL(s_oTablePool, name, host);
		s_leakMap.insert(std::make_pair(table, TableCreateInfo({ table, file, line })));
        return table;
    }

    static void EchoMemoryLeak(IKernel * pKernel) {
		for (auto itr = s_leakMap.begin(); itr != s_leakMap.end(); ++itr) {
			DBG_INFO("%s:%d table leak", itr->second.file.GetString(), itr->second.line);
		}
    }

    void Release() {
        s_oTablePool.Recover(this);
        s_leakMap.erase(this);
    }

    inline void Clear() {
        _host = NULL;
        _isInited = false;
        _columnInfo.key.index = -1;
        _columnInfo.key.type = DTYPE_CANT_BE_KEY;

        for (s32 i = 0; i < (s32)_rows.size(); ++i) {
            TableRowPool::GetInterface()->Recover(_rows[i]);
        }

        _updateCallPool.clear();
        _addCallPool.clear();
        _delCallPool.clear();
        _swapCallPool.clear();

        _rows.clear();
    }

    virtual void ClearRows() {
		for (s32 i = 0; i < (s32)_rows.size(); ++i) {
            TableRowPool::GetInterface()->Recover(_rows[i]);
        }

        _rows.clear();
        _intKeyMap.clear();
        _stringKeyMap.clear();
    }

    bool FormingWithTableInfo(const TableColumnInfo * columnInfo);

    //获取表主对象,如果是静态表则返回NULL
    virtual IObject * GetHost() {return _host;}

	virtual s32 GetTableName() const { return _tableName; }

    //注册回调
	virtual void RgsUpdate(TableUpdateCallback cb, const char * debug) { _updateCallPool.push_back(cb); }
	virtual void RgsAdd(TableAddCallback cb, const char * debug) { _addCallPool.push_back(cb); }
	virtual void RgsDelete(TableDeleteCallback cb, const char * debug) { _delCallPool.push_back(cb); }
	virtual void RgsChange(TableSwapCallback cb, const char * debug) { _swapCallPool.push_back(cb); }

    void UpdateCallBack(IKernel * pKernel, const s32 row, const s32 column, const void * value, const s32 size, const s8 type) {
		for (auto cb : _updateCallPool)
			cb(pKernel, this, row, column, value, size, type);
    }

    void AddCallBack(IKernel * pKernel, ITableControl * pTable, const s32 row, const void * key, const s32 size, const s8 type) {
		for (auto cb : _addCallPool)
			cb(pKernel, this, row, key, size, type);
    }

    void DeleteCallBack(IKernel * pKernel, ITableControl * pTable, const s32 row) {
		for (auto cb : _delCallPool)
			cb(pKernel, this, row);
    }

    void SwapCallBack(IKernel * pKernel, ITableControl * pTable, const s32 src, const s32 dst) {
		for (auto cb : _swapCallPool)
			cb(pKernel, this, src, dst);
    }

    //数据控制接口
    //通过key值查找行, 未找到返回-1
    virtual RowIndex FindRow(const s64 key) const;
    virtual RowIndex FindRow(const char * key) const;

    //获取行数
    virtual s32 RowCount() const { return (s32)_rows.size(); }
    //增加一行数据,失败返回-1,否则返回行index,如果为设了key的表,则key为必须值
    virtual RowIndex AddRow();
    virtual RowIndex AddRowKeyInt8(const s8 key);
    virtual RowIndex AddRowKeyInt16(const s16 key);
    virtual RowIndex AddRowKeyInt32(const s32 key);
    virtual RowIndex AddRowKeyInt64(const s64 key);
    virtual RowIndex AddRowKeyString(const char * key);

    virtual bool GetRowData(const s32 row, const void * & pData, s32 & size);
    virtual RowIndex AddRowData(const void * pData, const s32 size);
    //插入行
    virtual bool InsertRow(const RowIndex index);
    virtual bool InsertRowKeyInt8(const s8 key, const RowIndex index);
    virtual bool InsertRowKeyInt16(const s16 key, const RowIndex index);
    virtual bool InsertRowKeyInt32(const s32 key, const RowIndex index);
    virtual bool InsertRowKeyInt64(const s64 key, const RowIndex index);
    virtual bool InsertRowKeyString(const char * key, const RowIndex index);

    //删除一行(删除行后,该行后面的所有行index自减1
    virtual bool DelRow(const RowIndex index);
    //复制行
    virtual bool SwapRowIndex(const RowIndex src, const RowIndex dst);

    virtual s8 GetDataInt8(const RowIndex rowIndex, const ColumnIndex clmIndex) const;
    virtual s16 GetDataInt16(const RowIndex rowIndex, const ColumnIndex clmIndex) const;
    virtual s32 GetDataInt32(const RowIndex rowIndex, const ColumnIndex clmIndex) const;
    virtual s64 GetDataInt64(const RowIndex rowIndex, const ColumnIndex clmIndex) const;
    virtual float GetDataFloat(const RowIndex rowIndex, const ColumnIndex clmIndex) const;
    virtual const char * GetDataString(const RowIndex rowIndex, const ColumnIndex clmIndex) const;
    virtual const void * GetDataStruct(const RowIndex rowIndex, const ColumnIndex clmIndex, const s32 size) const;

    virtual bool SetDataInt8(const RowIndex rowIndex, const ColumnIndex clmIndex, const s8 value);
    virtual bool SetDataInt16(const RowIndex rowIndex, const ColumnIndex clmIndex, const s16 value);
    virtual bool SetDataInt32(const RowIndex rowIndex, const ColumnIndex clmIndex, const s32 value);
    virtual bool SetDataInt64(const RowIndex rowIndex, const ColumnIndex clmIndex, const s64 value);
    virtual bool SetDataFloat(const RowIndex rowIndex, const ColumnIndex clmIndex, const float value);
    virtual bool SetDataString(const RowIndex rowIndex, const ColumnIndex clmIndex, const char * value);
    virtual bool SetDataStruct(const RowIndex rowIndex, const ColumnIndex clmIndex, const void * pValue, const s32 size);
private:
    TableControl(const s32 name, IObject * pHost = NULL)
        : _tableName(name)
        , _host(pHost)
        , _isInited(false)
    {
        _columnInfo.key.index = -1;
        _columnInfo.key.type = DTYPE_CANT_BE_KEY;
        OASSERT(_rows.size() == 0, "wtf");
    }

    ~TableControl() {
        Clear();
    }

    bool ChangeKey(const s64 oldKey, const s64 newKey, const s8 type);
    bool ChangeKey(const char * oldKey, const char * newKey, const s8 type);
    void OrderProcIndex(const RowIndex index, const s32 index_diff); //改变index后面的行的index值

private:
    IObject * _host;

    UPDATE_CB_POOL _updateCallPool;
    ADD_CB_POOL _addCallPool;
    DELETE_CB_POOL _delCallPool;
    SWP_CB_POOL _swapCallPool;

    s32 _tableName;
    TableColumnInfo _columnInfo;

    KEY_INT_MAP _intKeyMap;
    KEY_STRING_MAP _stringKeyMap;
    TABLE_ROWS _rows;

    bool _isInited;

    friend TABLE_POOL;
    static TABLE_POOL s_oTablePool;
	static std::unordered_map<TableControl *, TableCreateInfo> s_leakMap;
};

#endif //defined __TableControl_h__
