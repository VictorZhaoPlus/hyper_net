#include "MMObject.h"
#include "Memory.h"
#include "ObjectFactory.h"
#include "ObjectMgr.h"

MMObject::MMObject(const char * type, const MemoryLayout * layout, ObjectFactory * factory)
	: _type(type)
	, _objectId(0)
	, _isShadow(false)
	, _layout(layout)
	, _factory(factory){
	_memory = NEW Memory(_layout->CalcMemorySize());

	for (auto ext : _factory->GetExts()) {
		void  * data = _memory->Get(ext.info);
		ext.creator(ObjectMgr::Instance()->GetKernel(), this, data, ext.info->size);
	}
}

MMObject::~MMObject() {
	_propCBPool.Clear();

	for (auto ext : _factory->GetExts()) {
		void  * data = _memory->Get(ext.info);
		ext.recover(ObjectMgr::Instance()->GetKernel(), this, data, ext.info->size);
	}

	DEL _memory;

	for (auto itr = _tableMap.begin(); itr != _tableMap.end(); ++itr)
		itr->second->Release();
	_tableMap.clear();
}

void MMObject::PropCall(const s32 prop, const PropInfo * info, const bool sync) {
    _propCBPool.Call(prop, ObjectMgr::Instance()->GetKernel(), this, _type.GetString(), prop, info, sync);
	_propCBPool.Call(ANY_CALL, ObjectMgr::Instance()->GetKernel(), this, _type.GetString(), prop, info, sync);
}

const PROP_INDEX & MMObject::GetPropsInfo(bool noParent) const {
	return _layout->GetPropsInfo(noParent);
}

bool MMObject::Set(const s32 prop, const s32 type, const void * data, const s32 size, const bool sync) {
	const PropInfo * info = _layout->Query(prop, type, size);
	if (!info)
		return false;

	_memory->Set(info, data, size);
	PropCall(prop, info, sync);
	return true;
}

const void *  MMObject::Get(const s32 prop, const s32 type, s32& size) const {
	const PropInfo * info = _layout->Query(prop, type, size);
	if (!info)
		return false;

	size = info->size;
	return _memory->Get(info);
}

void * MMObject::GetExtData(const s32 ext, const s32 size) {
	const PropInfo * info = _layout->QueryExt(ext, size);
	if (!info)
		return false;

	return _memory->Get(info);
}

ITableControl * MMObject::CreateTable(const s32 name, const char * file, const s32 line) {
    TABLE_MAP::iterator itor = _tableMap.find(name);
    if (itor != _tableMap.end()) {
        OASSERT(false, "table already exists");
        return nullptr;
    }

    TableControl * table = TableControl::Create(name, this, file, line);
    _tableMap.insert(std::make_pair(name, table));
    return table;
}

ITableControl * MMObject::FindTable(const s32 name) const {
    TABLE_MAP::const_iterator itor = _tableMap.find(name);
    if (itor == _tableMap.end())
        return nullptr;
    return itor->second;
}

bool MMObject::RemoveTable(const s32 name) {
    TABLE_MAP::iterator itor = _tableMap.find(name);
    if (itor == _tableMap.end()) {
        OASSERT(false, "table is not exists");
        return false;
    }

    ((TableControl *)itor->second)->Release();
    _tableMap.erase(itor);
    return true;
}

void MMObject::Clear() {
	for (auto ext : _factory->GetExts()) {
		void  * data = _memory->Get(ext.info);
		ext.resetor(ObjectMgr::Instance()->GetKernel(), this, data, ext.info->size);
	}

	_memory->Clear();
	_propCBPool.Clear();

	for (auto itr = _tableMap.begin(); itr != _tableMap.end(); ++itr)
		itr->second->Reset();
}

void MMObject::Release() {
	_factory->Recover(this);
}
