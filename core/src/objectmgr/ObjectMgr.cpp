#include "ObjectMgr.h"
#include "MMObject.h"
#include "tools.h"
#include "IHarbor.h"
#include "TableControl.h"
#include "XmlReader.h"
#include "IIdMgr.h"
#include "ObjectDescriptor.h"
#include "ObjectProp.h"
#include "TableRow.h"
#include "TableProp.h"

bool ObjectMgr::Initialize(IKernel * kernel) {
	_kernel = kernel;
	_nextTypeId = 1;

	char path[512] = { 0 };
	SafeSprintf(path, sizeof(path), "%s/config/object/object.xml", tools::GetWorkPath());
	olib::XmlReader conf;
	if (!conf.LoadXml(path)) {
		OASSERT(false, "load object.xml failed");
		return false;
	}

	if (conf.Root().IsExist("prop")) {
		const olib::IXmlObject& props = conf.Root()["prop"];
		for (s32 i = 0; i < props.Count(); ++i)
			_defines[props[i].GetAttributeString("name")] = (1 << i);
	}

	if (conf.Root().IsExist("table")) {
		const olib::IXmlObject& tables = conf.Root()["table"];
		for (s32 i = 0; i < tables.Count(); ++i) {
			TableDescriptor * tableModel = NEW TableDescriptor();
			const char * name = tables[i].GetAttributeString("name");
			if (!tableModel->LoadFrom(tables[i]))
				return false;

			_tableModels[tools::CalcStringUniqueId(name)] = tableModel;

			ITable * table = ObjectMgr::Instance()->SetTableProp("", name, 0, 0);
			tableModel->SetupColumn(table, 0);
		}
	}
    
    SafeSprintf(path, sizeof(path), "%s/config/object/model/", tools::GetWorkPath());
	tools::ListFileInDirection(path, ".xml", [this](const char * name, const char * path) {
		if (_namePathMap.end() != _namePathMap.find(name)) {
			OASSERT(false, "prop xml name repeated");
			return;
		}
		_namePathMap.insert(std::make_pair(name, path));
	});

	for (auto itr = _namePathMap.begin(); itr != _namePathMap.end(); ++itr)
		CreateTemplate(kernel, itr->first.c_str());

	for (auto itr = _props.begin(); itr != _props.end(); ++itr) {
		for (auto& prop : itr->second) {
			_propIds[(((s64)tools::CalcStringUniqueId(itr->first.c_str())) << 32) | (tools::CalcStringUniqueId(prop.first.c_str()))] = prop.second;
		}
	}

    return true;
}

bool ObjectMgr::Launched(IKernel * kernel) {
    return true;
}

bool ObjectMgr::Destroy(IKernel * kernel) {
    {
		for (auto itr = _objects.begin(); itr != _objects.end(); ++itr) {
            //DBG_INFO("%s %d object leak", itr->second.file, itr->second.line);
        }
    }

    {
		for (auto itr = _models.begin(); itr != _models.end(); ++itr) {
			//DEL itr->second.;
		}
    }
	return false;
}

ObjectDescriptor * ObjectMgr::QueryTemplate(IKernel * kernel, const char * name) {
    auto itr = _models.find(name);
    if (itr != _models.end())
        return itr->second;

    return CreateTemplate(kernel, name);
}

ObjectDescriptor * ObjectMgr::CreateTemplate(IKernel * kernel, const char * name) {
    auto itr = _namePathMap.find(name);
    if (itr == _namePathMap.cend()) {
        OASSERT(false, "wtf");
        return nullptr;
    }

	if (_models.find(name) != _models.end())
		return _models[name];

	olib::XmlReader conf;
    if (!conf.LoadXml(itr->second.c_str())) {
        OASSERT(false, "prop xml file %s load file", itr->second.c_str());
        return nullptr;
    }

	ObjectDescriptor * descriptor = nullptr;
	if (conf.Root().HasAttribute("parent")) {
		ObjectDescriptor * parent = QueryTemplate(kernel, conf.Root().GetAttributeString("parent"));
        OASSERT(parent, "where is parent %s xml", conf.Root().GetAttributeString("parent"));
        if (nullptr == parent)
            return nullptr;
    
		descriptor = NEW ObjectDescriptor(_nextTypeId++, name, parent);

		_groups[conf.Root().GetAttributeString("parent")].insert(name);

    } else {
		descriptor = NEW ObjectDescriptor(_nextTypeId++, name, nullptr);
    }

	if (!descriptor->LoadFrom(conf.Root(), _defines)) {
		DEL descriptor;
		return nullptr;
	}

	_models[name] = descriptor;
    return _models[name];
}

void ObjectMgr::TravelGroup(IKernel * kernel, const char * name, const std::function<void(ObjectDescriptor *, bool)>& f, bool self) {
	auto itr = _models.find(name);
	if (itr == _models.end()) {
		f(itr->second, self);

		auto itrGroup = _groups.find(name);
		if (itrGroup != _groups.end()) {
			for (auto& child : itrGroup->second)
				TravelGroup(kernel, child.c_str(), f, false);
		}
	}
}

IObject * ObjectMgr::FindObject(const s64 id) {
    auto itr = _objects.find(id);
    if (itr == _objects.end())
        return nullptr;

    return itr->second.object;
}

IObject * ObjectMgr::Create(const char * file, const s32 line, const char * name) {
	return CreateObjectByID(file, line, name, OMODULE(IdMgr)->AllocId());
}

