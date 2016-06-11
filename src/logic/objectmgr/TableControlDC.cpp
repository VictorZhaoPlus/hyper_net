#include "TableControl.h"

s32 TableControl::RowCount() const{
    return m_oRows.size();
}

//数据控制接口
RowIndex TableControl::FindRow(const s64 key) const {
    OASSERT(m_oColumnInfo.key.type == DTYPE_INT8
        || m_oColumnInfo.key.type == DTYPE_INT16
        || m_oColumnInfo.key.type == DTYPE_INT32
        || m_oColumnInfo.key.type == DTYPE_INT64, "WTF");
    RowIndex index = -1;
    if (m_oColumnInfo.key.type != DTYPE_STRING && m_oColumnInfo.key.type < DTYPE_CANT_BE_KEY) {
        KEY_INT_MAP::const_iterator itor = m_oIntKeyMap.find(key);
        if (itor != m_oIntKeyMap.end()) {
            index = itor->second;
        }
    }

    return index;
}

RowIndex TableControl::FindRow(const char * key) const {
    OASSERT(m_oColumnInfo.key.type == DTYPE_STRING, "WTF");
    RowIndex index = -1;
    if (m_oColumnInfo.key.type == DTYPE_STRING) {
        KEY_STRING_MAP::const_iterator itor = m_oStringKeyMap.find(key);
        if (itor != m_oStringKeyMap.end()) {
            index = itor->second;
        }
    }

    return index;
}


RowIndex TableControl::AddRow() {
    OASSERT(m_oColumnInfo.key.type == DTYPE_CANT_BE_KEY, "wtf");
    RowIndex index = -1;
    if (m_oColumnInfo.key.type == DTYPE_CANT_BE_KEY) {
        index = m_oRows.size();
        TableRow * pRow = TableRowPool::GetInterface()->Create(&m_oColumnInfo);
        m_oRows.push_back(pRow);
        AddCallBack(g_pKernel, this, index, nullptr, 0, DTYPE_CANT_BE_KEY);
    }
    return index;
}

RowIndex TableControl::AddRowKeyInt8(const s8 key) {
    OASSERT(m_oColumnInfo.key.type == DTYPE_INT8, "wtf");
    RowIndex index = -1;
    if (m_oColumnInfo.key.type == DTYPE_INT8) {
        if (m_oIntKeyMap.find(key) != m_oIntKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return index;
        }

        index = m_oRows.size();
        TableRow * pRow = TableRowPool::GetInterface()->Create(&m_oColumnInfo);
        m_oIntKeyMap.insert(make_pair(key, index));
        pRow->SetValue(m_oColumnInfo.key.index, DTYPE_INT8, &key, sizeof(key));
        m_oRows.push_back(pRow);

        AddCallBack(g_pKernel, this, index, &key, sizeof(key), DTYPE_INT8);
    }

    return index;
}

RowIndex TableControl::AddRowKeyInt16(const s16 key) {
    OASSERT(m_oColumnInfo.key.type == DTYPE_INT16, "wtf");
    RowIndex index = -1;
    if (m_oColumnInfo.key.type == DTYPE_INT16) {
        if (m_oIntKeyMap.find(key) != m_oIntKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return index;
        }

        index = m_oRows.size();
        TableRow * pRow = TableRowPool::GetInterface()->Create(&m_oColumnInfo);
        m_oIntKeyMap.insert(make_pair(key, index));
        pRow->SetValue(m_oColumnInfo.key.index, DTYPE_INT16, &key, sizeof(key));
        m_oRows.push_back(pRow);

        AddCallBack(g_pKernel, this, index, &key, sizeof(key), DTYPE_INT16);
    }

    return index;
}

RowIndex TableControl::AddRowKeyInt32(const s32 key) {
    OASSERT(m_oColumnInfo.key.type == DTYPE_INT32, "wtf");
    RowIndex index = -1;
    if (m_oColumnInfo.key.type == DTYPE_INT32) {
        if (m_oIntKeyMap.find(key) != m_oIntKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return index;
        }

        index = m_oRows.size();
        TableRow * pRow = TableRowPool::GetInterface()->Create(&m_oColumnInfo);
        m_oIntKeyMap.insert(make_pair(key, index));
        pRow->SetValue(m_oColumnInfo.key.index, DTYPE_INT32, &key, sizeof(key));
        m_oRows.push_back(pRow);

        AddCallBack(g_pKernel, this, index, &key, sizeof(key), DTYPE_INT32);
    }

    return index;
}

RowIndex TableControl::AddRowKeyInt64(const s64 key) {
    OASSERT(m_oColumnInfo.key.type == DTYPE_INT64, "wtf");
    RowIndex index = -1;
    if (m_oColumnInfo.key.type == DTYPE_INT64) {
        if (m_oIntKeyMap.find(key) != m_oIntKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return index;
        }

        index = m_oRows.size();
        TableRow * pRow = TableRowPool::GetInterface()->Create(&m_oColumnInfo);
        m_oIntKeyMap.insert(make_pair(key, index));
        pRow->SetValue(m_oColumnInfo.key.index, DTYPE_INT64, &key, sizeof(key));
        m_oRows.push_back(pRow);

        AddCallBack(g_pKernel, this, index, &key, sizeof(key), DTYPE_INT64);
    }

    return index;
}

