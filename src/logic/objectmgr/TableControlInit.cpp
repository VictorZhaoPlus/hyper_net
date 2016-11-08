#include "TableControl.h"

TableControl::TABLE_POOL TableControl::s_oTablePool;
std::unordered_map<TableControl *, TableControl::TableCreateInfo> TableControl::s_leakMap;

//初始化表结构(尽量在创建对象时初始化), 调用Forming生成表结构
bool TableControl::FormingWithTableInfo(const TableColumnInfo * pColumnInfo){
    OASSERT(!_isInited, "table has inited");
    if (_isInited) {
        return false;
    }

    _columnInfo.Copy(*pColumnInfo);
    return true;
}
