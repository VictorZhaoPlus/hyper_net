#include "ObjectMgr.h"
#include "MMObject.h"
#include "tools.h"
#include "IHarbor.h"
#include "TableControl.h"
#include "OCID.h"
#include "tinyxml/tinyxml.h"
#include "NodeDefine.h"

ObjectMgr * ObjectMgr::s_self = nullptr;
IKernel * ObjectMgr::s_kernel = nullptr;
IHarbor * ObjectMgr::s_harbor = nullptr;

bool ObjectMgr::Initialize(IKernel * kernel) {
	s_self = this;
    s_harbor = (IHarbor *)kernel->FindModule("Harbor");
    s_kernel = kernel;
    OASSERT(s_kernel && s_harbor, "where is kernel?");

    char cPath[512] = { 0 };
    SafeSprintf(cPath, sizeof(cPath), "%s/config/dccenter/", tools::GetAppPath());
    tools::VECTOR_PATH paths;
    tools::VECTOR_NAME names;
    s32 nCount = 0;

    if (tools::ListFileInDircetory(cPath, ".xml", paths, names, nCount)) {
        OASSERT(paths.size() == nCount && names.size() == nCount, "wtf");
        for (s32 i = 0; i < nCount; i++) {
            if (_namePathMap.end() != _namePathMap.find(names[i].c_str())) {
                OASSERT(false, "prop xml name repeated");
                return false;
            }

            _namePathMap.insert(std::make_pair(names[i].c_str(), paths[i].c_str()));
        }

        for (s32 i = 0; i < nCount; i++) {
            CreateTemplate(kernel, names[i].c_str());
        }
    }

    return true;
}

bool ObjectMgr::Launched(IKernel * kernel) {


    return true;
}

bool ObjectMgr::Destroy(IKernel * kernel) {
    {
        OBJCET_MAP::const_iterator itor = _objects.begin();
        while (itor != _objects.end()) {
            DBG_INFO("%s %d object leak", itor->second.file, itor->second.line);

            itor++;
        }
    }

    TableControl::EchoMemoryLeak(kernel);
    ObjectPropsPool::GetInterface()->Release();
    TableRowPool::GetInterface()->Release();

    {
        OBJECT_MODEL_MAP::const_iterator itor = _propMap.begin();
        while (itor != _propMap.end()) {
            //DEL itor->second.;
            itor++;
        }

    }

    //CSLEEP(100000);
	return false;
}

const ObjectMgr::ObjectModel * ObjectMgr::QueryTemplate(IKernel * kernel, const char * name) {
    OBJECT_MODEL_MAP::const_iterator itor = _propMap.find(name);
    if (itor != _propMap.end()) {
        return &(itor->second);
    }

    return CreateTemplate(kernel, name);
}

