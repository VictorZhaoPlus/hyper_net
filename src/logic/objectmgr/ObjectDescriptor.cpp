#include "ObjectDescriptor.h"
#include "MMObject.h"
#include "TableControl.h"
#include "Memory.h"
#include "ObjectMgr.h"
#include "TableRow.h"

ObjectDescriptor::ObjectDescriptor(s32 typeId, const char * name, ObjectDescriptor * parent)
	: _typeId(typeId)
	, _type(name) {
	if (parent) {
		_layouts = parent->_layouts;
		for (auto& layout : _layouts) {
			const IProp * prop = ObjectMgr::Instance()->SetObjectProp(layout.name.c_str(), _typeId, &layout);
			_props.push_back(prop);
		}
		_size = parent->_size;
		_tables = parent->_tables;
	}
	else
		_size = 0;
}

bool ObjectDescriptor::LoadFrom(const olib::IXmlObject& root, const std::unordered_map<std::string, s32>& defines) {
	if (!LoadProps(root["prop"], defines))
		return false;

	if (root.IsExist("table")) {
		if (!LoadTables(root["table"]))
			return false;
	}
	return true;
}

bool ObjectDescriptor::LoadProps(const olib::IXmlObject& props, const std::unordered_map<std::string, s32>& defines) {
	for (s32 i = 0; i < props.Count(); ++i) {
		const char * name = props[i].GetAttributeString("name");
		const char * typeStr = props[i].GetAttributeString("type");

		ObjectLayout layout;
		layout.offset = _size;
		layout.name = name;
		if (!strcmp(typeStr, "s8")) {
			layout.size = sizeof(s8);
			layout.type = DTYPE_INT8;
		}
		else if (!strcmp(typeStr, "s16")) {
			layout.size = sizeof(s16);
			layout.type = DTYPE_INT16;
		}
		else if (!strcmp(typeStr, "s32")) {
			layout.size = sizeof(s32);
			layout.type = DTYPE_INT32;
		}
		else if (!strcmp(typeStr, "s64")) {
			layout.size = sizeof(s64);
			layout.type = DTYPE_INT64;
		}
		else if (!strcmp(typeStr, "float")) {
			layout.size = sizeof(float);
			layout.type = DTYPE_FLOAT;
		}
		else if (!strcmp(typeStr, "string")) {
			layout.size = props[i].GetAttributeInt32("size");
			layout.type = DTYPE_STRING;
		}
		else if (!strcmp(typeStr, "blob")) {
			layout.size = props[i].GetAttributeInt32("size");
			layout.type = DTYPE_STRUCT;
		}
		else if (!strcmp(typeStr, "blob")) {
			layout.size = props[i].GetAttributeInt32("size");
			layout.type = DTYPE_BLOB;
		}
		else {
			OASSERT(false, "what's this");
			return false;
		}

		layout.setting = 0;
		for (auto itr = defines.begin(); itr != defines.end(); ++itr) {
			if (props[i].HasAttribute(itr->first.c_str()) && props[i].GetAttributeBoolean(itr->first.c_str()))
				layout.setting |= itr->second;
		}
		_layouts.push_back(layout);
		_size += layout.size;

		const IProp * prop = ObjectMgr::Instance()->SetObjectProp(name, _typeId, &(*_layouts.rbegin()));
		_props.push_back(prop);
		_selfProps.push_back(prop);
	}

	return true;
}

bool ObjectDescriptor::LoadTables(const olib::IXmlObject& tables) {
	for (s32 i = 0; i < tables.Count(); ++i) {
		TableDescriptor * tableModel = NEW TableDescriptor();
		const char * name = tables[i].GetAttributeString("name");
		if (!tableModel->LoadFrom(tables[i]))
			return false;

		TableInfo info = { tools::CalcStringUniqueId(name), tableModel };
		_tables.push_back(info);
	}
	return true;
}
