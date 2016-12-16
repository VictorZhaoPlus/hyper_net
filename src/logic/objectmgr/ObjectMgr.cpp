#include "ObjectMgr.h"
#include "MMObject.h"
#include "tools.h"
#include "IHarbor.h"
#include "TableControl.h"
#include "XmlReader.h"
#include "IIdMgr.h"
#include "ObjectFactory.h"

bool ObjectMgr::Initialize(IKernel * kernel) {
	_kernel = kernel;

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

    TableControl::EchoMemoryLeak(kernel);
    TableRowPool::GetInterface()->Release();

    {
		for (auto itr = _propMap.begin(); itr != _propMap.end(); ++itr) {
			//DEL itr->second.;
		}
    }
	return false;
}

ObjectFactory * ObjectMgr::QueryTemplate(IKernel * kernel, const char * name) {
    auto itr = _propMap.find(name);
    if (itr != _propMap.end())
        return itr->second;

    return CreateTemplate(kernel, name);
}

ObjectFactory * ObjectMgr::CreateTemplate(IKernel * kernel, const char * name) {
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

	ObjectFactory * factory = nullptr;
	if (conf.Root().HasAttribute("parent")) {
		ObjectFactory * parent = QueryTemplate(kernel, conf.Root().GetAttributeString("parent"));
        OASSERT(parent, "where is parent %s xml", conf.Root().GetAttributeString("parent"));
        if (nullptr == parent)
            return nullptr;
    
		factory = NEW ObjectFactory(name, parent);
    } else {
		factory = NEW ObjectFactory(name, nullptr);
    }

	if (!factory->LoadFrom(conf.Root(), _defines)) {
		DEL factory;
		return nullptr;
	}

	_propMap[name] = factory;
    return _propMap[name];
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
	
	auto itr = _propMap.find(name);
	if (itr == _propMap.end()) {
		OASSERT(false, "what's this, %s", name);
		return nullptr;
	}

	MMObject * object = itr->second->Create(id, shadow);
	OASSERT(object, "create object error");
	if (nullptr == object)
		return nullptr;

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

	((MMObject*)object)->Release();
    _objects.erase(itr);
}

s32 ObjectMgr::CalcProp(const char * name) {
	return tools::CalcStringUniqueId(name);
}

const PROP_INDEX * ObjectMgr::GetPropsInfo(const char * type, bool noFather) const {
	auto itr = _propMap.find(type);
    if (itr != _propMap.end())
        return itr->second->GetPropsInfo(noFather);

    return nullptr;
}

ITableControl * ObjectMgr::FindStaticTable(const s32 name) {
    TABLE_MAP::iterator itor = _tableMap.find(name);
    if (itor == _tableMap.end()) {
        OASSERT(false, "table is not exists");
        return nullptr;
    }

    return itor->second;
}

void ObjectMgr::RecoverStaticTable(ITableControl * pTable) {
    ((TableControl*)pTable)->Release();
}

ITableControl * ObjectMgr::CreateStaticTable(const s32 name, const char * file, const s32 line) {
    TABLE_MAP::iterator itor = _tableMap.find(name);
    if (itor != _tableMap.end()) {
        OASSERT(false, "table already exists");
        return nullptr;
    }

    TableControl * table = TableControl::Create(name, nullptr, file, line);
    _tableMap.insert(std::make_pair(name, table));
    return table;
}

void ObjectMgr::RgsExt(const s32 ext, const s32 size) {
	_exts[ext] = { ext, size, nullptr, nullptr, nullptr };
}

void ObjectMgr::RgsExtCreator(const s32 ext, const CreateOrReocverCB& creator, const char * debug) {
	_exts[ext].creator = creator;
}

void ObjectMgr::RgsExtResetor(const s32 ext, const CreateOrReocverCB& resetor, const char * debug) {
	_exts[ext].resetor = resetor;
}

void ObjectMgr::RgsExtRecover(const s32 ext, const CreateOrReocverCB& recover, const char * debug) {
	_exts[ext].recover = recover;
}

const ObjectMgr::Ext * ObjectMgr::FindExt(const s32 ext) const {
	auto itr = _exts.find(ext);
	if (itr != _exts.end())
		return &(itr->second);
	return nullptr;
}
