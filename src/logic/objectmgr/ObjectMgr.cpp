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
		CreateTemplate(kernel, itr->first.GetString());

    return true;
}

bool ObjectMgr::Launched(IKernel * kernel) {
	FIND_MODULE(_idMgr, IdMgr);
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
    if (!conf.LoadXml(itr->second.GetString())) {
        OASSERT(false, "prop xml file %s load file", itr->second.GetString());
        return nullptr;
    }

	ObjectDescriptor * descriptor = nullptr;
	if (conf.Root().HasAttribute("parent")) {
		ObjectDescriptor * parent = QueryTemplate(kernel, conf.Root().GetAttributeString("parent"));
        OASSERT(parent, "where is parent %s xml", conf.Root().GetAttributeString("parent"));
        if (nullptr == parent)
            return nullptr;
    
		descriptor = NEW ObjectDescriptor(_nextTypeId++, name, parent);
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

IObject * ObjectMgr::FindObject(const s64 id) {
    auto itr = _objects.find(id);
    if (itr == _objects.end())
        return nullptr;

    return itr->second.object;
}

IObject * ObjectMgr::Create(const char * file, const s32 line, const char * name, bool shadow) {
	return CreateObjectByID(file, line, name, _idMgr->AllocId(), shadow);
}

IObject * ObjectMgr::CreateObjectByID(const char * file, const s32 line, const char * name, const s64 id, bool shadow) {
	if (_objects.find(id) != _objects.end()) {
		OASSERT(false, "object id is exists");
		return nullptr;
	}
	
	auto itr = _models.find(name);
	if (itr == _models.end()) {
		OASSERT(false, "what's this, %s", name);
		return nullptr;
	}

	MMObject * object = NEW MMObject(name, itr->second);
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

	DEL object;
    _objects.erase(itr);
}

const IProp * ObjectMgr::CalcProp(const char * name) {
	auto itr = _props.find(name);
	OASSERT(itr != _props.end(), "wtf");
	if (itr != _props.end())
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

ITableControl * ObjectMgr::CreateStaticTable(const char * name, const char * model, const char * file, const s32 line) {
	if (_tableMap.find(tools::CalcStringUniqueId(name)) != _tableMap.end()) {
		OASSERT(false, "already hsa table %s", name);
		return nullptr;
	}

	auto itr = _tableModels.find(tools::CalcStringUniqueId(model));
	OASSERT(itr != _tableModels.end(), "wtf");
	if (itr == _tableModels.end())
		return nullptr;

	TableControl * table = NEW TableControl(tools::CalcStringUniqueId(name), itr->second);
	TableCreateInfo info({ table, file, line });
	_tableMap.insert(std::make_pair(tools::CalcStringUniqueId(name), info));
    return table;
}

void ObjectMgr::RecoverStaticTable(ITableControl * table) {
	OASSERT(!table->GetHost(), "wtf");
	_tableMap.erase(((TableControl*)table)->GetName());
	DEL table;
}

const IProp* ObjectMgr::SetObjectProp(const char* name, const s32 typeId, ObjectLayout * layout) {
	ObjectProp * prop = nullptr;
	auto itr = _props.find(name);
	if (itr != _props.end())
		prop = itr->second;
	else {
		prop = NEW ObjectProp((s32)_namePathMap.size());
		_props[name] = prop;
	}

	prop->SetLayout(typeId, layout);
	return prop;
}
