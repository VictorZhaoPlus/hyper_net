#include "TableControl.h"

TableControl::TABLE_POOL TableControl::s_oTablePool;
unordered_map<TableControl *, TableControl::TableCreateInfo> TableControl::s_leakMap;

//初始化表结构(尽量在创建对象时初始化), 调用Forming生成表结构
bool TableControl::AddColumnInt8(const ColumnIndex index, bool isKey) {
    OASSERT(!m_bIsInited, "table has inited");
    if (m_bIsInited) {
        OM_TRACE("Add Column error, table has inited");
        return false;
    }
    
    return m_oColumnInfo.AddColumnInt8(index, isKey);
}

bool TableControl::AddColumnInt16(const ColumnIndex index, bool isKey) {
    OASSERT(!m_bIsInited, "table has inited");
    if (m_bIsInited) {
        OM_TRACE("Add Column error, table has inited");
        return false;
    }
    return m_oColumnInfo.AddColumnInt16(index, isKey);
}

bool TableControl::AddColumnInt32(const ColumnIndex index, bool isKey) {
    OASSERT(!m_bIsInited, "table has inited");
    if (m_bIsInited) {
        OM_TRACE("Add Column error, table has inited");
        return false;
    }
    return m_oColumnInfo.AddColumnInt32(index, isKey);
}

bool TableControl::AddColumnInt64(const ColumnIndex index, bool isKey) {
    OASSERT(!m_bIsInited, "table has inited");
    if (m_bIsInited) {
        OM_TRACE("Add Column error, table has inited");
        return false;
    }
    return m_oColumnInfo.AddColumnInt64(index, isKey);
}

bool TableControl::AddColumnString(const ColumnIndex index, const s32 maxlen, bool isKey) {
    OASSERT(!m_bIsInited, "table has inited");
    if (m_bIsInited) {
        OM_TRACE("Add Column error, table has inited");
        return false;
    }
    return m_oColumnInfo.AddColumnString(index, maxlen, isKey);
}

bool TableControl::AddColumnFloat(const ColumnIndex index) {
    OASSERT(!m_bIsInited, "table has inited");
    if (m_bIsInited) {
        OM_TRACE("Add Column error, table has inited");
        return false;
    }
    return m_oColumnInfo.AddColumnFloat(index);
}

bool TableControl::AddColumnStruct(const ColumnIndex index, const s32 size) {
    OASSERT(!m_bIsInited, "table has inited");
    if (m_bIsInited) {
        OM_TRACE("Add Column error, table has inited");
        return false;
    }
    return m_oColumnInfo.AddColumnStruct(index, size);
}

bool TableControl::Forming() {
    OASSERT(!m_bIsInited, "table has inited");
    if (m_bIsInited) {
        return false;
    }
    m_bIsInited = true;
    return true;
}

bool TableControl::FormingWithTableInfo(const TableColumnInfo * pColumnInfo){
    OASSERT(!m_bIsInited, "table has inited");
    if (m_bIsInited) {
        return false;
    }

    m_oColumnInfo.Copy(*pColumnInfo);
    return true;
}