RowIndex TableControl::AddRowKeyString(const char * key) {
    OASSERT(m_oColumnInfo.key.type == DTYPE_STRING, "wtf");
    RowIndex index = -1;
    if (m_oColumnInfo.key.type == DTYPE_STRING) {
        if (m_oStringKeyMap.find(key) != m_oStringKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return index;
        }

        index = m_oRows.size();
        TableRow * pRow = TableRowPool::GetInterface()->Create(&m_oColumnInfo);
        m_oStringKeyMap.insert(make_pair(key, index));
        pRow->SetValue(m_oColumnInfo.key.index, DTYPE_STRING, key, strlen(key));
        m_oRows.push_back(pRow);

        AddCallBack(g_pKernel, this, index, key, strlen(key) + 1, DTYPE_STRING);
    }

    return index;
}

//插入行
bool TableControl::InsertRow(const RowIndex index) {
    OASSERT(index < m_oRows.size() && index >= 0 && m_oColumnInfo.key.type == DTYPE_CANT_BE_KEY, "index over flow");
    if (index >= m_oRows.size() || index < 0 || m_oColumnInfo.key.type != DTYPE_CANT_BE_KEY) {
        return false;
    }

    m_oRows.insert(m_oRows.begin() + index, TableRowPool::GetInterface()->Create(&m_oColumnInfo));
    AddCallBack(g_pKernel, this, index, nullptr, 0, DTYPE_CANT_BE_KEY);
    return true;
}

bool TableControl::InsertRowKeyInt8(const s8 key, const RowIndex index) {
    OASSERT(index < m_oRows.size() && index >= 0 && m_oColumnInfo.key.type == DTYPE_INT8, "index over flow");
    if (index >= m_oRows.size() || index < 0 || m_oColumnInfo.key.type != DTYPE_INT8) {
        return false;
    }

    if (m_oColumnInfo.key.type == DTYPE_INT8) {
        if (m_oIntKeyMap.find(key) != m_oIntKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return false;
        }
        TableRow * pRow = TableRowPool::GetInterface()->Create(&m_oColumnInfo);
        pRow->SetValue(m_oColumnInfo.key.index, DTYPE_INT8, &key, sizeof(key));
        m_oRows.insert(m_oRows.begin() + index, pRow);
        OrderProcIndex(index, 1);
        m_oIntKeyMap.insert(make_pair(key, index));

        AddCallBack(g_pKernel, this, index, &key, sizeof(key), DTYPE_INT8);
        return true;
    } 
    return false;
}

bool TableControl::InsertRowKeyInt16(const s16 key, const RowIndex index) {
    OASSERT(index < m_oRows.size() && index >= 0 && m_oColumnInfo.key.type == DTYPE_INT16, "index over flow");
    if (index >= m_oRows.size() || index < 0 || m_oColumnInfo.key.type != DTYPE_INT16) {
        return false;
    }

    if (m_oColumnInfo.key.type == DTYPE_INT16) {
        if (m_oIntKeyMap.find(key) != m_oIntKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return false;
        }

        TableRow * pRow = TableRowPool::GetInterface()->Create(&m_oColumnInfo);
        pRow->SetValue(m_oColumnInfo.key.index, DTYPE_INT16, &key, sizeof(key));
        m_oRows.insert(m_oRows.begin() + index, pRow);
        OrderProcIndex(index, 1);
        m_oIntKeyMap.insert(make_pair(key, index));
        AddCallBack(g_pKernel, this, index, &key, sizeof(key), DTYPE_INT16);
        return true;
    }
    return false;
}

bool TableControl::InsertRowKeyInt32(const s32 key, const RowIndex index) {
    OASSERT(index < m_oRows.size() && index >= 0 && m_oColumnInfo.key.type == DTYPE_INT32, "index over flow");
    if (index >= m_oRows.size() || index < 0 || m_oColumnInfo.key.type != DTYPE_INT32) {
        return false;
    }

    if (m_oColumnInfo.key.type == DTYPE_INT32) {
        if (m_oIntKeyMap.find(key) != m_oIntKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return false;
        }

        TableRow * pRow = TableRowPool::GetInterface()->Create(&m_oColumnInfo);
        pRow->SetValue(m_oColumnInfo.key.index, DTYPE_INT32, &key, sizeof(key));
        m_oRows.insert(m_oRows.begin() + index, pRow);
        OrderProcIndex(index, 1);
        m_oIntKeyMap.insert(make_pair(key, index));
        AddCallBack(g_pKernel, this, index, &key, sizeof(key), DTYPE_INT32);
        return true;
    }
    return false;
}

