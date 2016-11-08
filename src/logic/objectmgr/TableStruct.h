/* 
 * File:   TableStruct.h
 * Author: Alax
 *
 * Created on March 3, 2015, 10:46 AM
 */
#ifndef __TableStruct_h__
#define __TableStruct_h__
#include "util.h"
#include "OString.h"
#include <vector>
#include <unordered_map>
#include "IObjectMgr.h"

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
    TableColumnInfo() : _memorySize(0) {
        key.index = -1;
        key.type = DTYPE_CANT_BE_KEY;
    }

	inline bool AddColumnInt8(const ColumnIndex index, bool isKey) { return AddColumnWithKey(index, sizeof(s8), isKey, DTYPE_INT8); }
	inline bool AddColumnInt16(const ColumnIndex index, bool isKey) { return AddColumnWithKey(index, sizeof(s16), isKey, DTYPE_INT16); }
	inline bool AddColumnInt32(const ColumnIndex index, bool isKey) { return AddColumnWithKey(index, sizeof(s32), isKey, DTYPE_INT32); }
	inline bool AddColumnInt64(const ColumnIndex index, bool isKey) { return AddColumnWithKey(index, sizeof(s64), isKey, DTYPE_INT64); }
    inline bool AddColumnString(const ColumnIndex index, const s32 maxlen, bool isKey) { return AddColumnWithKey(index, maxlen, isKey, DTYPE_STRING); }
    inline bool AddColumnFloat(const ColumnIndex index) { return nullptr != AddColumn(index, sizeof(float), DTYPE_FLOAT); }

    bool AddColumnStruct(const ColumnIndex index, const s32 size) {
        return nullptr != AddColumn(index, size, DTYPE_STRUCT);
    }

	inline bool AddColumnWithKey(const ColumnIndex index, const s32 size, bool isKey, const s32 type) {
		if (isKey) {
			OASSERT(key.type >= DTYPE_CANT_BE_KEY, "key is already set");
			if (key.type < DTYPE_CANT_BE_KEY)
				return false;
		}

		const ColumnInfo * columnInfo = AddColumn(index, size, type);
		if (nullptr == columnInfo) {
			return false;
		}

		if (isKey) {
			key.type = type;
			key.index = columnInfo->index;
		}
		return true;
	}

    inline const ColumnInfo * AddColumn(const ColumnIndex index, const s32 size, const s8 mask) {
        if (index != _columnIndex.size()) {
            OASSERT(false, "column index error");
            return nullptr;
        }

        ColumnInfo info(_columnIndex.size(), _memorySize, size, mask);
        _columnIndex.push_back(info);
		_memorySize += size;
        return &_columnIndex[_columnIndex.size() - 1];
    }

    inline const ColumnInfo * QueryColumnInfo(const ColumnIndex index) const {
        if (index >= (s32)_columnIndex.size()) {
            OASSERT(false, "column index over flow");
            return nullptr;
        }
        return &_columnIndex[index];
    }

    inline s32 CalcMemorySize() const { return _memorySize; }

    inline void Copy(const TableColumnInfo & target) {
        SafeMemcpy(&key, sizeof(key), &target.key, sizeof(target.key));
		_columnIndex.assign(target._columnIndex.cbegin(), target._columnIndex.cend());
		_memorySize = target._memorySize;
    }

    KeyInfo key;
private:
    COLUMN_INDEX _columnIndex;
    s32 _memorySize;
};

class TableRowPool;
class TableRow {
public:
    inline bool SetValue(const ColumnIndex index, const s8 mask, const void * value, const s32 size) {
        const ColumnInfo * info = _columnInfo->QueryColumnInfo(index);
        if (nullptr == info) {
            OASSERT(false, "wtf");
            return false;
        }

        OASSERT(info->mask == mask, "value type error");
        OASSERT(info->size >= size, "value size over flow");

        SafeMemcpy(_buff + info->offset, info->size, value, size);
        return true;
    }

    inline const void * GetValue(const ColumnIndex index, const s8 mask, s32 & size) {
        const ColumnInfo * info = _columnInfo->QueryColumnInfo(index);
        if (nullptr == info) {
            OASSERT(false, "wtf");
            return nullptr;
        }

        OASSERT(info->mask == mask, "value type error");

        size = info->size;
        return _buff + info->offset;
    }

    inline void Copy(const void * data, const s32 size) {
        OASSERT(size == _buffSize, "wtf");
        SafeMemcpy(_buff, _buffSize, data, size);
    }

    inline const void * GetData() { return _buff; }

    inline s32 GetSize() {return _buffSize;}

private:
    friend TableRowPool;
    inline bool SetColumnInfo(const TableColumnInfo * columnInfo) {
        if (columnInfo->CalcMemorySize() != _buffSize && _buffSize != 0) {
            OASSERT(false, "memory size diff");
            return false;
        }

		_columnInfo = columnInfo;
        SafeMemset(_buff, _buffSize, 0, _buffSize);
        return true;
    }

    inline void Clear() { SafeMemset(_buff, _buffSize, 0, _buffSize);}

    TableRow(const TableColumnInfo * columnInfo) : _columnInfo(columnInfo), _buff(nullptr), _buffSize(columnInfo->CalcMemorySize()) {
        OASSERT(columnInfo && columnInfo->CalcMemorySize() > 0 && _buffSize == columnInfo->CalcMemorySize(), "nullptrptr ColumnInfo");
		_buff = NEW char[_buffSize];
        SafeMemset(_buff, _buffSize, 0, _buffSize);
    }

    virtual ~TableRow() {
        DEL[] _buff;
    }

private:
    const TableColumnInfo * _columnInfo;
    char * _buff;
    const s32 _buffSize;
};

#endif //defined __TableStruct_h__
