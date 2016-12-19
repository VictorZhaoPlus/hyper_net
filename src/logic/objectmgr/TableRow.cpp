#include "TableRow.h"
#include "Memory.h"
#include "TableControl.h"
#include "ObjectMgr.h"

bool TableDescriptor::LoadFrom(const olib::IXmlObject& root) {
	const olib::IXmlObject& columns = root["column"];
	for (s32 j = 0; j < columns.Count(); ++j) {
		const char * type = columns[j].GetAttributeString("type");
		bool key = columns[j].GetAttributeBoolean("key");

		if (!strcmp(type, "s8"))
			AddLayout(DTYPE_INT8, _size, sizeof(s8), key);
		else if (!strcmp(type, "s16"))
			AddLayout(DTYPE_INT16, _size, sizeof(s16), key);
		else if (!strcmp(type, "s32"))
			AddLayout(DTYPE_INT32, _size, sizeof(s32), key);
		else if (!strcmp(type, "s64"))
			AddLayout(DTYPE_INT64, _size, sizeof(s64), key);
		else if (!strcmp(type, "float"))
			AddLayout(DTYPE_FLOAT, _size, sizeof(float), key);
		else if (!strcmp(type, "string")) {
			s32 size = columns[j].GetAttributeInt32("size");
			OASSERT(size > 1, "wtf");
			AddLayout(DTYPE_STRING, _size, size, key);
		}
		else if (!strcmp(type, "struct")) {
			s32 size = columns[j].GetAttributeInt32("size");
			OASSERT(size > 0, "wtf");
			AddLayout(DTYPE_STRUCT, _size, size, key);
		}
		else if (!strcmp(type, "blob")) {
			s32 size = columns[j].GetAttributeInt32("size");
			OASSERT(size > 0, "wtf");
			AddLayout(DTYPE_BLOB, _size, size, key);
		}
		else {
			OASSERT(false, "what's this");
			return false;
		}
	}
	return true;
}

TableRow::TableRow(TableControl * table, const TableDescriptor * descriptor) {
	_table = table;
	_descriptor = descriptor;
	_memory = NEW Memory(descriptor->CalMemorySize());
}

TableRow::~TableRow() {
	DEL _memory;
}

const void * TableRow::Get(const s32 col, const s8 type, s32& size) const {
	const TableLayout * info = _descriptor->Query(col, type, size);
	if (!info)
		return false;

	size = info->size;
	return _memory->Get(info);
}

void TableRow::Set(const s32 col, const s8 type, const void * data, const s32 size, bool changeKey) {
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