bool TableControl::InsertRowKeyInt64(const s64 key, const RowIndex index) {
    OASSERT(index < m_oRows.size() && index >= 0 && m_oColumnInfo.key.type == DTYPE_INT64, "index over flow");
    if (index >= m_oRows.size() || index < 0 || m_oColumnInfo.key.type != DTYPE_INT64) {
        return false;
    }

    if (m_oColumnInfo.key.type == DTYPE_INT64) {
        if (m_oIntKeyMap.find(key) != m_oIntKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return false;
        }

        TableRow * pRow = TableRowPool::GetInterface()->Create(&m_oColumnInfo);
        pRow->SetValue(m_oColumnInfo.key.index, DTYPE_INT64, &key, sizeof(key));
        m_oRows.insert(m_oRows.begin() + index, pRow);
        OrderProcIndex(index, 1);
        m_oIntKeyMap.insert(make_pair(key, index));
        AddCallBack(g_pKernel, this, index, &key, sizeof(key), DTYPE_INT64);
        return true;
    }
    return false;
}

bool TableControl::InsertRowKeyString(const char * key, const RowIndex index) {
    OASSERT(index < m_oRows.size() && index >= 0 && m_oColumnInfo.key.type == DTYPE_STRING, "index over flow");
    if (index >= m_oRows.size() || index < 0 || m_oColumnInfo.key.type != DTYPE_STRING) {
        return false;
    }

    if (m_oColumnInfo.key.type == DTYPE_STRING) {
        if (m_oStringKeyMap.find(key) != m_oStringKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return false;
        }

        TableRow * pRow = TableRowPool::GetInterface()->Create(&m_oColumnInfo);
        pRow->SetValue(m_oColumnInfo.key.index, DTYPE_STRING, key, strlen(key));
        m_oRows.insert(m_oRows.begin() + index, pRow);
        OrderProcIndex(index, 1);
        m_oStringKeyMap.insert(make_pair(key, index));
        AddCallBack(g_pKernel, this, index, key, strlen(key) + 1, DTYPE_STRING);
        return true;
    }
    return false;
}

void TableControl::OrderProcIndex(const RowIndex index, const s32 index_diff) {
    switch (m_oColumnInfo.key.type) {
	case DTYPE_INT8:
		{
			for (s32 i = index + 1; i < m_oRows.size(); i++) {
				s32 nSize = 0;
				s8 key = *(s8 *)m_oRows[i]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = m_oIntKeyMap.find(key);
				OASSERT(itor != m_oIntKeyMap.end(), "where is key");
				if (itor != m_oIntKeyMap.end()) {
					s32 oldIndex = itor->second;
                    itor->second += index_diff;
					OM_TRACE("table %s row %lld index %d changed to %d", m_oTableName.GetString(), key, oldIndex, itor->second);
				}
			}
		}
		break;
	case DTYPE_INT16:
		{
			for (s32 i = index + 1; i < m_oRows.size(); i++) {
				s32 nSize = 0;
				s16 key = *(s16 *)m_oRows[i]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = m_oIntKeyMap.find(key);
				OASSERT(itor != m_oIntKeyMap.end(), "where is key");
				if (itor != m_oIntKeyMap.end()) {
					s32 oldIndex = itor->second;
                    itor->second += index_diff;
					OM_TRACE("table %s row %lld index %d changed to %d", m_oTableName.GetString(), key, oldIndex, itor->second);
				}
			}
		}
		break;
	case DTYPE_INT32:
		{
			for (s32 i = index + 1; i < m_oRows.size(); i++) {
				s32 nSize = 0;
				s32 key = *(s32 *)m_oRows[i]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = m_oIntKeyMap.find(key);
				OASSERT(itor != m_oIntKeyMap.end(), "where is key");
				if (itor != m_oIntKeyMap.end()) {
					s32 oldIndex = itor->second;
                    itor->second += index_diff;
					OM_TRACE("table %s row %lld index %d changed to %d", m_oTableName.GetString(), key, oldIndex, itor->second);
				}
			}
		}
		break;
	case DTYPE_INT64:
		{
			for (s32 i = index + 1; i < m_oRows.size(); i++) {
				s32 nSize = 0;
				s64 key = *(s64 *)m_oRows[i]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = m_oIntKeyMap.find(key);
				OASSERT(itor != m_oIntKeyMap.end(), "where is key");
				if (itor != m_oIntKeyMap.end()) {
					s32 oldIndex = itor->second;
                    itor->second += index_diff;
					OM_TRACE("table %s row %lld index %d changed to %d", m_oTableName.GetString(), key, oldIndex, itor->second);
				}
			}
		}
		break;
	case DTYPE_STRING:
		{
			for (s32 i = index + 1; i < m_oRows.size(); i++) {
				s32 nSize = 0;
				const char * key = (const char *)m_oRows[i]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
				KEY_STRING_MAP::iterator itor = m_oStringKeyMap.find(key);
				OASSERT(itor != m_oStringKeyMap.end(), "where is key");
				if (itor != m_oStringKeyMap.end()) {
					s32 oldIndex = itor->second;
                    itor->second += index_diff;
					OM_TRACE("table %s row %s index %d changed to %d", m_oTableName.GetString(), key, oldIndex, itor->second);
				}
			}
		}
		break;
	case DTYPE_CANT_BE_KEY:
		break;
	default:
		OASSERT(false, "wtf");
		break;
	}
}

