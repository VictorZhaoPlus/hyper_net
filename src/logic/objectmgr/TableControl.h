/* 
 * File:   TableControl.h
 * Author: Alax
 *
 * Created on March 3, 2015, 10:46 AM
 */
#ifndef __TableControl_h__
#define __TableControl_h__

#include "TableStruct.h"
#include "IDCCenter.h"
#include "ObjectMgrMacro.h"
#include "TPool.h"
#include "TableRowPool.h"
#include <vector>
#include <map>

class TableControl : public ITableControl{
    typedef std::vector<TableRow *> TABLE_ROWS;
    typedef std::unordered_map<s64, RowIndex> KEY_INT_MAP;
    typedef std::unordered_map<tlib::TString<game::MAX_NAME_LEN>, RowIndex, tools::HashFunc<tlib::TString<game::MAX_NAME_LEN>>, tools::ComFunc<tlib::TString<game::MAX_NAME_LEN>> > KEY_STRING_MAP;
    typedef tlib::TPool<TableControl> TABLE_POOL;
    

    typedef list<table_update> UPDATE_CB_POOL;
    typedef list<table_add> ADD_CB_POOL;
    typedef list<table_delete> DELETE_CB_POOL;
    typedef list<table_change> CHG_CB_POOL;

    struct TableCreateInfo {
        TableCreateInfo(TableControl * _table, const char * _file, const s32 _line) : pTable(_table), file(_file), line(_line) {}
        TableControl * pTable;
        const tlib::TString<128> file;
        const s32 line;
    };

public:
    static TableControl * Create(const char * name, IObject * pHost, const char * file, const s32 line) {
		TableControl * pTable = CREATE_FROM_POOL(s_oTablePool, name, pHost);
        s_leakMap.insert(make_pair(pTable, TableCreateInfo(pTable, file, line)));
        return pTable;
    }

    static void EchoMemoryLeak(IKernel * pKernel) {
        unordered_map<TableControl *, TableCreateInfo>::const_iterator itor = s_leakMap.begin();
        while (itor != s_leakMap.end()) {
            DBG_INFO("%s:%d table leak", itor->second.file, itor->second.line);
            itor++;
        }
    }

    void Release() {
        s_oTablePool.Recover(this);
        s_leakMap.erase(s_leakMap.find(this));
    }

    inline void Clear() {
        m_pHost = NULL;
        m_bIsInited = false;
        m_oColumnInfo.key.index = -1;
        m_oColumnInfo.key.type = DTYPE_CANT_BE_KEY;

        s32 nCount = m_oRows.size();
        for (s32 i=0; i<nCount; i++) {
            TableRowPool::GetInterface()->Recover(m_oRows[i]);
        }

        m_oUpdateCallPool.clear();
        m_oAddCallPool.clear();
        m_oDelCallPool.clear();
        m_oChangeCallPool.clear();

        m_oRows.clear();
    }

    virtual void ClearRows() {
        s32 nCount = m_oRows.size();
        for (s32 i=0; i < nCount; i++) {
            TableRowPool::GetInterface()->Recover(m_oRows[i]);
        }

        m_oRows.clear();
        m_oIntKeyMap.clear();
        m_oStringKeyMap.clear();
    }

    bool FormingWithTableInfo(const TableColumnInfo * pColumnInfo);

    //获取表主对象,如果是静态表则返回NULL
    virtual IObject * GetHost() {return m_pHost;}

    virtual const char * GetTableName() const {
        return m_oTableName.GetString();
    }

    //注册回调
    virtual void RgsUpdate(table_update pCallBack, const char * debug) {
        m_oUpdateCallPool.push_back(pCallBack);
    }

    virtual void RgsAdd(table_add pCallBack, const char * debug) {
        m_oAddCallPool.push_back(pCallBack);
    }

    virtual void RgsDelete(table_delete pCallBack, const char * debug) {
        m_oDelCallPool.push_back(pCallBack);
    }

    virtual void RgsChange(table_change pCallBack, const char * debug) {
        m_oChangeCallPool.push_back(pCallBack);
    }

    virtual bool IsShadow() const {
        return m_oColumnInfo.shadow;
    }