const ObjectMgr::ObjectModel * ObjectMgr::CreateTemplate(IKernel * kernel, const char * name) {
    ObjectPropInfo * pObjectPropInfo = NULL;
    TABLE_MODEL_LIST * pTables = NEW TABLE_MODEL_LIST();
    const char * path = NULL;
    {
        NAME_PATH_MAP::const_iterator itor = _namePathMap.find(name);
        if (itor == _namePathMap.cend()) {
            OASSERT(false, "wtf");
            return NULL;
        }
        path = itor->second.GetString();
    }

    TiXmlDocument xmlDocument;
    if (!xmlDocument.LoadFile(path)) {
        OASSERT(false, "prop xml file %s load file", path);
        return NULL;
    }

    TiXmlElement * root = xmlDocument.RootElement();
    OASSERT(root, "%s format error", path);
    const char * parent = root->Attribute("parent");
    if (parent != NULL) {
        const ObjectModel * parentPropInfo = QueryTemplate(kernel, parent);
        OASSERT(parentPropInfo, "where is parent %s xml", parent);
        if (NULL == parentPropInfo) {
            return NULL;
        }
    
        pObjectPropInfo = NEW ObjectPropInfo(parentPropInfo->pObjectPropInfo);
        pTables->assign(parentPropInfo->pTableModels->cbegin(), parentPropInfo->pTableModels->cend());
    } else {
        pObjectPropInfo = NEW ObjectPropInfo;
    }
    const char * pStartIndex = root->Attribute("start");
	s32 startIndex = pStartIndex ? tools::StringAsInt(pStartIndex) : 0;

    TiXmlElement * prop = root->FirstChildElement("prop");
    OASSERT(prop, "there is no prop");
    while (prop) {
        const char * name = prop->Attribute("name");
        s32 size, mask;
        const char * type = prop->Attribute("type");
        if (!strcmp(type, "s8")) {
            size = sizeof(s8);
            mask = DTYPE_INT8;
        } else if (!strcmp(type, "s16")) {
            size = sizeof(s16);
            mask = DTYPE_INT16;
        } else if (!strcmp(type, "s32")) {
            size = sizeof(s32);
            mask = DTYPE_INT32;
        } else if (!strcmp(type, "s64")) {
            size = sizeof(s64);
            mask = DTYPE_INT64;
        } else if (!strcmp(type, "float")) {
            size = sizeof(float);
            mask = DTYPE_FLOAT;
        } else if (!strcmp(type, "string")) {
            size = tools::StringAsInt(prop->Attribute("size"));
            mask = DTYPE_STRING;
        } else if (!strcmp(type, "blob")) {
            size = tools::StringAsInt(prop->Attribute("size"));
            mask = DTYPE_STRUCT;
        }
        else {
            OASSERT(false, "what's this");
            return false;
        }

        bool _visable = tools::StringAsBool(prop->Attribute("visable"));
        bool _share = tools::StringAsBool(prop->Attribute("share"));
        bool _save = tools::StringAsBool(prop->Attribute("save"));
        bool _signficant = tools::StringAsBool(prop->Attribute("signficant"));
        bool _copy = tools::StringAsBool(prop->Attribute("copy"));

        pObjectPropInfo->AddProp(startIndex++, size, mask, _visable, _share, _save, _signficant, _copy);

        prop = prop->NextSiblingElement("prop");
    }

    TiXmlElement * pTable = root->FirstChildElement("table");
    while (pTable) {
        TableColumnInfo * pTableColumnInfo = NEW TableColumnInfo();
        const char * pName = pTable->Attribute("name");
        TableInfo * pInfo = NEW TableInfo(pName, pTableColumnInfo);
        pTable->QueryBoolAttribute("copy", &(pTableColumnInfo->shadow));
        
        TiXmlElement * pColumn = pTable->FirstChildElement("column");
        OASSERT(pColumn, "wtf");
        
        s32 index = 0;
        while (pColumn) {
            const char * type = pColumn->Attribute("type");
            bool key = false;
            pColumn->QueryBoolAttribute("key", &key);
            if (!strcmp(type, "s8")) {
                pTableColumnInfo->AddColumnInt8(index++, key);
            }
            else if (!strcmp(type, "s16")) {
                pTableColumnInfo->AddColumnInt16(index++, key);
            }
            else if (!strcmp(type, "s32")) {
                pTableColumnInfo->AddColumnInt32(index++, key);
            }
            else if (!strcmp(type, "s64")) {
                pTableColumnInfo->AddColumnInt64(index++, key);
            }
            else if (!strcmp(type, "float")) {
                pTableColumnInfo->AddColumnFloat(index++);
            }
            else if (!strcmp(type, "string")) {
                s32 size = 0;
                pColumn->QueryIntAttribute("size", &size);
                OASSERT(size > 1, "wtf");
                pTableColumnInfo->AddColumnString(index++, size, key);
            }
            else if (!strcmp(type, "blob")) {
                s32 size = 0;
                pColumn->QueryIntAttribute("size", &size);
                OASSERT(size > 0, "wtf");
                pTableColumnInfo->AddColumnStruct(index++, size);
            }
            else {
                OASSERT(false, "what's this");
                return false;
            }

            pColumn = pColumn->NextSiblingElement("column");
        }
        pTables->push_back(pInfo);
        pTable = pTable->NextSiblingElement("table");
    }

    _propMap.insert(std::make_pair(name, ObjectModel(pObjectPropInfo, pTables)));

//     SYS_IMP("============== %s prop enum end =================", name);
//     pObjectPropInfo->EchoTemplate();
//     SYS_IMP("============== %s prop enum end =================", name);
    return &(_propMap[name]);
}

IObject * ObjectMgr::FindObject(const s64 id) {
    OBJCET_MAP::iterator itor = _objects.find(id);
    if (itor == _objects.end()) {
        return NULL;
    }

    return itor->second.pObject;
}

s64 ObjectMgr::AllotID() {
    return OCID::CreateID(s_harbor->GetAreaId(), s_harbor->GetNodeType() - nodetype::USER, s_harbor->GetNodeId());
}

