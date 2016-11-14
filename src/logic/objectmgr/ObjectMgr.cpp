#include "ObjectMgr.h"
#include "MMObject.h"
#include "tools.h"
#include "IHarbor.h"
#include "TableControl.h"
#include "XmlReader.h"
#include "IIdMgr.h"

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
    ObjectPropsPool::Instance()->Release();
    TableRowPool::GetInterface()->Release();

    {
		for (auto itr = _propMap.begin(); itr != _propMap.end(); ++itr) {
			//DEL itr->second.;
		}
    }
	return false;
}

const ObjectMgr::ObjectModel * ObjectMgr::QueryTemplate(IKernel * kernel, const char * name) {
    OBJECT_MODEL_MAP::const_iterator itor = _propMap.find(name);
    if (itor != _propMap.end())
        return &(itor->second);

    return CreateTemplate(kernel, name);
}

const ObjectMgr::ObjectModel * ObjectMgr::CreateTemplate(IKernel * kernel, const char * name) {
    ObjectPropInfo * objectPropInfo = nullptr;
    TABLE_MODEL_LIST * tableList = NEW TABLE_MODEL_LIST();
    const char * path = nullptr;
    {
        NAME_PATH_MAP::const_iterator itor = _namePathMap.find(name);
        if (itor == _namePathMap.cend()) {
            OASSERT(false, "wtf");
            return nullptr;
        }
        path = itor->second.GetString();
    }

	olib::XmlReader conf;
    if (!conf.LoadXml(path)) {
        OASSERT(false, "prop xml file %s load file", path);
        return nullptr;
    }

	if (conf.Root().HasAttribute("parent")) {
        const ObjectModel * parentPropInfo = QueryTemplate(kernel, conf.Root().GetAttributeString("parent"));
        OASSERT(parentPropInfo, "where is parent %s xml", conf.Root().GetAttributeString("parent"));
        if (nullptr == parentPropInfo) {
            return nullptr;
        }
    
		objectPropInfo = NEW ObjectPropInfo(parentPropInfo->objectPropInfo);
		tableList->assign(parentPropInfo->tableModels->cbegin(), parentPropInfo->tableModels->cend());
    } else {
		objectPropInfo = NEW ObjectPropInfo;
    }

	const olib::IXmlObject& props = conf.Root()["prop"];
	for (s32 i = 0; i < props.Count(); ++i) {
		const char * name = props[i].GetAttributeString("name");
		const char * typeStr = props[i].GetAttributeString("type");
		s32 size, type;
		if (!strcmp(typeStr, "s8")) {
			size = sizeof(s8);
			type = DTYPE_INT8;
		}
		else if (!strcmp(typeStr, "s16")) {
			size = sizeof(s16);
			type = DTYPE_INT16;
		}
		else if (!strcmp(typeStr, "s32")) {
			size = sizeof(s32);
			type = DTYPE_INT32;
		}
		else if (!strcmp(typeStr, "s64")) {
			size = sizeof(s64);
			type = DTYPE_INT64;
		}
		else if (!strcmp(typeStr, "float")) {
			size = sizeof(float);
			type = DTYPE_FLOAT;
		}
		else if (!strcmp(typeStr, "string")) {
			size = props[i].GetAttributeInt32("size");
			type = DTYPE_STRING;
		}
		else if (!strcmp(typeStr, "blob")) {
			size = props[i].GetAttributeInt32("size");
			type = DTYPE_STRUCT;
		}
		else if (!strcmp(typeStr, "blob")) {
			size = props[i].GetAttributeInt32("size");
			type = DTYPE_BLOB;
		}
		else {
			OASSERT(false, "what's this");
			return false;
		}

		s32 setting = 0;
		for (auto itr = _defines.begin(); itr != _defines.end(); ++itr) {
			if (props[i].HasAttribute(itr->first.GetString()) && props[i].GetAttributeBoolean(itr->first.GetString()))
				setting |= itr->second;
		}
		objectPropInfo->AddProp(tools::CalcStringUniqueId(name), size, type, setting);
	}

	if (conf.Root().IsExist("table")) {
		const olib::IXmlObject& tables = conf.Root()["table"];
		for (s32 i = 0; i < tables.Count(); ++i) {
			TableColumnInfo * tableColumnInfo = NEW TableColumnInfo();
			const char * name = tables[i].GetAttributeString("name");
			TableInfo * info = NEW TableInfo({ tools::CalcStringUniqueId(name), tableColumnInfo });

			const olib::IXmlObject& columns = tables[i]["column"];
			for (s32 j = 0; j < columns.Count(); ++j) {
				const char * type = columns[j].GetAttributeString("type");
				bool key = columns[j].GetAttributeBoolean("key");

				if (!strcmp(type, "s8")) {
					tableColumnInfo->AddColumnInt8(j, key);
				}
				else if (!strcmp(type, "s16")) {
					tableColumnInfo->AddColumnInt16(j, key);
				}
				else if (!strcmp(type, "s32")) {
					tableColumnInfo->AddColumnInt32(j, key);
				}
				else if (!strcmp(type, "s64")) {
					tableColumnInfo->AddColumnInt64(j, key);
				}
				else if (!strcmp(type, "float")) {
					tableColumnInfo->AddColumnFloat(j);
				}
				else if (!strcmp(type, "string")) {
					s32 size = columns[j].GetAttributeInt32("size");
					OASSERT(size > 1, "wtf");
					tableColumnInfo->AddColumnString(j, size, key);
				}
				else if (!strcmp(type, "blob")) {
					s32 size = columns[j].GetAttributeInt32("size");
					OASSERT(size > 0, "wtf");
					tableColumnInfo->AddColumnStruct(j, size);
				}
				else {
					OASSERT(false, "what's this");
					return false;
				}
			}

			tableList->push_back(info);
		}
	}

	_propMap.insert(std::make_pair(name, ObjectModel({ tableList, objectPropInfo })));

    return &(_propMap[name]);
}

