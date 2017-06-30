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

bool ObjectMgr::Initialize(IKernel * kernel) {
	_kernel = kernel;
	_nextTypeId = 1;

	char path[512] = { 0 };
	SafeSprintf(path, sizeof(path), "%s/config/object.xml", tools::GetAppPath());
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
		}
	}
    
    SafeSprintf(path, sizeof(path), "%s/config/dccenter/", tools::GetAppPath());
	tools::ListFileInDirection(path, ".xml", [this](const char * name, const char * path) {
		if (_namePathMap.end() != _namePathMap.find(name)) {
			OASSERT(false, "prop xml name repeated");
			return;
		}
		_namePathMap.insert(std::make_pair(name, path));
	});

	for (auto itr = _namePathMap.begin(); itr != _namePathMap.end(); ++itr)
		CreateTemplate(kernel, itr->first.c_str());

	for (auto itr = _props.begin(); itr != _props.end(); ++itr)
		_propIds[tools::CalcStringUniqueId(itr->first.c_str())] = itr->second;

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

const IProp * ObjectMgr::CalcProp(const char * name) {
	auto itr = _props.find(name);
	OASSERT(itr != _props.end(), "wtf");
	if (itr != _props.end())
		return itr->second;
	return nullptr;
}

const IProp * ObjectMgr::CalcProp(const s32 name) {
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

const std::vector<const IProp*>* ObjectMgr::GetPropsInfo(const char * type, bool noFather) const {
	auto itr = _models.find(type);
    if (itr != _models.end())
        return &(itr->second->GetPropsInfo(noFather));

    return nullptr;
}

s32 ObjectMgr::ParseSetting(va_list args) {
	s32 setting = 0;
	const char * s = nullptr;
	while ((s = va_arg(args, const char *)) != nullptr){
		auto itr = _defines.find(s);
		if (itr != _defines.end())
			setting |= itr->second;
	}
	return setting;
}

void ObjectMgr::ExtendInt8(const char * type, const char * module, const char * name, ...) {
	std::string realName = std::string(module) + "." + name;

	va_list args;
	va_start(args, name);
	s32 setting = ParseSetting(args);
	va_end(args);

	TravelGroup(_kernel, type, [&realName, setting](ObjectDescriptor * descriptor, bool self) {
		descriptor->AddProp(realName.c_str(), DTYPE_INT8, sizeof(s8), setting, self);
	});
}

void ObjectMgr::ExtendInt16(const char * type, const char * module, const char * name, ...) {
	std::string realName = std::string(module) + "." + name;

	va_list args;
	va_start(args, name);
	s32 setting = ParseSetting(args);
	va_end(args);

	TravelGroup(_kernel, type, [&realName, setting](ObjectDescriptor * descriptor, bool self) {
		descriptor->AddProp(realName.c_str(), DTYPE_INT16, sizeof(s16), setting, self);
	});
}

void ObjectMgr::ExtendInt32(const char * type, const char * module, const char * name, ...) {
	std::string realName = std::string(module) + "." + name;

	va_list args;
	va_start(args, name);
	s32 setting = ParseSetting(args);
	va_end(args);

	TravelGroup(_kernel, type, [&realName, setting](ObjectDescriptor * descriptor, bool self) {
		descriptor->AddProp(realName.c_str(), DTYPE_INT32, sizeof(s32), setting, self);
	});
}

void ObjectMgr::ExtendInt64(const char * type, const char * module, const char * name, ...) {
	std::string realName = std::string(module) + "." + name;

	va_list args;
	va_start(args, name);
	s32 setting = ParseSetting(args);
	va_end(args);

	TravelGroup(_kernel, type, [&realName, setting](ObjectDescriptor * descriptor, bool self) {
		descriptor->AddProp(realName.c_str(), DTYPE_INT64, sizeof(s64), setting, self);
	});
}

void ObjectMgr::ExtendFloat(const char * type, const char * module, const char * name, ...) {
	std::string realName = std::string(module) + "." + name;

	va_list args;
	va_start(args, name);
	s32 setting = ParseSetting(args);
	va_end(args);

	TravelGroup(_kernel, type, [&realName, setting](ObjectDescriptor * descriptor, bool self) {
		descriptor->AddProp(realName.c_str(), DTYPE_FLOAT, sizeof(float), setting, self);
	});
}

void ObjectMgr::ExtendString(const char * type, const char * module, const char * name, const s32 size, ...) {
	std::string realName = std::string(module) + "." + name;

	va_list args;
	va_start(args, name);
	s32 setting = ParseSetting(args);
	va_end(args);

	TravelGroup(_kernel, type, [&realName, setting, size](ObjectDescriptor * descriptor, bool self) {
		descriptor->AddProp(realName.c_str(), DTYPE_STRING, size, setting, self);
	});
}

void ObjectMgr::ExtendStruct(const char * type, const char * module, const char * name, const s32 size, const PropFunc& init, const PropFunc& uninit) {
	std::string realName = std::string(module) + "." + name;

	TravelGroup(_kernel, type, [&realName, size, &init, &uninit](ObjectDescriptor * descriptor, bool self) {
		auto * prop = descriptor->AddProp(realName.c_str(), DTYPE_STRUCT, size, 0, self);
		if (init || uninit)
			descriptor->SetupInitial(prop, init, uninit);
	});
}

TableDescriptor * ObjectMgr::ParseTable(s32 count, va_list args) {
	TableDescriptor * table = NEW TableDescriptor();
	for (s32 i = 0; i < count; ++i) {
		s8 type = va_arg(args, s8);
		s32 size = 0;
		switch (type) {
		case DTYPE_INT8: size = sizeof(s8); break;
		case DTYPE_INT16: size = sizeof(s16); break;
		case DTYPE_INT32: size = sizeof(s32); break;
		case DTYPE_INT64: size = sizeof(s64); break;
		case DTYPE_FLOAT: size = sizeof(float); break;
		default: size = va_arg(args, s32); break;
		}
		bool key = va_arg(args, bool);
		
		table->AddLayout(type, size, key);
	}
	return table;
}

void ObjectMgr::ExtendTable(const char * type, const char * name, s32 count, ...) {
	va_list args;
	va_start(args, count);
	TableDescriptor * table = ParseTable(count, args);
	va_end(args);

	s32 tableName = CalcTableName(name);
	TravelGroup(_kernel, type, [tableName, table](ObjectDescriptor * descriptor, bool self) {
		descriptor->AddTable(tableName, table);
	});
}

ITableControl * ObjectMgr::CreateStaticTable(const char * name, const char * model, const char * file, const s32 line) {
	if (_tableMap.find(tools::CalcStringUniqueId(name)) != _tableMap.end()) {
		OASSERT(false, "already hsa table %s", name);
		return nullptr;
	}

	auto itr = _tableModels.find(tools::CalcStringUniqueId(model));
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

const IProp* ObjectMgr::SetObjectProp(const char* name, const s32 typeId, ObjectLayout * layout) {
	ObjectProp * prop = nullptr;
	auto itr = _props.find(name);
	if (itr != _props.end())
		prop = itr->second;
	else {
		prop = NEW ObjectProp(tools::CalcStringUniqueId(name), name, (s32)_namePathMap.size());
		_props[name] = prop;
	}

	prop->SetLayout(typeId, layout);
	return prop;
}
