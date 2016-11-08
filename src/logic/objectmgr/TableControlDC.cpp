#include "TableControl.h"
#include "ObjectMgr.h"

//数据控制接口
RowIndex TableControl::FindRow(const s64 key) const {
    OASSERT(_columnInfo.key.type == DTYPE_INT8
        || _columnInfo.key.type == DTYPE_INT16
        || _columnInfo.key.type == DTYPE_INT32
        || _columnInfo.key.type == DTYPE_INT64, "WTF");
    RowIndex index = -1;
    if (_columnInfo.key.type != DTYPE_STRING && _columnInfo.key.type < DTYPE_CANT_BE_KEY) {
        KEY_INT_MAP::const_iterator itor = _intKeyMap.find(key);
        if (itor != _intKeyMap.end()) {
            index = itor->second;
        }
    }

    return index;
}

RowIndex TableControl::FindRow(const char * key) const {
    OASSERT(_columnInfo.key.type == DTYPE_STRING, "WTF");
    RowIndex index = -1;
    if (_columnInfo.key.type == DTYPE_STRING) {
        KEY_STRING_MAP::const_iterator itor = _stringKeyMap.find(key);
        if (itor != _stringKeyMap.end()) {
            index = itor->second;
        }
    }

    return index;
}


RowIndex TableControl::AddRow() {
    OASSERT(_columnInfo.key.type == DTYPE_CANT_BE_KEY, "wtf");
    RowIndex index = -1;
    if (_columnInfo.key.type == DTYPE_CANT_BE_KEY) {
        index = _rows.size();
        TableRow * pRow = TableRowPool::GetInterface()->Create(&_columnInfo);
        _rows.push_back(pRow);
        AddCallBack(ObjectMgr::Instance()->GetKernel(), this, index, nullptr, 0, DTYPE_CANT_BE_KEY);
    }
    return index;
}

RowIndex TableControl::AddRowKeyInt8(const s8 key) {
    OASSERT(_columnInfo.key.type == DTYPE_INT8, "wtf");
    RowIndex index = -1;
    if (_columnInfo.key.type == DTYPE_INT8) {
        if (_intKeyMap.find(key) != _intKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return index;
        }

        index = _rows.size();
        TableRow * pRow = TableRowPool::GetInterface()->Create(&_columnInfo);
        _intKeyMap.insert(std::make_pair(key, index));
        pRow->SetValue(_columnInfo.key.index, DTYPE_INT8, &key, sizeof(key));
        _rows.push_back(pRow);

        AddCallBack(ObjectMgr::Instance()->GetKernel(), this, index, &key, sizeof(key), DTYPE_INT8);
    }

    return index;
}

RowIndex TableControl::AddRowKeyInt16(const s16 key) {
    OASSERT(_columnInfo.key.type == DTYPE_INT16, "wtf");
    RowIndex index = -1;
    if (_columnInfo.key.type == DTYPE_INT16) {
        if (_intKeyMap.find(key) != _intKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return index;
        }

        index = _rows.size();
        TableRow * pRow = TableRowPool::GetInterface()->Create(&_columnInfo);
        _intKeyMap.insert(std::make_pair(key, index));
        pRow->SetValue(_columnInfo.key.index, DTYPE_INT16, &key, sizeof(key));
        _rows.push_back(pRow);

        AddCallBack(ObjectMgr::Instance()->GetKernel(), this, index, &key, sizeof(key), DTYPE_INT16);
    }

    return index;
}

RowIndex TableControl::AddRowKeyInt32(const s32 key) {
    OASSERT(_columnInfo.key.type == DTYPE_INT32, "wtf");
    RowIndex index = -1;
    if (_columnInfo.key.type == DTYPE_INT32) {
        if (_intKeyMap.find(key) != _intKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return index;
        }

        index = _rows.size();
        TableRow * pRow = TableRowPool::GetInterface()->Create(&_columnInfo);
        _intKeyMap.insert(std::make_pair(key, index));
        pRow->SetValue(_columnInfo.key.index, DTYPE_INT32, &key, sizeof(key));
        _rows.push_back(pRow);

        AddCallBack(ObjectMgr::Instance()->GetKernel(), this, index, &key, sizeof(key), DTYPE_INT32);
    }

    return index;
}