bool TableControl::DelRow(const RowIndex index) {
    OASSERT(index < m_oRows.size() && index >= 0, "index over flow");
    if (index >= m_oRows.size() || index < 0) {
        return false;
    }
    
    switch (m_oColumnInfo.key.type) {
	case DTYPE_INT8:
		{
			s32 nSize = 0;
			s8 key = *(s8 *)m_oRows[index]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
			KEY_INT_MAP::iterator itor = m_oIntKeyMap.find(key);
			OASSERT(itor != m_oIntKeyMap.end(), "where is key");
			if (itor != m_oIntKeyMap.end()) {
				m_oIntKeyMap.erase(itor);
				//OM_TRACE("Table %s delete key %ld", m_oTableName.GetString(), key);

                DeleteCallBack(g_pKernel, this, index);
			}
			break;
		}
	case DTYPE_INT16:
		{
			s32 nSize = 0;
			s16 key = *(s16 *)m_oRows[index]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
			KEY_INT_MAP::iterator itor = m_oIntKeyMap.find(key);
			OASSERT(itor != m_oIntKeyMap.end(), "where is key");
			if (itor != m_oIntKeyMap.end()) {
				m_oIntKeyMap.erase(itor);
                //OM_TRACE("Table %s delete key %ld", m_oTableName.GetString(), key);
                DeleteCallBack(g_pKernel, this, index);
			}
			break;
		}
	case DTYPE_INT32:
		{
			s32 nSize = 0;
			s32 key = *(s32 *)m_oRows[index]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
			KEY_INT_MAP::iterator itor = m_oIntKeyMap.find(key);
			OASSERT(itor != m_oIntKeyMap.end(), "where is key");
			if (itor != m_oIntKeyMap.end()) {
				m_oIntKeyMap.erase(itor);
                //OM_TRACE("Table %s delete key %ld", m_oTableName.GetString(), key);
                DeleteCallBack(g_pKernel, this, index);
			}
			break;
		}
    case DTYPE_INT64:
        {
            s32 nSize = 0;
            s64 key = *(s64 *)m_oRows[index]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
            KEY_INT_MAP::iterator itor = m_oIntKeyMap.find(key);
            OASSERT(itor != m_oIntKeyMap.end(), "where is key");
            if (itor != m_oIntKeyMap.end()) {
                m_oIntKeyMap.erase(itor);
                //OM_TRACE("Table %s delete key %ld", m_oTableName.GetString(), key);
                DeleteCallBack(g_pKernel, this, index);
            }
        }
        break;
    case DTYPE_STRING:
        {
            s32 nSize = 0;
            const char * key = (const char *)m_oRows[index]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
            KEY_STRING_MAP::iterator itor = m_oStringKeyMap.find(key);
            OASSERT(itor != m_oStringKeyMap.end(), "where is key");
            if (itor != m_oStringKeyMap.end()) {
                m_oStringKeyMap.erase(itor);
                //OM_TRACE("Table %s delete key %s", m_oTableName.GetString(), key);
                DeleteCallBack(g_pKernel, this, index);
            }
        }
        break;
    case DTYPE_CANT_BE_KEY:
        break;
    default:
        OASSERT(false, "wtf");
        break;
    }


    TABLE_ROWS::iterator itor = m_oRows.begin() + index;
    TableRowPool::GetInterface()->Recover(*itor);

    OrderProcIndex(index, -1);

    m_oRows.erase(itor);

    return true;
}