IObject * ObjectMgr::FindObject(const s64 id) {
    OBJCET_MAP::iterator itor = _objects.find(id);
    if (itor == _objects.end()) {
        return nullptr;
    }

    return itor->second.object;
}

IObject * ObjectMgr::Create(const char * file, const s32 line, const char * name, bool shadow) {
	return CreateObjectByID(file, line, name, _idMgr->AllocId(), shadow);
}

IObject * ObjectMgr::CreateObjectByID(const char * file, const s32 line, const char * name, const s64 id, bool shadow) {
	OBJECT_MODEL_MAP::const_iterator citor = _propMap.find(name);
	if (citor == _propMap.end()) {
		OASSERT(false, "what's this, %s", name);
		return nullptr;
	}

	const ObjectModel * info = &(citor->second);
	if (_objects.find(id) != _objects.end()) {
		OASSERT(false, "object id is exists");
		return nullptr;
	}

	MMObject * object = MMObject::Create(name, id, info->objectPropInfo);
	OASSERT(object, "create object error");
	if (nullptr == object) {
		return nullptr;
	}
	object->SetShadow(shadow);

	TABLE_MODEL_LIST::const_iterator itor = info->tableModels->cbegin();
	TABLE_MODEL_LIST::const_iterator iend = info->tableModels->cend();
	for (auto itr = info->tableModels->cbegin(); itr != info->tableModels->cend(); ++itr) {
		TableControl * table = (TableControl *)object->CreateTable((*itr)->name, file, line);
		OASSERT(table, "wtf");
		if (table)
			table->FormingWithTableInfo((*itor)->columnInfo);
	}

	_objects.insert(std::make_pair(id, ObjectCreateInfo({ object, file, line })));
	return object;
}

void ObjectMgr::Recove(IObject * pObject) {
    OASSERT(pObject, "wtf");
    if (nullptr == pObject) {
        return;
    }

    OBJCET_MAP::iterator itor = _objects.find(pObject->GetID());
    if (itor == _objects.end()) {
        OASSERT(false, "where is this object %lld", pObject->GetID());
        return;
    }

    itor->second.object->Release();
    _objects.erase(itor);
}

s32 ObjectMgr::CalcProp(const char * name) {
	return tools::CalcStringUniqueId(name);
}

const PROP_INDEX * ObjectMgr::GetPropsInfo(const char * type, bool noFather) const {
    OBJECT_MODEL_MAP::const_iterator const_itor = _propMap.find(type);
    if (const_itor != _propMap.end()) {
        return &(const_itor->second.objectPropInfo->GetPropsInfo(noFather));
    }

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