    void UpdateCallBack(IKernel * pKernel, ITableControl * pTable, const s32 row, const s32 column, const void * pValue, const s32 size, const s8 type) {
        UPDATE_CB_POOL::iterator itor = m_oUpdateCallPool.begin();
        UPDATE_CB_POOL::iterator iend = m_oUpdateCallPool.end();
        while (itor != iend) {
            (*itor)(pKernel, pTable, m_oColumnInfo.shadow, row, column, pValue, size, type);
            itor++;
        }
    }

    void AddCallBack(IKernel * pKernel, ITableControl * pTable, const s32 row, const void * pKey, const s32 size, const s8 type) {
        ADD_CB_POOL::iterator itor = m_oAddCallPool.begin();
        ADD_CB_POOL::iterator iend = m_oAddCallPool.end();
        while (itor != iend) {
            (*itor)(pKernel, pTable, m_oColumnInfo.shadow, row, pKey, size, type);
            itor++;
        }
    }

    void DeleteCallBack(IKernel * pKernel, ITableControl * pTable, const s32 row) {
        DELETE_CB_POOL::iterator itor = m_oDelCallPool.begin();
        DELETE_CB_POOL::iterator iend = m_oDelCallPool.end();
        while (itor != iend) {
            (*itor)(pKernel, pTable, m_oColumnInfo.shadow, row);
            itor++;
        }

    }

    void ChangeCallBack(IKernel * pKernel, ITableControl * pTable, const s32 src, const s32 dst) {
        CHG_CB_POOL::iterator itor = m_oChangeCallPool.begin();
        CHG_CB_POOL::iterator iend = m_oChangeCallPool.end();
        while (itor != iend) {
            (*itor)(pKernel, pTable, m_oColumnInfo.shadow, src, dst);
            itor++;
        }
    }

    //初始化表结构(尽量在创建对象时初始化), 调用Forming生成表结构
    virtual bool AddColumnInt8(const ColumnIndex index, bool isKey);
    virtual bool AddColumnInt16(const ColumnIndex index, bool isKey);
    virtual bool AddColumnInt32(const ColumnIndex index, bool isKey);
    virtual bool AddColumnInt64(const ColumnIndex index, bool isKey);
    virtual bool AddColumnString(const ColumnIndex index, const s32 maxlen, bool isKey);
    virtual bool AddColumnFloat(const ColumnIndex index);
    virtual bool AddColumnStruct(const ColumnIndex index, const s32 size);
    virtual bool Forming();

    //数据控制接口
    //通过key值查找行, 未找到返回-1
    virtual RowIndex FindRow(const s64 key) const;
    virtual RowIndex FindRow(const char * key) const;

    //获取行数
    virtual s32 RowCount() const;
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
    TableControl(const char * name, IObject * pHost = NULL)
        : m_oTableName(name),
        m_pHost(pHost),
        m_bIsInited(false)
    {
        m_oColumnInfo.key.index = -1;
        m_oColumnInfo.key.type = DTYPE_CANT_BE_KEY;
        OASSERT(m_oRows.size() == 0, "wtf");
    }

    ~TableControl() {
        Clear();
    }

    bool ChangeKey(const s64 oldKey, const s64 newKey, const s8 type);
    bool ChangeKey(const char * oldKey, const char * newKey, const s8 type);
    void OrderProcIndex(const RowIndex index, const s32 index_diff); //改变index后面的行的index值

    static unordered_map<TableControl *, TableCreateInfo> s_leakMap;

private:
    IObject * m_pHost;

    UPDATE_CB_POOL m_oUpdateCallPool;
    ADD_CB_POOL m_oAddCallPool;
    DELETE_CB_POOL m_oDelCallPool;
    CHG_CB_POOL m_oChangeCallPool;

    tlib::TString<game::MAX_NAME_LEN> m_oTableName;
    TableColumnInfo m_oColumnInfo;

    KEY_INT_MAP m_oIntKeyMap;
    KEY_STRING_MAP m_oStringKeyMap;
    TABLE_ROWS m_oRows;

    bool m_bIsInited;

    friend TABLE_POOL;
    static TABLE_POOL s_oTablePool;
};

#endif //defined __TableControl_h__
