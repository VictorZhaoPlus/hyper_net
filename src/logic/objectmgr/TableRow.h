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
#include "TableControl.h"
#include "ObjectMgr.h"

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

	inline const void * Get(const s32 col, const s8 type, s32& size) const {
		const TableLayout * info = _descriptor->Query(col, type, size);
		if (!info)
			return nullptr;

		size = info->size;
		return _memory->Get(info);
	}
	virtual s8 GetDataInt8(const s32 col) const { s32 size = sizeof(s8); return *(s8*)Get(col, DTYPE_INT8, size); }
	virtual s16 GetDataInt16(const s32 col) const { s32 size = sizeof(s16); return *(s8*)Get(col, DTYPE_INT16, size); }
	virtual s32 GetDataInt32(const s32 col) const { s32 size = sizeof(s32); return *(s8*)Get(col, DTYPE_INT32, size); }
	virtual s64 GetDataInt64(const s32 col) const { s32 size = sizeof(s64); return *(s8*)Get(col, DTYPE_INT64, size); }
	virtual float GetDataFloat(const s32 col) const { s32 size = sizeof(float); return *(s8*)Get(col, DTYPE_FLOAT, size); }
	virtual const char * GetDataString(const s32 col) const { s32 size = 0; return (const char *)Get(col, DTYPE_STRING, size); }
	virtual const void * GetDataStruct(const s32 col, const s32 size) const { s32 tempSize = size; return (const char *)Get(col, DTYPE_STRUCT, tempSize); }
	virtual const void * GetDataBlob(const s32 col, s32& size) const { size = 0; return (const char *)Get(col, DTYPE_BLOB, size); }

	inline void Set(const s32 col, const s8 type, const void * data, const s32 size, bool changeKey = true) {
		const TableLayout * info = _descriptor->Query(col, type, size);
		if (!info)
			return;

		if (changeKey) {
			if (info->key) {
				switch (info->type) {
				case DTYPE_STRING: _table->ChangeKey((const char *)_memory->Get(info), (const char *)data, info->type); break;
				case DTYPE_INT8: _table->ChangeKey(*(s8*)_memory->Get(info), *(s8*)data, info->type); break;
				case DTYPE_INT16: _table->ChangeKey(*(s16*)_memory->Get(info), *(s16*)data, info->type); break;
				case DTYPE_INT32: _table->ChangeKey(*(s32*)_memory->Get(info), *(s32*)data, info->type); break;
				case DTYPE_INT64: _table->ChangeKey(*(s64*)_memory->Get(info), *(s64*)data, info->type); break;
				default: OASSERT(false, "key type invalid");  break;
				}
			}
		}

		_memory->Set(info, data, size);
		_table->UpdateCallBack(ObjectMgr::Instance()->GetKernel(), this, col, info->type);
	}
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