bool TableControl::SwapRowIndex(const RowIndex src, const RowIndex dst) {
    OASSERT(src < m_oRows.size() && dst < m_oRows.size(), "wtf");

    switch (m_oColumnInfo.key.type) {
		case DTYPE_INT8:
		{
			{
				s32 nSize = 0;
				s8 key = *(s8 *)m_oRows[src]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = m_oIntKeyMap.find(key);
				OASSERT(itor != m_oIntKeyMap.end(), "where is key");
				if (itor != m_oIntKeyMap.end()) {
					itor->second = dst;
					//OM_TRACE("table %s row %lld index %d changed to %d", m_oTableName.GetString(), key, src, itor->second);
				}
			}
			{
				s32 nSize = 0;
				s8 key = *(s8 *)m_oRows[dst]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = m_oIntKeyMap.find(key);
				OASSERT(itor != m_oIntKeyMap.end(), "where is key");
				if (itor != m_oIntKeyMap.end()) {
					itor->second = src;
					//OM_TRACE("table %s row %lld index %d changed to %d", m_oTableName.GetString(), key, dst, itor->second);
				}
			}
			break;
		}
		case DTYPE_INT16:
		{
			{
				s32 nSize = 0;
				s16 key = *(s16 *)m_oRows[src]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = m_oIntKeyMap.find(key);
				OASSERT(itor != m_oIntKeyMap.end(), "where is key");
				if (itor != m_oIntKeyMap.end()) {
					itor->second = dst;
					//OM_TRACE("table %s row %lld index %d changed to %d", m_oTableName.GetString(), key, src, itor->second);
				}
			}
			{
				s32 nSize = 0;
				s16 key = *(s16 *)m_oRows[dst]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = m_oIntKeyMap.find(key);
				OASSERT(itor != m_oIntKeyMap.end(), "where is key");
				if (itor != m_oIntKeyMap.end()) {
					itor->second = src;
					//OM_TRACE("table %s row %lld index %d changed to %d", m_oTableName.GetString(), key, dst, itor->second);
				}
			}
			break;
		}
		case DTYPE_INT32:
		{
			{
				s32 nSize = 0;
				s32 key = *(s32 *)m_oRows[src]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = m_oIntKeyMap.find(key);
				OASSERT(itor != m_oIntKeyMap.end(), "where is key");
				if (itor != m_oIntKeyMap.end()) {
					itor->second = dst;
				}
			}
			{
				s32 nSize = 0;
				s32 key = *(s32 *)m_oRows[dst]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = m_oIntKeyMap.find(key);
				OASSERT(itor != m_oIntKeyMap.end(), "where is key");
				if (itor != m_oIntKeyMap.end()) {
					itor->second = src;
				}
			}
			break;
		}
        case DTYPE_INT64:
        {
            {
                s32 nSize = 0;
                s64 key = *(s64 *)m_oRows[src]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
                KEY_INT_MAP::iterator itor = m_oIntKeyMap.find(key);
                OASSERT(itor != m_oIntKeyMap.end(), "where is key");
                if (itor != m_oIntKeyMap.end()) {
                    itor->second = dst;
                }
            }
            {
                s32 nSize = 0;
                s64 key = *(s64 *)m_oRows[dst]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
                KEY_INT_MAP::iterator itor = m_oIntKeyMap.find(key);
                OASSERT(itor != m_oIntKeyMap.end(), "where is key");
                if (itor != m_oIntKeyMap.end()) {
                    itor->second = src;
                }
            }
            break;
        }
        case DTYPE_STRING:
        {
            {
                s32 nSize = 0;
                const char * key = (const char *)m_oRows[src]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
                KEY_STRING_MAP::iterator itor = m_oStringKeyMap.find(key);
                OASSERT(itor != m_oStringKeyMap.end(), "where is key");
                if (itor != m_oStringKeyMap.end()) {
                    itor->second = dst;
                    //OM_TRACE("table %s row %s index %d changed to %d", m_oTableName.GetString(), key, src, itor->second);
                }
            }

            {
                s32 nSize = 0;
                const char * key = (const char *)m_oRows[dst]->GetValue(m_oColumnInfo.key.index, m_oColumnInfo.key.type, nSize);
                KEY_STRING_MAP::iterator itor = m_oStringKeyMap.find(key);
                OASSERT(itor != m_oStringKeyMap.end(), "where is key");
                if (itor != m_oStringKeyMap.end()) {
                    itor->second = src;
                    //OM_TRACE("table %s row %s index %d changed to %d", m_oTableName.GetString(), key, dst, itor->second);
                }
            }
            break;
        }
        case DTYPE_CANT_BE_KEY:
            break;
        default:
            OASSERT(false, "wtf");
            break;
    }

    TableRow * pSwap = m_oRows[src];
    m_oRows[src] = m_oRows[dst];
    m_oRows[dst] = pSwap;

    ChangeCallBack(g_pKernel, this, src, dst);
    return true;
}

/*========CHANGE KEY=========*/
bool TableControl::ChangeKey(const s64 oldKey, const s64 newKey, const s8 type) {
    OASSERT(type == DTYPE_INT8 || type == DTYPE_INT16
        ||type == DTYPE_INT32 || type == DTYPE_INT64, "wtf");

    OASSERT(m_oColumnInfo.key.type == type, "wtf");
    if (m_oColumnInfo.key.type != type) {
        return false;
    }

    KEY_INT_MAP::iterator iNewtor = m_oIntKeyMap.find(newKey);
    if (iNewtor != m_oIntKeyMap.end()) {
        OM_TRACE("table %s key %lld is exist", m_oTableName.GetString(), newKey);
        return false;
    }

    KEY_INT_MAP::iterator iOldtor = m_oIntKeyMap.find(oldKey);
    OASSERT(iOldtor != m_oIntKeyMap.end(), "where is old key");
    if (iOldtor == m_oIntKeyMap.end()) {
        OM_TRACE("table %s can't find key %lld", m_oTableName.GetString(), oldKey);
        return false;
    }
    m_oIntKeyMap.insert(make_pair(newKey, iOldtor->second));
    m_oIntKeyMap.erase(iOldtor);
    return false;
}

bool TableControl::ChangeKey(const char * oldKey, const char * newKey, const s8 type) {
    OASSERT(type == DTYPE_STRING && m_oColumnInfo.key.type == type, "wtf");

    if (m_oColumnInfo.key.type != type) {
        return false;
    }

    KEY_STRING_MAP::iterator iNewtor = m_oStringKeyMap.find(newKey);
    if (iNewtor != m_oStringKeyMap.end()) {
        OM_TRACE("table %s key %s is exist", m_oTableName.GetString(), newKey);
        return false;
    }

    KEY_STRING_MAP::iterator iOldtor = m_oStringKeyMap.find(oldKey);
    OASSERT(iOldtor != m_oStringKeyMap.end(), "where is old key");
    if (iOldtor == m_oStringKeyMap.end()) {
        OM_TRACE("table %s can't find key %s", m_oTableName.GetString(), oldKey);
        return false;
    }
    m_oStringKeyMap.insert(make_pair(newKey, iOldtor->second));
    m_oStringKeyMap.erase(iOldtor);
    return false;
}