IObject * ObjectMgr::Create(const char * file, const s32 line, const char * name, bool shadow) {
    OBJECT_MODEL_MAP::const_iterator citor = _propMap.find(name);
    if (citor == _propMap.end()) {
        OASSERT(false, "what's this, %s", name);
        return NULL;
    }

    const ObjectModel * pInfo = &(citor->second);

	s64 ocid = OCID::CreateID(s_harbor->GetAreaId(), s_harbor->GetNodeType() - nodetype::USER, s_harbor->GetNodeId());
    if (_objects.find(ocid) != _objects.end()) {
        OASSERT(false, "object id is exists");
        return NULL;
    }

    MMObject * pObject = MMObject::Create(name, ocid, pInfo->pObjectPropInfo);
    OASSERT(pObject, "create object error");
    if (NULL == pObject) {
        return NULL;
    }
    pObject->SetShadow(shadow);

    TABLE_MODEL_LIST::const_iterator itor = pInfo->pTableModels->cbegin();
    TABLE_MODEL_LIST::const_iterator iend = pInfo->pTableModels->cend();
    while (itor != iend) {
        if (shadow && !(*itor)->pColumnInfo->shadow) {
            itor++;
            continue;
        }

        TableControl * pTable = (TableControl *)pObject->CreateTable((*itor)->name.GetString(), file, line);
        OASSERT(pTable, "wtf");
        if (pTable) {
            pTable->FormingWithTableInfo((*itor)->pColumnInfo);
        }

        itor++;
    }

    _objects.insert(make_pair(ocid, ObjectCreateInfo(pObject, file, line)));

    return pObject;
}

IObject * ObjectMgr::CreateObjectByID(const char * file, const s32 line, const char * name, const s64 id, bool shadow) {
    OBJECT_MODEL_MAP::const_iterator citor = _propMap.find(name);
    if (citor == _propMap.end()) {
        OASSERT(false, "what's this, %s", name);
        return NULL;
    }
    const ObjectModel * pInfo = &(citor->second);
    if (_objects.find(id) != _objects.end()) {
        OASSERT(false, "object id is exists");
        return NULL;
    }

    MMObject * pObject = MMObject::Create(name, id, pInfo->pObjectPropInfo);
    OASSERT(pObject, "create object error");
    if (NULL == pObject) {
        return NULL;
    }
    pObject->SetShadow(shadow);

    TABLE_MODEL_LIST::const_iterator itor = pInfo->pTableModels->cbegin();
    TABLE_MODEL_LIST::const_iterator iend = pInfo->pTableModels->cend();
    while (itor != iend) {
        if (shadow && !(*itor)->pColumnInfo->shadow) {
            itor++;
            continue;
        }

        TableControl * pTable = (TableControl *)pObject->CreateTable((*itor)->name.GetString(), file, line);
        OASSERT(pTable, "wtf");
        if (pTable) {
            pTable->FormingWithTableInfo((*itor)->pColumnInfo);
        }

        itor++;
    }

    _objects.insert(make_pair(id, ObjectCreateInfo(pObject, file, line)));

    return pObject;
}

void ObjectMgr::Recove(IObject * pObject) {
    OASSERT(pObject, "wtf");
    if (NULL == pObject) {
        return;
    }

    OBJCET_MAP::iterator itor = _objects.find(pObject->GetID());
    if (itor == _objects.end()) {
        OASSERT(false, "where is this object %ld", pObject->GetID());
        return;
    }

    itor->second.pObject->Release();
    _objects.erase(itor);
}

const PROP_INDEX * ObjectMgr::GetPropsInfo(const char * type, bool noFather) const {
    OBJECT_MODEL_MAP::const_iterator const_itor = _propMap.find(type);
    if (const_itor != _propMap.end()) {
        return &(const_itor->second.pObjectPropInfo->GetPropsInfo(noFather));
    }

    return NULL;
}

ITableControl * ObjectMgr::FindStaticTable(const char * name) {
    TABLE_MAP::iterator itor = _tableMap.find(name);
    if (itor == _tableMap.end()) {
        OASSERT(false, "table is not exists");
        return NULL;
    }

    return itor->second;
}

void ObjectMgr::RecoverStaticTable(ITableControl * pTable) {
    ((TableControl*)pTable)->Release();
}

ITableControl * ObjectMgr::CreateStaticTable(const char * name, const char * file, const s32 line) {
    TABLE_MAP::iterator itor = _tableMap.find(name);
    if (itor != _tableMap.end()) {
        OASSERT(false, "table already exists");
        return NULL;
    }

    TableControl * table = TableControl::Create(name, NULL, file, line);
    _tableMap.insert(std::make_pair(name, table));
    return table;
}