RowIndex TableControl::AddRowKeyInt64(const s64 key) {
    OASSERT(_columnInfo.key.type == DTYPE_INT64, "wtf");
    RowIndex index = -1;
    if (_columnInfo.key.type == DTYPE_INT64) {
        if (_intKeyMap.find(key) != _intKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return index;
        }

        index = _rows.size();
        TableRow * pRow = TableRowPool::GetInterface()->Create(&_columnInfo);
        _intKeyMap.insert(std::make_pair(key, index));
        pRow->SetValue(_columnInfo.key.index, DTYPE_INT64, &key, sizeof(key));
        _rows.push_back(pRow);

        AddCallBack(ObjectMgr::Instance()->GetKernel(), this, index, &key, sizeof(key), DTYPE_INT64);
    }

    return index;
}

RowIndex TableControl::AddRowKeyString(const char * key) {
    OASSERT(_columnInfo.key.type == DTYPE_STRING, "wtf");
    RowIndex index = -1;
    if (_columnInfo.key.type == DTYPE_STRING) {
        if (_stringKeyMap.find(key) != _stringKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return index;
        }

        index = _rows.size();
        TableRow * pRow = TableRowPool::GetInterface()->Create(&_columnInfo);
        _stringKeyMap.insert(std::make_pair(key, index));
        pRow->SetValue(_columnInfo.key.index, DTYPE_STRING, key, strlen(key));
        _rows.push_back(pRow);

        AddCallBack(ObjectMgr::Instance()->GetKernel(), this, index, key, strlen(key) + 1, DTYPE_STRING);
    }

    return index;
}

//插入行
bool TableControl::InsertRow(const RowIndex index) {
    OASSERT(index < (s32)_rows.size() && index >= 0 && _columnInfo.key.type == DTYPE_CANT_BE_KEY, "index over flow");
    if (index >= (s32)_rows.size() || index < 0 || _columnInfo.key.type != DTYPE_CANT_BE_KEY) {
        return false;
    }

    _rows.insert(_rows.begin() + index, TableRowPool::GetInterface()->Create(&_columnInfo));
    AddCallBack(ObjectMgr::Instance()->GetKernel(), this, index, nullptr, 0, DTYPE_CANT_BE_KEY);
    return true;
}

bool TableControl::InsertRowKeyInt8(const s8 key, const RowIndex index) {
    OASSERT(index < (s32)_rows.size() && index >= 0 && _columnInfo.key.type == DTYPE_INT8, "index over flow");
    if (index >= (s32)_rows.size() || index < 0 || _columnInfo.key.type != DTYPE_INT8) {
        return false;
    }

    if (_columnInfo.key.type == DTYPE_INT8) {
        if (_intKeyMap.find(key) != _intKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return false;
        }
        TableRow * pRow = TableRowPool::GetInterface()->Create(&_columnInfo);
        pRow->SetValue(_columnInfo.key.index, DTYPE_INT8, &key, sizeof(key));
        _rows.insert(_rows.begin() + index, pRow);
        OrderProcIndex(index, 1);
        _intKeyMap.insert(std::make_pair(key, index));

        AddCallBack(ObjectMgr::Instance()->GetKernel(), this, index, &key, sizeof(key), DTYPE_INT8);
        return true;
    } 
    return false;
}

bool TableControl::InsertRowKeyInt16(const s16 key, const RowIndex index) {
    OASSERT(index < (s32)_rows.size() && index >= 0 && _columnInfo.key.type == DTYPE_INT16, "index over flow");
    if (index >= (s32)_rows.size() || index < 0 || _columnInfo.key.type != DTYPE_INT16) {
        return false;
    }

    if (_columnInfo.key.type == DTYPE_INT16) {
        if (_intKeyMap.find(key) != _intKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return false;
        }

        TableRow * pRow = TableRowPool::GetInterface()->Create(&_columnInfo);
        pRow->SetValue(_columnInfo.key.index, DTYPE_INT16, &key, sizeof(key));
        _rows.insert(_rows.begin() + index, pRow);
        OrderProcIndex(index, 1);
        _intKeyMap.insert(std::make_pair(key, index));
        AddCallBack(ObjectMgr::Instance()->GetKernel(), this, index, &key, sizeof(key), DTYPE_INT16);
        return true;
    }
    return false;
}

bool TableControl::InsertRowKeyInt32(const s32 key, const RowIndex index) {
    OASSERT(index < (s32)_rows.size() && index >= 0 && _columnInfo.key.type == DTYPE_INT32, "index over flow");
    if (index >= (s32)_rows.size() || index < 0 || _columnInfo.key.type != DTYPE_INT32) {
        return false;
    }

    if (_columnInfo.key.type == DTYPE_INT32) {
        if (_intKeyMap.find(key) != _intKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return false;
        }

        TableRow * pRow = TableRowPool::GetInterface()->Create(&_columnInfo);
        pRow->SetValue(_columnInfo.key.index, DTYPE_INT32, &key, sizeof(key));
        _rows.insert(_rows.begin() + index, pRow);
        OrderProcIndex(index, 1);
        _intKeyMap.insert(std::make_pair(key, index));
        AddCallBack(ObjectMgr::Instance()->GetKernel(), this, index, &key, sizeof(key), DTYPE_INT32);
        return true;
    }
    return false;
}