bool TableControl::GetRowData(const s32 row, const void * & pData, s32 & size) {
    pData = m_oRows[row]->GetData();
    size = m_oRows[row]->GetSize();
    return true;
}

RowIndex TableControl::AddRowData(const void * pData, const s32 size) {
    TableRow * pRow = TableRowPool::GetInterface()->Create(&m_oColumnInfo);
    pRow->Copy(pData, size);
    s32 index = m_oRows.size();

    switch (m_oColumnInfo.key.type) {
    case DTYPE_INT8:
    {
                       s32 size = 0;
                       s8 key = *(s8*)pRow->GetValue(m_oColumnInfo.key.index, DTYPE_INT8, size);
                       OASSERT(size == sizeof(key), "wtf");
                       if (m_oIntKeyMap.find(key) != m_oIntKeyMap.end()) {
                           OASSERT(false, "key is already exits");
                           TableRowPool::GetInterface()->Recover(pRow);
                           return false;
                       }

                       m_oRows.push_back(pRow);
                       m_oIntKeyMap.insert(make_pair(key, index));
                       break;
    }
    case DTYPE_INT16:
    {
                        s32 size = 0;
                        s16 key = *(s16*)pRow->GetValue(m_oColumnInfo.key.index, DTYPE_INT16, size);
                        OASSERT(size == sizeof(key), "wtf");
                        if (m_oIntKeyMap.find(key) != m_oIntKeyMap.end()) {
                            OASSERT(false, "key is already exits");
                            TableRowPool::GetInterface()->Recover(pRow);
                            return false;
                        }

                        m_oRows.push_back(pRow);
                        m_oIntKeyMap.insert(make_pair(key, index));
                        break;
    }
    case DTYPE_INT32:
    {
                        s32 size = 0;
                        s32 key = *(s32*)pRow->GetValue(m_oColumnInfo.key.index, DTYPE_INT32, size);
                        OASSERT(size == sizeof(key), "wtf");
                        if (m_oIntKeyMap.find(key) != m_oIntKeyMap.end()) {
                            OASSERT(false, "key is already exits");
                            TableRowPool::GetInterface()->Recover(pRow);
                            return false;
                        }

                        m_oRows.push_back(pRow);
                        m_oIntKeyMap.insert(make_pair(key, index));
                        break;
    }
    case DTYPE_INT64:
    {
                        s32 size = 0;
                        s64 key = *(s64*)pRow->GetValue(m_oColumnInfo.key.index, DTYPE_INT64, size);
                        OASSERT(size == sizeof(key), "wtf");
                        if (m_oIntKeyMap.find(key) != m_oIntKeyMap.end()) {
                            OASSERT(false, "key is already exits");
                            TableRowPool::GetInterface()->Recover(pRow);
                            return false;
                        }

                        m_oRows.push_back(pRow);
                        m_oIntKeyMap.insert(make_pair(key, index));
                        break;
    }
    case DTYPE_STRING:
    {
                         s32 size = 0;
                         const char * key = (const char *)pRow->GetValue(m_oColumnInfo.key.index, DTYPE_STRING, size);
                         if (m_oStringKeyMap.find(key) != m_oStringKeyMap.end()) {
                             OASSERT(false, "key is already exits");
                             TableRowPool::GetInterface()->Recover(pRow);
                             return false;
                         }

                         m_oRows.push_back(pRow);
                         m_oStringKeyMap.insert(make_pair(key, index));
                         break;
    }
    default:
        break;
    }

    return index;
}

/*==============================================================================*/
s8 TableControl::GetDataInt8(const RowIndex rowIndex, const ColumnIndex clmIndex) const {
    OASSERT(rowIndex < m_oRows.size(), "row index over flow");
    if (rowIndex >= m_oRows.size()) {
        OM_TRACE("table %s, row index %d over flow, row count %d", m_oTableName.GetString(), rowIndex, m_oRows.size());
        return 0;
    }

    s32 nLen = 0;
    const void * pValue = m_oRows[rowIndex]->GetValue(clmIndex, DTYPE_INT8, nLen);
    OASSERT(nLen == sizeof(s8), "wtf");

    return *(s8 *)pValue;
}

bool TableControl::SetDataInt8(const RowIndex rowIndex, const ColumnIndex clmIndex, const s8 value) {
    OASSERT(rowIndex < m_oRows.size(), "row index over flow");
    if (rowIndex >= m_oRows.size()) {
        OM_TRACE("table %s, row index %d over flow, row count %d", m_oTableName.GetString(), rowIndex, m_oRows.size());
        return 0;
    }

    if (m_oColumnInfo.key.index == clmIndex) {
        OASSERT(false, "wtf");
        OASSERT(m_oColumnInfo.key.type == DTYPE_INT8, "key type error, check ur om logic");
        s8 oldValue = GetDataInt8(rowIndex, m_oColumnInfo.key.index);
        if (!ChangeKey(oldValue, value, DTYPE_INT8)) {
            return false;
        }

        OM_TRACE("table %s changer key %d to %d", m_oTableName.GetString(), oldValue, value);
    }

    if (m_oRows[rowIndex]->SetValue(clmIndex, DTYPE_INT8, &value, sizeof(value))) {
        UpdateCallBack(g_pKernel, this, rowIndex, clmIndex, &value, sizeof(value), DTYPE_INT8);
        return true;
    }

    return false;
}
/*==============================================================================*/
s16 TableControl::GetDataInt16(const RowIndex rowIndex, const ColumnIndex clmIndex) const {
    OASSERT(rowIndex < m_oRows.size(), "row index over flow");
    if (rowIndex >= m_oRows.size()) {
        OM_TRACE("table %s, row index %d over flow, row count %d", m_oTableName.GetString(), rowIndex, m_oRows.size());
        return 0;
    }

    s32 nLen = 0;
    const void * pValue = m_oRows[rowIndex]->GetValue(clmIndex, DTYPE_INT16, nLen);
    OASSERT(nLen == sizeof(s16), "wtf");
    return *(s16 *)pValue;
}

