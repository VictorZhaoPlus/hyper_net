#include "TableRow.h"
#include "Memory.h"
#include "TableControl.h"
#include "ObjectMgr.h"

TableRow::TableRow(TableControl * table, TableDescriptor * descriptor) {
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
