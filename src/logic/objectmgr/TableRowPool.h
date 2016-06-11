/* 
 * File:   TableRowPool.h
 * Author: Alax
 *
 * Created on March 3, 2015, 10:46 AM
 */

#include "TableStruct.h"
#include <list>
class TableRowPool {
    typedef std::list<TableRow *> TABLE_ROW_LIST;
    typedef std::unordered_map<s32, TABLE_ROW_LIST> TABLE_ROW_MAP;
public:
    static TableRowPool * GetInterface() {
        static TableRowPool * s_pSelf = NULL;
        if (NULL == s_pSelf) {
            s_pSelf = NEW TableRowPool;
        }

        OASSERT(s_pSelf, "wtf");
        return s_pSelf;
    }

    void Release() {
        DEL this;
    }

    TableRow * Create(const TableColumnInfo * pTableColumnInfo);
    void Recover(TableRow * pTable);

private:
    TableRowPool();
    ~TableRowPool();

private:
    TABLE_ROW_MAP m_oTablesPool;
    s32 m_nTotalRowCount;
};