bool TableControl::SetDataInt16(const RowIndex rowIndex, const ColumnIndex clmIndex, const s16 value) {
    OASSERT(rowIndex < m_oRows.size(), "row index over flow");
    if (rowIndex >= m_oRows.size()) {
        OM_TRACE("table %s, row index %d over flow, row count %d", m_oTableName.GetString(), rowIndex, m_oRows.size());
        return 0;
    }

    if (m_oColumnInfo.key.index == clmIndex) {
        OASSERT(false, "wtf");
        OASSERT(m_oColumnInfo.key.type == DTYPE_INT16, "key type error, check ur om logic");
        s16 oldValue = GetDataInt16(rowIndex, m_oColumnInfo.key.index);
        if (!ChangeKey(oldValue, value, DTYPE_INT16)) {
            return false;
        }

        OM_TRACE("table %s changer key %d to %d", m_oTableName.GetString(), oldValue, value);
    }

    if (m_oRows[rowIndex]->SetValue(clmIndex, DTYPE_INT16, &value, sizeof(value))) {
        UpdateCallBack(g_pKernel, this, rowIndex, clmIndex, &value, sizeof(value), DTYPE_INT16);
        return true;
    }
    return false;
}

/*==============================================================================*/
s32 TableControl::GetDataInt32(const RowIndex rowIndex, const ColumnIndex clmIndex) const {
    OASSERT(rowIndex < m_oRows.size(), "row index over flow");
    if (rowIndex >= m_oRows.size()) {
        OM_TRACE("table %s, row index %d over flow, row count %d", m_oTableName.GetString(), rowIndex, m_oRows.size());
        return 0;
    }

    s32 nLen = 0;
    const void * pValue = m_oRows[rowIndex]->GetValue(clmIndex, DTYPE_INT32, nLen);
    OASSERT(nLen == sizeof(s32), "wtf");
    return *(s32 *)pValue;
}

bool TableControl::SetDataInt32(const RowIndex rowIndex, const ColumnIndex clmIndex, const s32 value) {
    OASSERT(rowIndex < m_oRows.size(), "row index over flow");
    if (rowIndex >= m_oRows.size()) {
        OM_TRACE("table %s, row index %d over flow, row count %d", m_oTableName.GetString(), rowIndex, m_oRows.size());
        return 0;
    }

    if (m_oColumnInfo.key.index == clmIndex) {
        OASSERT(false, "wtf");
        OASSERT(m_oColumnInfo.key.type == DTYPE_INT32, "key type error, check ur om logic");
        s32 oldValue = GetDataInt32(rowIndex, m_oColumnInfo.key.index);
        if (!ChangeKey(oldValue, value, DTYPE_INT32)) {
            return false;
        }

        OM_TRACE("table %s changer key %d to %d", m_oTableName.GetString(), oldValue, value);
    }

    if (m_oRows[rowIndex]->SetValue(clmIndex, DTYPE_INT32, &value, sizeof(value))) {
        UpdateCallBack(g_pKernel, this, rowIndex, clmIndex, &value, sizeof(value), DTYPE_INT32);
        return true;
    }
    return false;
}
/*==============================================================================*/
s64 TableControl::GetDataInt64(const RowIndex rowIndex, const ColumnIndex clmIndex) const {
    OASSERT(rowIndex < m_oRows.size(), "row index over flow");
    if (rowIndex >= m_oRows.size()) {
        OM_TRACE("table %s, row index %d over flow, row count %d", m_oTableName.GetString(), rowIndex, m_oRows.size());
        return 0;
    }

    s32 nLen = 0;
    const void * pValue = m_oRows[rowIndex]->GetValue(clmIndex, DTYPE_INT64, nLen);
    OASSERT(nLen == sizeof(s64), "wtf");
    return *(s64 *)pValue;
}

