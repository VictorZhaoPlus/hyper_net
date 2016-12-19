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
	, _descriptor(descriptor){
	_memory = NEW Memory(_descriptor->CalcMemorySize());

	descriptor->QueryTableModel([this](const s32 name, const TableDescriptor * model) {
		TableControl * table = NEW TableControl(name, model);
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

bool MMObject::Set(const IProp * prop, const s8 type, const void * data, const s32 size, const bool sync) {
	const ObjectLayout * layout = ((ObjectProp*)prop)->GetLayout(_descriptor->GetTypeId());
	OASSERT(layout, "wtf");
	if (layout != nullptr) {
		OASSERT(layout->type == type && layout->size >= size, "wtf");

		if (layout->type == type && layout->size >= size) {
			_memory->Set(layout, data, size);
			return true;
		}
		PropCall(prop, sync);
	}
	return false;
}

const void *  MMObject::Get(const IProp * prop, const s8 type, s32& size) const {
	const ObjectLayout * layout = ((ObjectProp*)prop)->GetLayout(_descriptor->GetTypeId());
	OASSERT(layout, "wtf");
	if (layout != nullptr) {
		OASSERT(layout->type == type && layout->size >= size, "wtf");

		if (layout->type == type && layout->size >= size) {
			size = layout->size;
			return _memory->Get(layout);
		}
	}
	return nullptr;
}

ITableControl * MMObject::FindTable(const s32 name) const {
    TABLE_MAP::const_iterator itor = _tables.find(name);
    if (itor == _tables.end())
        return nullptr;
    return itor->second;
}

void MMObject::PropCall(const IProp * prop, const bool sync) {
	_propCBPool.Call(prop, ObjectMgr::Instance()->GetKernel(), this, _type.GetString(), prop, sync);
	_propCBPool.Call(nullptr, ObjectMgr::Instance()->GetKernel(), this, _type.GetString(), prop, sync);
}
