#include "MMObject.h"
#include "Memory.h"
#include "ObjectDescriptor.h"
#include "ObjectMgr.h"
#include "ObjectProp.h"
#include "TableControl.h"

MMObject::MMObject(const char * type, ObjectDescriptor * descriptor)
	: _type(type)
	, _objectId(0)
	, _descriptor(descriptor) {
	_memory = MemoryPool::Instance()->Create<Memory>(__FILE__, __LINE__, _descriptor->CalcMemorySize());

	descriptor->QueryTableModel([this](const s32 name, const TableDescriptor * model) {
		TableControl * table = MemoryPool::Instance()->Create<TableControl>(__FILE__, __LINE__, name, model, this);
		_tables[name] = table;
	});
}

MMObject::~MMObject() {
	_propCBPool.Clear();

	MemoryPool::Instance()->Recover(_memory);

	for (auto itr = _tables.begin(); itr != _tables.end(); ++itr) {
		MemoryPool::Instance()->Recover(itr->second);
	}
	_tables.clear();
}

const std::vector<const IProp*>& MMObject::GetPropsInfo(bool noParent) const {
	return _descriptor->GetPropsInfo(noParent);
}

ITableControl * MMObject::FindTable(const s32 name) const {
    TABLE_MAP::const_iterator itor = _tables.find(name);
    if (itor == _tables.end())
        return nullptr;
    return itor->second;
}