bool TableControl::SetDataInt64(const RowIndex rowIndex, const ColumnIndex clmIndex, const s64 value) {
    OASSERT(rowIndex < m_oRows.size(), "row index over flow");
    if (rowIndex >= m_oRows.size()) {
        OM_TRACE("table %s, row index %d over flow, row count %d", m_oTableName.GetString(), rowIndex, m_oRows.size());
        return 0;
    }
    if (m_oColumnInfo.key.index == clmIndex) {
        OASSERT(false, "wtf");
        OASSERT(m_oColumnInfo.key.type == DTYPE_INT64, "key type error, check ur om logic");
        s64 oldValue = GetDataInt64(rowIndex, m_oColumnInfo.key.index);
        if (!ChangeKey(oldValue, value, DTYPE_INT64)) {
            return false;
        }

        OM_TRACE("table %s changer key %d to %d", m_oTableName.GetString(), oldValue, value);
    }

    if (m_oRows[rowIndex]->SetValue(clmIndex, DTYPE_INT64, &value, sizeof(value))) {
        UpdateCallBack(g_pKernel, this, rowIndex, clmIndex, &value, sizeof(value), DTYPE_INT64);
        return true;
    }
    return false;
}
/*==============================================================================*/
float TableControl::GetDataFloat(const RowIndex rowIndex, const ColumnIndex clmIndex) const {
    OASSERT(rowIndex < m_oRows.size(), "row index over flow");
    if (rowIndex >= m_oRows.size()) {
        OM_TRACE("table %s, row index %d over flow, row count %d", m_oTableName.GetString(), rowIndex, m_oRows.size());
        return 0;
    }    
    
    s32 nLen = 0;
    const void * pValue = m_oRows[rowIndex]->GetValue(clmIndex, DTYPE_FLOAT, nLen);
    OASSERT(nLen == sizeof(float), "wtf");
    return *(float *)pValue;
}

bool TableControl::SetDataFloat(const RowIndex rowIndex, const ColumnIndex clmIndex, const float value) {
    OASSERT(rowIndex < m_oRows.size(), "row index over flow");
    if (rowIndex >= m_oRows.size()) {
        OM_TRACE("table %s, row index %d over flow, row count %d", m_oTableName.GetString(), rowIndex, m_oRows.size());
        return 0;
    }

    if (m_oRows[rowIndex]->SetValue(clmIndex, DTYPE_FLOAT, &value, sizeof(value))) {
        UpdateCallBack(g_pKernel, this, rowIndex, clmIndex, &value, sizeof(value), DTYPE_FLOAT);
        return true;
    }
    return false;
}

/*==============================================================================*/
const char * TableControl::GetDataString(const RowIndex rowIndex, const ColumnIndex clmIndex) const {
    OASSERT(rowIndex < m_oRows.size(), "row index over flow");
    if (rowIndex >= m_oRows.size()) {
        OM_TRACE("table %s, row index %d over flow, row count %d", m_oTableName.GetString(), rowIndex, m_oRows.size());
        return 0;
    }

    s32 nLen = 0;
    const void * pValue = m_oRows[rowIndex]->GetValue(clmIndex, DTYPE_STRING, nLen);
    return (const char *)pValue;
}

bool TableControl::SetDataString(const RowIndex rowIndex, const ColumnIndex clmIndex, const char * value) {
    OASSERT(rowIndex < m_oRows.size(), "row index over flow");
    if (rowIndex >= m_oRows.size()) {
        OM_TRACE("table %s, row index %d over flow, row count %d", m_oTableName.GetString(), rowIndex, m_oRows.size());
        return 0;
    }
    if (m_oColumnInfo.key.index == clmIndex) {
        OASSERT(false, "wtf");
        OASSERT(m_oColumnInfo.key.type == DTYPE_STRING, "key type error, check ur om logic");
        const char * oldValue = GetDataString(rowIndex, m_oColumnInfo.key.index);
        if (!ChangeKey(oldValue, value, DTYPE_STRING)) {
            return false;
        }

        OM_TRACE("table %s changer key %s to %s", m_oTableName.GetString(), oldValue, value);
    }

    if (m_oRows[rowIndex]->SetValue(clmIndex, DTYPE_STRING, value, strlen(value) + 1)) {
        UpdateCallBack(g_pKernel, this, rowIndex, clmIndex, value, strlen(value) + 1, DTYPE_STRING);
        return true;
    }
    return false;
}

/*==============================================================================*/
const void * TableControl::GetDataStruct(const RowIndex rowIndex, const ColumnIndex clmIndex, const s32 size) const {
    OASSERT(rowIndex < m_oRows.size(), "row index over flow");
    if (rowIndex >= m_oRows.size()) {
        OM_TRACE("table %s, row index %d over flow, row count %d", m_oTableName.GetString(), rowIndex, m_oRows.size());
        return 0;
    }

    s32 nLen = 0;
    const void * pValue = m_oRows[rowIndex]->GetValue(clmIndex, DTYPE_STRUCT, nLen);
    OASSERT(nLen == size, "wtf");
    return pValue;
}

bool TableControl::SetDataStruct(const RowIndex rowIndex, const ColumnIndex clmIndex, const void * pValue, const s32 size) {
    OASSERT(rowIndex < m_oRows.size(), "row index over flow");
    if (rowIndex >= m_oRows.size()) {
        OM_TRACE("table %s, row index %d over flow, row count %d", m_oTableName.GetString(), rowIndex, m_oRows.size());
        return 0;
    }

    if (m_oRows[rowIndex]->SetValue(clmIndex, DTYPE_STRUCT, pValue, size)) {
        UpdateCallBack(g_pKernel, this, rowIndex, clmIndex, pValue, size, DTYPE_STRUCT);
        return true;
    }
    return false;
}

/*==============================================================================*/
