#include "TableRowPool.h"

TableRowPool::TableRowPool() : m_nTotalRowCount(0) {

}

TableRowPool::~TableRowPool() {
    TABLE_ROW_MAP::iterator iMapTor = m_oTablesPool.begin();
    TABLE_ROW_MAP::iterator iMapEnd = m_oTablesPool.end();
    while (iMapTor != iMapEnd) {
        TABLE_ROW_LIST::iterator itor = iMapTor->second.begin();
        while (itor != iMapTor->second.end()) {
            m_nTotalRowCount --;
            DEL *itor;
            itor = iMapTor->second.erase(itor);
        }

        iMapTor ++;
    }
    
    OASSERT(m_nTotalRowCount == 0, "wtf");
}

TableRow * TableRowPool::Create(const TableColumnInfo * pTableColumnInfo) {
    s32 nSize = pTableColumnInfo->CalcMemorySize();
    OASSERT(nSize > 0, "wtf");
    TABLE_ROW_MAP::iterator itor = m_oTablesPool.find(nSize);
    if (itor == m_oTablesPool.end() || itor->second.empty()) {
        m_nTotalRowCount ++;
        return NEW TableRow(pTableColumnInfo);
    }

    TableRow * pRow = *(itor->second.begin());
    itor->second.erase(itor->second.begin());
    pRow->SetColumnInfo(pTableColumnInfo);
    return pRow;
}

void TableRowPool::Recover(TableRow * pTable) {
    TABLE_ROW_MAP::iterator itor = m_oTablesPool.find(pTable->GetSize());
    if (itor == m_oTablesPool.end()) {
        m_oTablesPool.insert(make_pair(pTable->GetSize(), TABLE_ROW_LIST()));
        itor = m_oTablesPool.find(pTable->GetSize());
    }
    itor->second.push_back(pTable);
}