bool TableControl::InsertRowKeyInt64(const s64 key, const RowIndex index) {
    OASSERT(index < (s32)_rows.size() && index >= 0 && _columnInfo.key.type == DTYPE_INT64, "index over flow");
    if (index >= (s32)_rows.size() || index < 0 || _columnInfo.key.type != DTYPE_INT64) {
        return false;
    }

    if (_columnInfo.key.type == DTYPE_INT64) {
        if (_intKeyMap.find(key) != _intKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return false;
        }

        TableRow * pRow = TableRowPool::GetInterface()->Create(&_columnInfo);
        pRow->SetValue(_columnInfo.key.index, DTYPE_INT64, &key, sizeof(key));
        _rows.insert(_rows.begin() + index, pRow);
        OrderProcIndex(index, 1);
        _intKeyMap.insert(std::make_pair(key, index));
        AddCallBack(ObjectMgr::Instance()->GetKernel(), this, index, &key, sizeof(key), DTYPE_INT64);
        return true;
    }
    return false;
}

bool TableControl::InsertRowKeyString(const char * key, const RowIndex index) {
    OASSERT(index < (s32)_rows.size() && index >= 0 && _columnInfo.key.type == DTYPE_STRING, "index over flow");
    if (index >= (s32)_rows.size() || index < 0 || _columnInfo.key.type != DTYPE_STRING) {
        return false;
    }

    if (_columnInfo.key.type == DTYPE_STRING) {
        if (_stringKeyMap.find(key) != _stringKeyMap.end()) {
            OASSERT(false, "key is already exits");
            return false;
        }

        TableRow * pRow = TableRowPool::GetInterface()->Create(&_columnInfo);
        pRow->SetValue(_columnInfo.key.index, DTYPE_STRING, key, strlen(key));
        _rows.insert(_rows.begin() + index, pRow);
        OrderProcIndex(index, 1);
        _stringKeyMap.insert(std::make_pair(key, index));
        AddCallBack(ObjectMgr::Instance()->GetKernel(), this, index, key, strlen(key) + 1, DTYPE_STRING);
        return true;
    }
    return false;
}

