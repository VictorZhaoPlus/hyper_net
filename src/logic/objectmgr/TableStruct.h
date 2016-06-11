/* 
 * File:   TableStruct.h
 * Author: Alax
 *
 * Created on March 3, 2015, 10:46 AM
 */
#ifndef __TableStruct_h__
#define __TableStruct_h__
#include "MultiSys.h"
#include "TString.h"
#include "ObjectMgrMacro.h"
#include <vector>
#include <unordered_map>

typedef s32 ColumnIndex;
typedef s32 MemOffset;
struct ColumnInfo{
    ColumnInfo(const ColumnInfo & target): index(target.index), offset(target.offset), size(target.size), mask(target.mask) {}
    ColumnInfo(const ColumnIndex _index, const MemOffset _offset, const s32 _size, const s8 _mask)
        : index(_index), offset(_offset), size(_size), mask(_mask) {}
    const ColumnIndex index;
    const MemOffset offset;
    const s32 size;
    const s8 mask;

    ColumnInfo & operator=(const ColumnInfo & target) {
        SafeMemcpy(this, sizeof(*this), &target, sizeof(target));
        return *this;
    }
};

class TableColumnInfo {
    struct KeyInfo {
        s32 type;
        s32 index;
    };
    typedef std::vector<ColumnInfo> COLUMN_INDEX;
public:
    TableColumnInfo() : m_nMemorySize(0) {
        shadow = false;
        key.index = -1;
        key.type = DTYPE_CANT_BE_KEY;
    }

    inline bool AddColumnInt8(const ColumnIndex index, bool isKey) {
        if (isKey) {
            OASSERT(key.type >= DTYPE_CANT_BE_KEY, "key is already set");
            if (key.type < DTYPE_CANT_BE_KEY) {
                OM_TRACE("Can not set Column as key, key is already set");
                return false;
            }
        }

        const ColumnInfo * pColumnInfo = AddColumn(index, sizeof(s8), DTYPE_INT8);
        if (NULL == pColumnInfo) {
            return false;
        }

        if (isKey) {
            key.type = DTYPE_INT8;
            key.index = pColumnInfo->index;
        }

        return true;
    }

    bool AddColumnInt16(const ColumnIndex index, bool isKey) {
        if (isKey) {
            OASSERT(key.type >= DTYPE_CANT_BE_KEY, "key is already set");
            if (key.type < DTYPE_CANT_BE_KEY) {
                OM_TRACE("Can not set Column as key, key is already set");
                return false;
            }
        }

        const ColumnInfo * pColumnInfo = AddColumn(index, sizeof(s16), DTYPE_INT16);
        if (NULL == pColumnInfo) {
            return false;
        }

        if (isKey) {
            key.type = DTYPE_INT16;
            key.index = pColumnInfo->index;
        }

        return true;
    }

    bool AddColumnInt32(const ColumnIndex index, bool isKey) {
        if (isKey) {
            OASSERT(key.type >= DTYPE_CANT_BE_KEY, "key is already set");
            if (key.type < DTYPE_CANT_BE_KEY) {
                OM_TRACE("Can not set Column as key, key is already set");
                return false;
            }
        }

        const ColumnInfo * pColumnInfo = AddColumn(index, sizeof(s32), DTYPE_INT32);
        if (NULL == pColumnInfo) {
            return false;
        }

        if (isKey) {
            key.type = DTYPE_INT32;
            key.index = pColumnInfo->index;
        }
        return true;
    }

    bool AddColumnInt64(const ColumnIndex index, bool isKey) {
        if (isKey) {
            OASSERT(key.type >= DTYPE_CANT_BE_KEY, "key is already set");
            if (key.type < DTYPE_CANT_BE_KEY) {
                OM_TRACE("Can not set Column as key, key is already set");
                return false;
            }
        }

        const ColumnInfo * pColumnInfo = AddColumn(index, sizeof(s64), DTYPE_INT64);
        if (NULL == pColumnInfo) {
            return false;
        }

        if (isKey) {
            key.type = DTYPE_INT64;
            key.index = pColumnInfo->index;
        }
        return true;
    }

    bool AddColumnString(const ColumnIndex index, const s32 maxlen, bool isKey) {
        if (isKey) {
            OASSERT(key.type >= DTYPE_CANT_BE_KEY, "key is already set");
            if (key.type < DTYPE_CANT_BE_KEY) {
                OM_TRACE("Can not set Column as key, key is already set");
                return false;
            }
        }

        const ColumnInfo * pColumnInfo = AddColumn(index, maxlen, DTYPE_STRING);
        if (NULL == pColumnInfo) {
            return false;
        }

        if (isKey) {
            key.type = DTYPE_STRING;
            key.index = pColumnInfo->index;
        }
        return true;
    }

