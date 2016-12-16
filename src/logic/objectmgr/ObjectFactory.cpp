#include "ObjectFactory.h"
#include "MMObject.h"
#include "TableControl.h"
#include "Memory.h"
#include "ObjectMgr.h"

#define MIN_NEXT_SIZE 10
ObjectFactory::ObjectFactory(const char * name, ObjectFactory * parent)
	: _type(name)
	, _nextSize(MIN_NEXT_SIZE) {
	if (parent)
		_layout = NEW MemoryLayout(parent->_layout);
	else
		_layout = NEW MemoryLayout;

	if (parent && parent->_tables) {
		_tables = NEW TableModelList;
		_tables->assign(parent->_tables->begin(), parent->_tables->end());
	}
	else
		_tables = nullptr;
}

ObjectFactory::~ObjectFactory() {
	for (const auto& ptr : _ptrs) {
		for (s32 i = 0; i < ptr.size; ++i) {
			((MMObject*)ptr.ptr + i)->~MMObject();
		}
		FREE(ptr.ptr);
	}
	_ptrs.clear();

	DEL _layout;
}

const PROP_INDEX * ObjectFactory::GetPropsInfo(bool noFather) const {
	return &(_layout->GetPropsInfo(noFather));
}

bool ObjectFactory::LoadFrom(const olib::IXmlObject& root, const std::unordered_map<olib::OString<MAX_MODEL_NAME_LEN>, s32, olib::OStringHash<MAX_MODEL_NAME_LEN>>& defines) {
	if (!LoadProps(root["prop"], defines))
		return false;

	if (root.IsExist("section")) {
		if (!LoadSections(root["section"]))
			return false;
	}

	if (root.IsExist("table")) {
		if (!LoadTables(root["table"]))
			return false;
	}
	return true;
}

MMObject * ObjectFactory::Create(s64 id, bool shadow) {
	if (_objects.empty())
		Alloc();
	OASSERT(!_objects.empty(), "wtf");

	if (_objects.empty())
		return nullptr;

	MMObject * ret = (MMObject *)*_objects.begin();
	_objects.pop_front();
	ret->SetID(id);
	ret->SetShadow(shadow);
	return ret;
}

void ObjectFactory::Recover(MMObject * object) {
	OASSERT(strcmp(object->GetTypeName(), _type.GetString()) == 0, "wtf");
	object->Clear();
	_objects.push_back(object);
}

void ObjectFactory::Alloc() {
	MMObject * objects = (MMObject *)MALLOC(sizeof(MMObject) * _nextSize);
	for (s32 i = 0; i < _nextSize; ++i) {
		new (objects + i) MMObject(_type.GetString(), _layout, this);

		if (_tables) {
			for (auto itr = _tables->cbegin(); itr != _tables->cend(); ++itr) {
				TableControl * table = (TableControl *)(objects + i)->CreateTable((*itr)->name, __FILE__, __LINE__);
				OASSERT(table, "wtf");
				if (table)
					table->FormingWithTableInfo((*itr)->columnInfo);
			}
		}

		_objects.push_back(objects + i);
	}
	_ptrs.push_back({ objects, _nextSize });
	_nextSize *= 2;
}

bool ObjectFactory::LoadProps(const olib::IXmlObject& props, const std::unordered_map<olib::OString<MAX_MODEL_NAME_LEN>, s32, olib::OStringHash<MAX_MODEL_NAME_LEN>>& defines) {
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
		for (auto itr = defines.begin(); itr != defines.end(); ++itr) {
			if (props[i].HasAttribute(itr->first.GetString()) && props[i].GetAttributeBoolean(itr->first.GetString()))
				setting |= itr->second;
		}
		_layout->Add(tools::CalcStringUniqueId(name), size, type, setting);
	}

	return true;
}

bool ObjectFactory::LoadSections(const olib::IXmlObject& sections) {
	for (s32 i = 0; i < sections.Count(); ++i) {
		s32 name = tools::CalcStringUniqueId(sections[i].GetAttributeString("name"));

		const ObjectMgr::Ext * ext = ObjectMgr::Instance()->FindExt(name);
		OASSERT(ext, "find object ext %s failed", sections[i].GetAttributeString("name"));
		if (!ext)
			return false;

		const PropInfo * prop = _layout->AddExt(name, ext->size);
		_exts.push_back({ prop, ext->creator, ext->resetor, ext->recover });
	}
	return true;
}

bool ObjectFactory::LoadTables(const olib::IXmlObject& tables) {
	if (_tables == nullptr)
		_tables = NEW TableModelList;

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

		_tables->push_back(info);
	}
	return true;
}