IObject * ObjectMgr::CreateObjectByID(const char * file, const s32 line, const char * name, const s64 id) {
	if (_objects.find(id) != _objects.end()) {
		OASSERT(false, "object id is exists");
		return nullptr;
	}
	
	auto itr = _models.find(name);
	if (itr == _models.end()) {
		OASSERT(false, "what's this, %s", name);
		return nullptr;
	}

	MMObject * object = MemoryPool::Instance()->Create<MMObject>(__FILE__, __LINE__, name, itr->second);
	object->SetID(id);
	_objects.insert(std::make_pair(id, ObjectCreateInfo({ object, file, line })));
	return object;
}

void ObjectMgr::Recove(IObject * object) {
    OASSERT(object, "wtf");
    if (nullptr == object)
        return;

    auto itr = _objects.find(object->GetID());
    if (itr == _objects.end()) {
        OASSERT(false, "where is this object %lld", object->GetID());
        return;
    }
	OASSERT(itr->second.object == object, "wtf");

	MemoryPool::Instance()->Recover(object);
    _objects.erase(itr);
}

const IProp * ObjectMgr::CalcProp(const char * name, const char * module) {
	auto itr = _props[module].find(name);
	OASSERT(itr != _props[module].end(), "wtf");
	if (itr != _props[module].end())
		return itr->second;
	return nullptr;
}

const IProp * ObjectMgr::CalcProp(const s64 name) {
	auto itr = _propIds.find(name);
	OASSERT(itr != _propIds.end(), "wtf");
	if (itr != _propIds.end())
		return itr->second;
	return nullptr;
}

s32 ObjectMgr::CalcPropSetting(const char * setting) {
	auto itr = _defines.find(setting);
	if (itr != _defines.end())
		return itr->second;
	return 0;
}

const ITable * ObjectMgr::CalcTable(const char * name, const char * module) {
	auto itr = _tableProps[module].find(name);
	OASSERT(itr != _tableProps[module].end(), "wtf");
	if (itr != _tableProps[module].end())
		return itr->second;
	return nullptr;
}

const std::vector<const IProp*>* ObjectMgr::GetPropsInfo(const char * type, bool noFather) const {
	auto itr = _models.find(type);
    if (itr != _models.end())
        return &(itr->second->GetPropsInfo(noFather));

    return nullptr;
}

void ObjectMgr::ExtendData(const char * type, const char * module, const char * name, const s32 dataType, const s32 size, const PropFunc& init, const PropFunc& reset, const PropFunc& uninit) {
	TravelGroup(_kernel, type, [module, name, size, dataType, &init, &reset, &uninit](ObjectDescriptor * descriptor, bool self) {
		const IProp * prop = descriptor->AddProp(module, name, dataType, size, 0, self);
		if (init || reset || uninit)
			descriptor->SetupInitial(prop, init, reset, uninit);
	});
}

ITableControl * ObjectMgr::CreateStaticTable(const char * name, const ITable * model, const char * file, const s32 line) {
	if (_tableMap.find(tools::CalcStringUniqueId(name)) != _tableMap.end()) {
		OASSERT(false, "already hsa table %s", name);
		return nullptr;
	}

	auto itr = _tableModels.find(tools::CalcStringUniqueId(model->GetRealName()));
	OASSERT(itr != _tableModels.end(), "wtf");
	if (itr == _tableModels.end())
		return nullptr;

	TableControl * table = MemoryPool::Instance()->Create<TableControl>(__FILE__, __LINE__, tools::CalcStringUniqueId(name), itr->second);
	TableCreateInfo info({ table, file, line });
	_tableMap.insert(std::make_pair(tools::CalcStringUniqueId(name), info));
    return table;
}

void ObjectMgr::RecoverStaticTable(ITableControl * table) {
	OASSERT(!table->GetHost(), "wtf");
	_tableMap.erase(((TableControl*)table)->GetName());
	MemoryPool::Instance()->Recover(table);
}

s32 ObjectMgr::CalcTableName(const char * table) {
	return tools::CalcStringUniqueId(table);
}

void ObjectMgr::RgsObjectCRCB(const char * type, const ObjectCRCB& init, const ObjectCRCB& uninit) {
	TravelGroup(_kernel, type, [&init, &uninit](ObjectDescriptor * descriptor, bool self) {
		if (init)
			descriptor->AddInit(init);

		if (uninit)
			descriptor->AddDeinit(uninit);
	});
}

const IProp* ObjectMgr::SetObjectProp(const char * module, const char* name, const s32 typeId, ObjectLayout * layout) {
	ObjectProp * prop = nullptr;
	auto itr = _props[module].find(name);
	if (itr != _props[module].end())
		prop = itr->second;
	else {
		prop = NEW ObjectProp((((s64)tools::CalcStringUniqueId(module)) << 32) | tools::CalcStringUniqueId(name), name, (s32)_namePathMap.size());
		_props[module][name] = prop;
	}

	prop->SetLayout(typeId, layout);
	return prop;
}

ITable* ObjectMgr::SetTableProp(const char * module, const char* name, const s32 typeId, s32 layout) {
	TableProp * table = nullptr;
	auto itr = _tableProps[module].find(name);
	if (itr != _tableProps[module].end())
		table = itr->second;
	else {
		table = NEW TableProp(name, (s32)_namePathMap.size());
		_tableProps[module][name] = table;
	}

	table->SetLayout(typeId, layout);

	return table;
}
