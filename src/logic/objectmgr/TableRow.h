/* 
 * File:   TableRow.h
 * Author: Alax
 *
 * Created on March 3, 2015, 10:46 AM
 */
#ifndef __TABLEROW_H__
#define __TABLEROW_H__
#include "IObjectMgr.h"
#include "Memory.h"
#include "XmlReader.h"

struct TableLayout : public Layout {
	s8 type;
	bool key;
};

class TableDescriptor {
public:
	TableDescriptor() : _key(DTYPE_CANT_BE_KEY), _size(0), _keyCol(-1) {}
	~TableDescriptor() {}

	inline s32 CalMemorySize() const { return _size; }
	inline s8 GetKeyType() const { return _key; }
	inline s32 GetKeyCol() const { return _keyCol; }

	bool LoadFrom(const olib::IXmlObject& root);

	const TableLayout * Query(s32 col, s32 type, s32 size) const {
		OASSERT(col >= 0 && col < (s32)_layouts.size(), "wtf");
		if (col < 0 || col > (s32)_layouts.size())
			return nullptr;

		const TableLayout& layout = _layouts[col];
		OASSERT(layout.type == type && layout.size >= size, "wtf");
		if (layout.type == type && layout.size >= size)
			return &layout;
		return nullptr;
	}

private:
	void AddLayout(s8 type, s32 offset, s32 size, bool key) {
		TableLayout layout;
		layout.type = type;
		layout.offset = offset;
		layout.size = size;
		layout.key = key;

		if (key) {
			OASSERT(_key == DTYPE_CANT_BE_KEY, "wtf");
			_key = type;
			_keyCol = (s32)_layouts.size();
		}

		_layouts.push_back(layout);
		_size += size;
	}

private:
	std::vector<TableLayout> _layouts;
	s8 _key;
	s32 _keyCol;
	s32 _size;
};

class Memory;
class TableControl;
class TableRow : public IRow {
public:
	TableRow(TableControl * table, const TableDescriptor * descriptor);
	virtual ~TableRow();

	virtual s32 GetRowIndex() const { return _index; }
	void SetRowIndex(const s32 index) { _index = index; }

	const void * Get(const s32 prop, const s8 type, s32& size) const;
	virtual s8 GetDataInt8(const s32 col) const { s32 size = sizeof(s8); return *(s8*)Get(col, DTYPE_INT8, size); }
	virtual s16 GetDataInt16(const s32 col) const { s32 size = sizeof(s16); return *(s8*)Get(col, DTYPE_INT16, size); }
	virtual s32 GetDataInt32(const s32 col) const { s32 size = sizeof(s32); return *(s8*)Get(col, DTYPE_INT32, size); }
	virtual s64 GetDataInt64(const s32 col) const { s32 size = sizeof(s64); return *(s8*)Get(col, DTYPE_INT64, size); }
	virtual float GetDataFloat(const s32 col) const { s32 size = sizeof(float); return *(s8*)Get(col, DTYPE_FLOAT, size); }
	virtual const char * GetDataString(const s32 col) const { s32 size = 0; return (const char *)Get(col, DTYPE_STRING, size); }
	virtual const void * GetDataStruct(const s32 col, const s32 size) const { s32 tempSize = size; return (const char *)Get(col, DTYPE_STRUCT, tempSize); }
	virtual const void * GetDataBlob(const s32 col, s32& size) const { size = 0; return (const char *)Get(col, DTYPE_BLOB, size); }

	void Set(const s32 col, const s8 type, const void * data, const s32 size, bool changeKey = true);
	virtual void SetDataInt8(const s32 col, const s8 value) { Set(col, DTYPE_INT8, &value, sizeof(s8)); }
	virtual void SetDataInt16(const s32 col, const s16 value) { Set(col, DTYPE_INT16, &value, sizeof(s16)); }
	virtual void SetDataInt32(const s32 col, const s32 value) { Set(col, DTYPE_INT32, &value, sizeof(s32)); }
	virtual void SetDataInt64(const s32 col, const s64 value) { Set(col, DTYPE_INT64, &value, sizeof(s64)); }
	virtual void SetDataFloat(const s32 col, const float value) { Set(col, DTYPE_FLOAT, &value, sizeof(float)); }
	virtual void SetDataString(const s32 col, const char * value) { Set(col, DTYPE_STRING, value, (s32)strlen(value) + 1); }
	virtual void SetDataStruct(const s32 col, const void * value, const s32 size) { Set(col, DTYPE_STRUCT, value, size); }
	virtual void SetDataBlob(const s32 col, const void * value, const s32 size) { Set(col, DTYPE_BLOB, value, size); }

private:
	TableControl * _table;
	const TableDescriptor * _descriptor;
	s32 _index;

	Memory * _memory;
};

#endif //__TABLEROW_H__