void TableControl::OrderProcIndex(const RowIndex index, const s32 index_diff) {
    switch (_columnInfo.key.type) {
	case DTYPE_INT8:
		{
			for (s32 i = index + 1; i < (s32)_rows.size(); i++) {
				s32 nSize = 0;
				s8 key = *(s8 *)_rows[i]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = _intKeyMap.find(key);
				OASSERT(itor != _intKeyMap.end(), "where is key");
				if (itor != _intKeyMap.end()) {
					s32 oldIndex = itor->second;
                    itor->second += index_diff;
				}
			}
		}
		break;
	case DTYPE_INT16:
		{
			for (s32 i = index + 1; i < (s32)_rows.size(); i++) {
				s32 nSize = 0;
				s16 key = *(s16 *)_rows[i]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = _intKeyMap.find(key);
				OASSERT(itor != _intKeyMap.end(), "where is key");
				if (itor != _intKeyMap.end()) {
					s32 oldIndex = itor->second;
                    itor->second += index_diff;
				}
			}
		}
		break;
	case DTYPE_INT32:
		{
			for (s32 i = index + 1; i < (s32)_rows.size(); i++) {
				s32 nSize = 0;
				s32 key = *(s32 *)_rows[i]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = _intKeyMap.find(key);
				OASSERT(itor != _intKeyMap.end(), "where is key");
				if (itor != _intKeyMap.end()) {
					s32 oldIndex = itor->second;
                    itor->second += index_diff;
				}
			}
		}
		break;
	case DTYPE_INT64:
		{
			for (s32 i = index + 1; i < (s32)_rows.size(); i++) {
				s32 nSize = 0;
				s64 key = *(s64 *)_rows[i]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = _intKeyMap.find(key);
				OASSERT(itor != _intKeyMap.end(), "where is key");
				if (itor != _intKeyMap.end()) {
					s32 oldIndex = itor->second;
                    itor->second += index_diff;
				}
			}
		}
		break;
	case DTYPE_STRING:
		{
			for (s32 i = index + 1; i < (s32)_rows.size(); i++) {
				s32 nSize = 0;
				const char * key = (const char *)_rows[i]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
				KEY_STRING_MAP::iterator itor = _stringKeyMap.find(key);
				OASSERT(itor != _stringKeyMap.end(), "where is key");
				if (itor != _stringKeyMap.end()) {
					s32 oldIndex = itor->second;
                    itor->second += index_diff;
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
    OASSERT(index < (s32)_rows.size() && index >= 0, "index over flow");
    if (index >= (s32)_rows.size() || index < 0) {
        return false;
    }
    
    switch (_columnInfo.key.type) {
	case DTYPE_INT8:
		{
			s32 nSize = 0;
			s8 key = *(s8 *)_rows[index]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
			KEY_INT_MAP::iterator itor = _intKeyMap.find(key);
			OASSERT(itor != _intKeyMap.end(), "where is key");
			if (itor != _intKeyMap.end()) {
				_intKeyMap.erase(itor);
				//OM_TRACE("Table %s delete key %ld", m_oTableName.GetString(), key);

                DeleteCallBack(ObjectMgr::Instance()->GetKernel(), this, index);
			}
			break;
		}
	case DTYPE_INT16:
		{
			s32 nSize = 0;
			s16 key = *(s16 *)_rows[index]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
			KEY_INT_MAP::iterator itor = _intKeyMap.find(key);
			OASSERT(itor != _intKeyMap.end(), "where is key");
			if (itor != _intKeyMap.end()) {
				_intKeyMap.erase(itor);
                //OM_TRACE("Table %s delete key %ld", m_oTableName.GetString(), key);
                DeleteCallBack(ObjectMgr::Instance()->GetKernel(), this, index);
			}
			break;
		}
	case DTYPE_INT32:
		{
			s32 nSize = 0;
			s32 key = *(s32 *)_rows[index]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
			KEY_INT_MAP::iterator itor = _intKeyMap.find(key);
			OASSERT(itor != _intKeyMap.end(), "where is key");
			if (itor != _intKeyMap.end()) {
				_intKeyMap.erase(itor);
                //OM_TRACE("Table %s delete key %ld", m_oTableName.GetString(), key);
                DeleteCallBack(ObjectMgr::Instance()->GetKernel(), this, index);
			}
			break;
		}
    case DTYPE_INT64:
        {
            s32 nSize = 0;
            s64 key = *(s64 *)_rows[index]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
            KEY_INT_MAP::iterator itor = _intKeyMap.find(key);
            OASSERT(itor != _intKeyMap.end(), "where is key");
            if (itor != _intKeyMap.end()) {
                _intKeyMap.erase(itor);
                //OM_TRACE("Table %s delete key %ld", m_oTableName.GetString(), key);
                DeleteCallBack(ObjectMgr::Instance()->GetKernel(), this, index);
            }
        }
        break;
    case DTYPE_STRING:
        {
            s32 nSize = 0;
            const char * key = (const char *)_rows[index]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
            KEY_STRING_MAP::iterator itor = _stringKeyMap.find(key);
            OASSERT(itor != _stringKeyMap.end(), "where is key");
            if (itor != _stringKeyMap.end()) {
                _stringKeyMap.erase(itor);
                //OM_TRACE("Table %s delete key %s", m_oTableName.GetString(), key);
                DeleteCallBack(ObjectMgr::Instance()->GetKernel(), this, index);
            }
        }
        break;
    case DTYPE_CANT_BE_KEY:
        break;
    default:
        OASSERT(false, "wtf");
        break;
    }


    TABLE_ROWS::iterator itor = _rows.begin() + index;
    TableRowPool::GetInterface()->Recover(*itor);

    OrderProcIndex(index, -1);

    _rows.erase(itor);

    return true;
}

bool TableControl::SwapRowIndex(const RowIndex src, const RowIndex dst) {
    OASSERT(src < (s32)_rows.size() && dst < (s32)_rows.size(), "wtf");

    switch (_columnInfo.key.type) {
		case DTYPE_INT8:
		{
			{
				s32 nSize = 0;
				s8 key = *(s8 *)_rows[src]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = _intKeyMap.find(key);
				OASSERT(itor != _intKeyMap.end(), "where is key");
				if (itor != _intKeyMap.end()) {
					itor->second = dst;
					//OM_TRACE("table %s row %lld index %d changed to %d", m_oTableName.GetString(), key, src, itor->second);
				}
			}
			{
				s32 nSize = 0;
				s8 key = *(s8 *)_rows[dst]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = _intKeyMap.find(key);
				OASSERT(itor != _intKeyMap.end(), "where is key");
				if (itor != _intKeyMap.end()) {
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
				s16 key = *(s16 *)_rows[src]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = _intKeyMap.find(key);
				OASSERT(itor != _intKeyMap.end(), "where is key");
				if (itor != _intKeyMap.end()) {
					itor->second = dst;
					//OM_TRACE("table %s row %lld index %d changed to %d", m_oTableName.GetString(), key, src, itor->second);
				}
			}
			{
				s32 nSize = 0;
				s16 key = *(s16 *)_rows[dst]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = _intKeyMap.find(key);
				OASSERT(itor != _intKeyMap.end(), "where is key");
				if (itor != _intKeyMap.end()) {
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
				s32 key = *(s32 *)_rows[src]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = _intKeyMap.find(key);
				OASSERT(itor != _intKeyMap.end(), "where is key");
				if (itor != _intKeyMap.end()) {
					itor->second = dst;
				}
			}
			{
				s32 nSize = 0;
				s32 key = *(s32 *)_rows[dst]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
				KEY_INT_MAP::iterator itor = _intKeyMap.find(key);
				OASSERT(itor != _intKeyMap.end(), "where is key");
				if (itor != _intKeyMap.end()) {
					itor->second = src;
				}
			}
			break;
		}
        case DTYPE_INT64:
        {
            {
                s32 nSize = 0;
                s64 key = *(s64 *)_rows[src]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
                KEY_INT_MAP::iterator itor = _intKeyMap.find(key);
                OASSERT(itor != _intKeyMap.end(), "where is key");
                if (itor != _intKeyMap.end()) {
                    itor->second = dst;
                }
            }
            {
                s32 nSize = 0;
                s64 key = *(s64 *)_rows[dst]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
                KEY_INT_MAP::iterator itor = _intKeyMap.find(key);
                OASSERT(itor != _intKeyMap.end(), "where is key");
                if (itor != _intKeyMap.end()) {
                    itor->second = src;
                }
            }
            break;
        }
        case DTYPE_STRING:
        {
            {
                s32 nSize = 0;
                const char * key = (const char *)_rows[src]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
                KEY_STRING_MAP::iterator itor = _stringKeyMap.find(key);
                OASSERT(itor != _stringKeyMap.end(), "where is key");
                if (itor != _stringKeyMap.end()) {
                    itor->second = dst;
                    //OM_TRACE("table %s row %s index %d changed to %d", m_oTableName.GetString(), key, src, itor->second);
                }
            }

            {
                s32 nSize = 0;
                const char * key = (const char *)_rows[dst]->GetValue(_columnInfo.key.index, _columnInfo.key.type, nSize);
                KEY_STRING_MAP::iterator itor = _stringKeyMap.find(key);
                OASSERT(itor != _stringKeyMap.end(), "where is key");
                if (itor != _stringKeyMap.end()) {
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

    TableRow * pSwap = _rows[src];
    _rows[src] = _rows[dst];
    _rows[dst] = pSwap;

    SwapCallBack(ObjectMgr::Instance()->GetKernel(), this, src, dst);
    return true;
}

/*========CHANGE KEY=========*/
bool TableControl::ChangeKey(const s64 oldKey, const s64 newKey, const s8 type) {
    OASSERT(type == DTYPE_INT8 || type == DTYPE_INT16
        ||type == DTYPE_INT32 || type == DTYPE_INT64, "wtf");

    OASSERT(_columnInfo.key.type == type, "wtf");
    if (_columnInfo.key.type != type) {
        return false;
    }

    KEY_INT_MAP::iterator iNewtor = _intKeyMap.find(newKey);
    if (iNewtor != _intKeyMap.end()) {
        return false;
    }

    KEY_INT_MAP::iterator iOldtor = _intKeyMap.find(oldKey);
    OASSERT(iOldtor != _intKeyMap.end(), "where is old key");
    if (iOldtor == _intKeyMap.end()) {
        return false;
    }
    _intKeyMap.insert(std::make_pair(newKey, iOldtor->second));
    _intKeyMap.erase(iOldtor);
    return false;
}

bool TableControl::ChangeKey(const char * oldKey, const char * newKey, const s8 type) {
    OASSERT(type == DTYPE_STRING && _columnInfo.key.type == type, "wtf");

    if (_columnInfo.key.type != type) {
        return false;
    }

    KEY_STRING_MAP::iterator iNewtor = _stringKeyMap.find(newKey);
    if (iNewtor != _stringKeyMap.end()) {
        return false;
    }

    KEY_STRING_MAP::iterator iOldtor = _stringKeyMap.find(oldKey);
    OASSERT(iOldtor != _stringKeyMap.end(), "where is old key");
    if (iOldtor == _stringKeyMap.end()) {
        return false;
    }
    _stringKeyMap.insert(std::make_pair(newKey, iOldtor->second));
    _stringKeyMap.erase(iOldtor);
    return false;
}

bool TableControl::GetRowData(const s32 row, const void * & pData, s32 & size) {
    pData = _rows[row]->GetData();
    size = _rows[row]->GetSize();
    return true;
}

RowIndex TableControl::AddRowData(const void * pData, const s32 size) {
    TableRow * pRow = TableRowPool::GetInterface()->Create(&_columnInfo);
    pRow->Copy(pData, size);
    s32 index = _rows.size();

    switch (_columnInfo.key.type) {
    case DTYPE_INT8:
    {
                       s32 size = 0;
                       s8 key = *(s8*)pRow->GetValue(_columnInfo.key.index, DTYPE_INT8, size);
                       OASSERT(size == sizeof(key), "wtf");
                       if (_intKeyMap.find(key) != _intKeyMap.end()) {
                           OASSERT(false, "key is already exits");
                           TableRowPool::GetInterface()->Recover(pRow);
                           return false;
                       }

                       _rows.push_back(pRow);
                       _intKeyMap.insert(std::make_pair(key, index));
                       break;
    }
    case DTYPE_INT16:
    {
                        s32 size = 0;
                        s16 key = *(s16*)pRow->GetValue(_columnInfo.key.index, DTYPE_INT16, size);
                        OASSERT(size == sizeof(key), "wtf");
                        if (_intKeyMap.find(key) != _intKeyMap.end()) {
                            OASSERT(false, "key is already exits");
                            TableRowPool::GetInterface()->Recover(pRow);
                            return false;
                        }

                        _rows.push_back(pRow);
                        _intKeyMap.insert(std::make_pair(key, index));
                        break;
    }
    case DTYPE_INT32:
    {
                        s32 size = 0;
                        s32 key = *(s32*)pRow->GetValue(_columnInfo.key.index, DTYPE_INT32, size);
                        OASSERT(size == sizeof(key), "wtf");
                        if (_intKeyMap.find(key) != _intKeyMap.end()) {
                            OASSERT(false, "key is already exits");
                            TableRowPool::GetInterface()->Recover(pRow);
                            return false;
                        }

                        _rows.push_back(pRow);
                        _intKeyMap.insert(std::make_pair(key, index));
                        break;
    }
    case DTYPE_INT64:
    {
                        s32 size = 0;
                        s64 key = *(s64*)pRow->GetValue(_columnInfo.key.index, DTYPE_INT64, size);
                        OASSERT(size == sizeof(key), "wtf");
                        if (_intKeyMap.find(key) != _intKeyMap.end()) {
                            OASSERT(false, "key is already exits");
                            TableRowPool::GetInterface()->Recover(pRow);
                            return false;
                        }

                        _rows.push_back(pRow);
                        _intKeyMap.insert(std::make_pair(key, index));
                        break;
    }
    case DTYPE_STRING:
    {
                         s32 size = 0;
                         const char * key = (const char *)pRow->GetValue(_columnInfo.key.index, DTYPE_STRING, size);
                         if (_stringKeyMap.find(key) != _stringKeyMap.end()) {
                             OASSERT(false, "key is already exits");
                             TableRowPool::GetInterface()->Recover(pRow);
                             return false;
                         }

                         _rows.push_back(pRow);
                         _stringKeyMap.insert(std::make_pair(key, index));
                         break;
    }
    default:
        break;
    }

    return index;
}

/*==============================================================================*/
s8 TableControl::GetDataInt8(const RowIndex rowIndex, const ColumnIndex clmIndex) const {
    OASSERT(rowIndex < (s32)_rows.size(), "row index over flow");
    if (rowIndex >= (s32)_rows.size())
        return 0;

    s32 nLen = 0;
    const void * value = _rows[rowIndex]->GetValue(clmIndex, DTYPE_INT8, nLen);
    OASSERT(nLen == sizeof(s8), "wtf");

    return *(s8 *)value;
}

bool TableControl::SetDataInt8(const RowIndex rowIndex, const ColumnIndex clmIndex, const s8 value) {
    OASSERT(rowIndex < (s32)_rows.size(), "row index over flow");
    if (rowIndex >= (s32)_rows.size()) {
        return 0;
    }

    if (_columnInfo.key.index == clmIndex) {
        OASSERT(false, "wtf");
        OASSERT(_columnInfo.key.type == DTYPE_INT8, "key type error, check ur om logic");
        s8 oldValue = GetDataInt8(rowIndex, _columnInfo.key.index);
        if (!ChangeKey(oldValue, value, DTYPE_INT8)) {
            return false;
        }
    }

    if (_rows[rowIndex]->SetValue(clmIndex, DTYPE_INT8, &value, sizeof(value))) {
        UpdateCallBack(ObjectMgr::Instance()->GetKernel(), rowIndex, clmIndex, &value, sizeof(value), DTYPE_INT8);
        return true;
    }

    return false;
}
/*==============================================================================*/
s16 TableControl::GetDataInt16(const RowIndex rowIndex, const ColumnIndex clmIndex) const {
    OASSERT(rowIndex < (s32)_rows.size(), "row index over flow");
    if (rowIndex >= (s32)_rows.size()) {
        return 0;
    }

    s32 nLen = 0;
    const void * value = _rows[rowIndex]->GetValue(clmIndex, DTYPE_INT16, nLen);
    OASSERT(nLen == sizeof(s16), "wtf");
    return *(s16 *)value;
}

bool TableControl::SetDataInt16(const RowIndex rowIndex, const ColumnIndex clmIndex, const s16 value) {
    OASSERT(rowIndex < (s32)_rows.size(), "row index over flow");
    if (rowIndex >= (s32)_rows.size()) {
        return 0;
    }

    if (_columnInfo.key.index == clmIndex) {
        OASSERT(false, "wtf");
        OASSERT(_columnInfo.key.type == DTYPE_INT16, "key type error, check ur om logic");
        s16 oldValue = GetDataInt16(rowIndex, _columnInfo.key.index);
        if (!ChangeKey(oldValue, value, DTYPE_INT16)) {
            return false;
        }
    }

    if (_rows[rowIndex]->SetValue(clmIndex, DTYPE_INT16, &value, sizeof(value))) {
        UpdateCallBack(ObjectMgr::Instance()->GetKernel(), rowIndex, clmIndex, &value, sizeof(value), DTYPE_INT16);
        return true;
    }
    return false;
}

/*==============================================================================*/
s32 TableControl::GetDataInt32(const RowIndex rowIndex, const ColumnIndex clmIndex) const {
    OASSERT(rowIndex < (s32)_rows.size(), "row index over flow");
    if (rowIndex >= (s32)_rows.size()) {
        return 0;
    }

    s32 nLen = 0;
    const void * value = _rows[rowIndex]->GetValue(clmIndex, DTYPE_INT32, nLen);
    OASSERT(nLen == sizeof(s32), "wtf");
    return *(s32 *)value;
}

bool TableControl::SetDataInt32(const RowIndex rowIndex, const ColumnIndex clmIndex, const s32 value) {
    OASSERT(rowIndex < (s32)_rows.size(), "row index over flow");
    if (rowIndex >= (s32)_rows.size()) {
        return 0;
    }

    if (_columnInfo.key.index == clmIndex) {
        OASSERT(false, "wtf");
        OASSERT(_columnInfo.key.type == DTYPE_INT32, "key type error, check ur om logic");
        s32 oldValue = GetDataInt32(rowIndex, _columnInfo.key.index);
        if (!ChangeKey(oldValue, value, DTYPE_INT32)) {
            return false;
        }
    }

    if (_rows[rowIndex]->SetValue(clmIndex, DTYPE_INT32, &value, sizeof(value))) {
        UpdateCallBack(ObjectMgr::Instance()->GetKernel(), rowIndex, clmIndex, &value, sizeof(value), DTYPE_INT32);
        return true;
    }
    return false;
}
/*==============================================================================*/
s64 TableControl::GetDataInt64(const RowIndex rowIndex, const ColumnIndex clmIndex) const {
    OASSERT(rowIndex < (s32)_rows.size(), "row index over flow");
    if (rowIndex >= (s32)_rows.size()) {
        return 0;
    }

    s32 nLen = 0;
    const void * value = _rows[rowIndex]->GetValue(clmIndex, DTYPE_INT64, nLen);
    OASSERT(nLen == sizeof(s64), "wtf");
    return *(s64 *)value;
}

bool TableControl::SetDataInt64(const RowIndex rowIndex, const ColumnIndex clmIndex, const s64 value) {
    OASSERT(rowIndex < (s32)_rows.size(), "row index over flow");
    if (rowIndex >= (s32)_rows.size()) {
        return 0;
    }
    if (_columnInfo.key.index == clmIndex) {
        OASSERT(false, "wtf");
        OASSERT(_columnInfo.key.type == DTYPE_INT64, "key type error, check ur om logic");
        s64 oldValue = GetDataInt64(rowIndex, _columnInfo.key.index);
        if (!ChangeKey(oldValue, value, DTYPE_INT64)) {
            return false;
        }
    }

    if (_rows[rowIndex]->SetValue(clmIndex, DTYPE_INT64, &value, sizeof(value))) {
        UpdateCallBack(ObjectMgr::Instance()->GetKernel(), rowIndex, clmIndex, &value, sizeof(value), DTYPE_INT64);
        return true;
    }
    return false;
}
/*==============================================================================*/
float TableControl::GetDataFloat(const RowIndex rowIndex, const ColumnIndex clmIndex) const {
    OASSERT(rowIndex < (s32)_rows.size(), "row index over flow");
    if (rowIndex >= (s32)_rows.size()) {
        return 0;
    }    
    
    s32 nLen = 0;
    const void * value = _rows[rowIndex]->GetValue(clmIndex, DTYPE_FLOAT, nLen);
    OASSERT(nLen == sizeof(float), "wtf");
    return *(float *)value;
}

bool TableControl::SetDataFloat(const RowIndex rowIndex, const ColumnIndex clmIndex, const float value) {
    OASSERT(rowIndex < (s32)_rows.size(), "row index over flow");
    if (rowIndex >= (s32)_rows.size()) {
        return 0;
    }

    if (_rows[rowIndex]->SetValue(clmIndex, DTYPE_FLOAT, &value, sizeof(value))) {
        UpdateCallBack(ObjectMgr::Instance()->GetKernel(), rowIndex, clmIndex, &value, sizeof(value), DTYPE_FLOAT);
        return true;
    }
    return false;
}

/*==============================================================================*/
const char * TableControl::GetDataString(const RowIndex rowIndex, const ColumnIndex clmIndex) const {
    OASSERT(rowIndex < (s32)_rows.size(), "row index over flow");
    if (rowIndex >= (s32)_rows.size()) {
        return 0;
    }

    s32 nLen = 0;
    const void * value = _rows[rowIndex]->GetValue(clmIndex, DTYPE_STRING, nLen);
    return (const char *)value;
}

bool TableControl::SetDataString(const RowIndex rowIndex, const ColumnIndex clmIndex, const char * value) {
    OASSERT(rowIndex < (s32)_rows.size(), "row index over flow");
    if (rowIndex >= (s32)_rows.size()) {
        return 0;
    }
    if (_columnInfo.key.index == clmIndex) {
        OASSERT(false, "wtf");
        OASSERT(_columnInfo.key.type == DTYPE_STRING, "key type error, check ur om logic");
        const char * oldValue = GetDataString(rowIndex, _columnInfo.key.index);
        if (!ChangeKey(oldValue, value, DTYPE_STRING)) {
            return false;
        }
    }

    if (_rows[rowIndex]->SetValue(clmIndex, DTYPE_STRING, value, strlen(value) + 1)) {
        UpdateCallBack(ObjectMgr::Instance()->GetKernel(), rowIndex, clmIndex, value, strlen(value) + 1, DTYPE_STRING);
        return true;
    }
    return false;
}

/*==============================================================================*/
const void * TableControl::GetDataStruct(const RowIndex rowIndex, const ColumnIndex clmIndex, const s32 size) const {
    OASSERT(rowIndex < (s32)_rows.size(), "row index over flow");
    if (rowIndex >= (s32)_rows.size()) {
        return 0;
    }

    s32 nLen = 0;
    const void * value = _rows[rowIndex]->GetValue(clmIndex, DTYPE_STRUCT, nLen);
    OASSERT(nLen == size, "wtf");
    return value;
}

bool TableControl::SetDataStruct(const RowIndex rowIndex, const ColumnIndex clmIndex, const void * value, const s32 size) {
    OASSERT(rowIndex < (s32)_rows.size(), "row index over flow");
    if (rowIndex >= (s32)_rows.size()) {
        return 0;
    }

    if (_rows[rowIndex]->SetValue(clmIndex, DTYPE_STRUCT, value, size)) {
        UpdateCallBack(ObjectMgr::Instance()->GetKernel(), rowIndex, clmIndex, value, size, DTYPE_STRUCT);
        return true;
    }
    return false;
}

/*==============================================================================*/
