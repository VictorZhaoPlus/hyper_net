#include "TableControl.h"

TableControl::TABLE_POOL TableControl::s_oTablePool;
std::unordered_map<TableControl *, TableControl::TableCreateInfo> TableControl::s_leakMap;

//��ʼ����ṹ(�����ڴ�������ʱ��ʼ��), ����Forming���ɱ�ṹ
bool TableControl::FormingWithTableInfo(const TableColumnInfo * pColumnInfo){
    OASSERT(!_isInited, "table has inited");
    if (_isInited) {
        return false;
    }

    _columnInfo.Copy(*pColumnInfo);
    return true;
}
