#include "TableRow.h"
#include "Memory.h"
#include "TableControl.h"
#include "ObjectMgr.h"
#include "TableProp.h"
#include "MMObject.h"

bool TableDescriptor::LoadFrom(const olib::IXmlObject& root) {
	const olib::IXmlObject& columns = root["column"];
	for (s32 j = 0; j < columns.Count(); ++j) {
		const char * name = columns[j].GetAttributeString("name");
		const char * type = columns[j].GetAttributeString("type");
		bool key = columns[j].GetAttributeBoolean("key");

		if (!strcmp(type, "s8"))
			AddLayout(name, DTYPE_INT8, _size, sizeof(s8), key);
		else if (!strcmp(type, "s16"))
			AddLayout(name, DTYPE_INT16, _size, sizeof(s16), key);
		else if (!strcmp(type, "s32"))
			AddLayout(name, DTYPE_INT32, _size, sizeof(s32), key);
		else if (!strcmp(type, "s64"))
			AddLayout(name, DTYPE_INT64, _size, sizeof(s64), key);
		else if (!strcmp(type, "float"))
			AddLayout(name, DTYPE_FLOAT, _size, sizeof(float), key);
		else if (!strcmp(type, "string")) {
			s32 size = columns[j].GetAttributeInt32("size");
			OASSERT(size > 1, "wtf");
			AddLayout(name, DTYPE_STRING, _size, size, key);
		}
		else if (!strcmp(type, "struct")) {
			s32 size = columns[j].GetAttributeInt32("size");
			OASSERT(size > 0, "wtf");
			AddLayout(name, DTYPE_STRUCT, _size, size, key);
		}
		else if (!strcmp(type, "blob")) {
			s32 size = columns[j].GetAttributeInt32("size");
			OASSERT(size > 0, "wtf");
			AddLayout(name, DTYPE_BLOB, _size, size, key);
		}
		else {
			OASSERT(false, "what's this");
			return false;
		}
	}
	return true;
}

void TableDescriptor::SetupColumn(ITable * table, s32 typeId) {
	s32 idx = 0;
	for (auto& layout : _layouts) {
		layout.col = ((TableProp*)table)->AddColumn(layout.name.c_str(), typeId, idx++);
	}
}

TableRow::TableRow(TableControl * table, const TableDescriptor * descriptor) {
	_table = table;
	_descriptor = descriptor;
	_memory = MemoryPool::Instance()->Create<Memory>(__FILE__, __LINE__, descriptor->CalMemorySize());

	if (table && table->GetHost())
		_typeId = ((MMObject*)(table->GetHost()))->GetDescriptor()->GetTypeId();
	else
		_typeId = 0;
}

TableRow::~TableRow() {
	MemoryPool::Instance()->Recover(_memory);
}

