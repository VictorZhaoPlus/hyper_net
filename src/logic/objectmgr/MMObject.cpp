#include "MMObject.h"
#include "Memory.h"
#include "ObjectDescriptor.h"
#include "ObjectMgr.h"
#include "ObjectProp.h"
#include "TableControl.h"

MMObject::MMObject(const char * type, ObjectDescriptor * descriptor)
	: _type(type)
	, _objectId(0)
	, _isShadow(false)
	, _descriptor(descriptor) {
	_memory = NEW Memory(_descriptor->CalcMemorySize());

	descriptor->QueryTableModel([this](const s32 name, const TableDescriptor * model) {
		TableControl * table = NEW TableControl(name, model, this);
		_tables[name] = table;
	});
}

MMObject::~MMObject() {
	_propCBPool.Clear();

	DEL _memory;

	for (auto itr = _tables.begin(); itr != _tables.end(); ++itr) {
		DEL itr->second;
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
