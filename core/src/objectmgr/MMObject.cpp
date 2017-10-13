#include "MMObject.h"
#include "Memory.h"
#include "ObjectDescriptor.h"
#include "ObjectMgr.h"
#include "ObjectProp.h"
#include "TableControl.h"
#include "TableProp.h"

MMObject::MMObject(const char * type, ObjectDescriptor * descriptor)
	: _type(type)
	, _objectId(0)
	, _descriptor(descriptor) {
	_memory = MemoryPool::Instance()->Create<Memory>(__FILE__, __LINE__, _descriptor->CalcMemorySize());

	_tables = (TableControl**)MALLOC(descriptor->CountTable() * sizeof(TableControl*));
	descriptor->QueryTableModel([this](const s64 name, const s32 idx, const TableDescriptor * model) {
		_tables[idx] = MemoryPool::Instance()->Create<TableControl>(__FILE__, __LINE__, name, model, this);
	});

	descriptor->InitObject(_memory, this);
}

MMObject::~MMObject() {
	_propCBPool.Clear();

	_descriptor->DeInitObject(_memory, this);

	MemoryPool::Instance()->Recover(_memory);

	for (s32 i = 0; i < _descriptor->CountTable(); ++i) {
		MemoryPool::Instance()->Recover(_tables[i]);
	}
	FREE(_tables);
}

const std::vector<const IProp*>& MMObject::GetPropsInfo(bool noParent) const {
	return _descriptor->GetPropsInfo(noParent);
}

ITableControl * MMObject::FindTable(const ITable* table) const {
	s32 idx = ((TableProp*)table)->GetLayout(_descriptor->GetTypeId());
	if (idx > 0)
		return _tables[idx - 1];
    return nullptr;
}