    bool AddColumnFloat(const ColumnIndex index) {
        return NULL != AddColumn(index, sizeof(float), DTYPE_FLOAT);
    }

    bool AddColumnStruct(const ColumnIndex index, const s32 size) {
        return NULL != AddColumn(index, size, DTYPE_STRUCT);
    }

    inline const ColumnInfo * AddColumn(const ColumnIndex index, const s32 size, const s8 mask) {
        if (index != m_oColumnIndex.size()) {
            OASSERT(false, "column index error");
            return NULL;
        }

        ColumnInfo info(m_oColumnIndex.size(), m_nMemorySize, size, mask);
        m_oColumnIndex.push_back(info);
        m_nMemorySize += size;
        return &m_oColumnIndex[m_oColumnIndex.size() - 1];
    }

    inline const ColumnInfo * QueryColumnInfo(const ColumnIndex index) const {
        if (index >= m_oColumnIndex.size()) {
            OASSERT(false, "column index over flow");
            return NULL;
        }
        return &m_oColumnIndex[index];
    }

    inline s32 CalcMemorySize() const {
        return m_nMemorySize;
    }

    inline void Copy(const TableColumnInfo & target) {
        tools::SafeMemcpy(&key, sizeof(key), &target.key, sizeof(target.key));
        shadow = target.shadow;
        m_oColumnIndex.assign(target.m_oColumnIndex.cbegin(), target.m_oColumnIndex.cend());
        m_nMemorySize = target.m_nMemorySize;
    }

    KeyInfo key;
    bool shadow;
private:
    COLUMN_INDEX m_oColumnIndex;
    s32 m_nMemorySize;
};

class TableRowPool;
class TableRow {
public:
    inline bool SetValue(const ColumnIndex index, const s8 mask, const void * pValue, const s32 size) {
        const ColumnInfo * pInfo = m_pColumnInfo->QueryColumnInfo(index);
        if (NULL == pInfo) {
            OASSERT(false, "wtf");
            return false;
        }

        OASSERT(pInfo->mask == mask, "value type error");
        OASSERT(pInfo->size >= size, "value size over flow");

        tools::SafeMemcpy(m_pBuff + pInfo->offset, pInfo->size, pValue, size);
        return true;
    }

    inline const void * GetValue(const ColumnIndex index, const s8 mask, s32 & size) {
        const ColumnInfo * pInfo = m_pColumnInfo->QueryColumnInfo(index);
        if (NULL == pInfo) {
            OASSERT(false, "wtf");
            return NULL;
        }

        OASSERT(pInfo->mask == mask, "value type error");

        size = pInfo->size;
        return m_pBuff + pInfo->offset;
    }

    inline void Copy(const void * pData, const s32 size) {
        OASSERT(size == m_nBuffSize, "wtf");
        tools::SafeMemcpy(m_pBuff, m_nBuffSize, pData, size);
    }

    inline const void * GetData() { return m_pBuff; }

    inline s32 GetSize() {return m_nBuffSize;}

private:
    friend TableRowPool;
    inline bool SetColumnInfo(const TableColumnInfo * pColumnInfo) {
        if (pColumnInfo->CalcMemorySize() != m_nBuffSize && m_nBuffSize != 0) {
            OASSERT(false, "memory size diff");
            return false;
        }

        m_pColumnInfo = pColumnInfo;
        tools::SafeMemset(m_pBuff, m_nBuffSize, 0, m_nBuffSize);
        return true;
    }

    inline void Clear() {tools::SafeMemset(m_pBuff, m_nBuffSize, 0, m_nBuffSize);}

    TableRow(const TableColumnInfo * pColumnInfo) : m_pColumnInfo(pColumnInfo), m_pBuff(NULL), m_nBuffSize(pColumnInfo->CalcMemorySize()) {
        OASSERT(pColumnInfo && pColumnInfo->CalcMemorySize() > 0 && m_nBuffSize == pColumnInfo->CalcMemorySize(), "nullptr ColumnInfo");
        m_pBuff = NEW char[m_nBuffSize];
        tools::SafeMemset(m_pBuff, m_nBuffSize, 0, m_nBuffSize);
    }

    virtual ~TableRow() {
        DEL[] m_pBuff;
    }

private:
    const TableColumnInfo * m_pColumnInfo;
    char * m_pBuff;
    const s32 m_nBuffSize;
};

#endif //defined __TableStruct_h__
