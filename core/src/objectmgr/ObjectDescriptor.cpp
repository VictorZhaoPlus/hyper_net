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
			const IProp * prop = ObjectMgr::Instance()->SetObjectProp(layout->module.c_str(), layout->name.c_str(), _typeId, NEW ObjectLayout(*layout));
			_props.push_back(prop);
		}
		_size = parent->_size;
		_tables = parent->_tables;
		for (s32 i = 0; i < (s32)_tables.size(); ++i) {
			ITable * table = ObjectMgr::Instance()->SetTableProp(_tables[i].module.c_str(), _tables[i].name.c_str(), _typeId, i);
			_tables[i].tableModel->SetupColumn(table, _typeId);
		}
	}
	else
		_size = 0;
}

bool ObjectDescriptor::LoadFrom(const olib::IXmlObject& root, const std::unordered_map<std::string, s32>& defines) {
	if (root.IsExist("prop")) {
		if (!LoadProps("", root["prop"], defines)) {
			return false;
		}
	}

	if (root.IsExist("table")) {
		if (!LoadTables("", root["table"])) {
			return false;
		}
	}

	bool ret = true;
	root.ForEach([this, &defines, &ret](const char * module, const olib::IXmlObject& unit) {
		if (strcmp(module, "prop") == 0)
			return;

		if (strcmp(module, "table") == 0)
			return;

		if (unit[0].IsExist("prop")) {
			if (!LoadProps(module, unit[0]["prop"], defines)) {
				ret = false;
				return;
			}
		}

		if (unit[0].IsExist("table")) {
			if (!LoadTables(module, unit[0]["table"])) {
				ret = false;
				return;
			}
		}
	});

	return ret;
}

bool ObjectDescriptor::LoadProps(const char * module, const olib::IXmlObject& props, const std::unordered_map<std::string, s32>& defines) {
	for (s32 i = 0; i < props.Count(); ++i) {
		const char * name = props[i].GetAttributeString("name");
		const char * typeStr = props[i].GetAttributeString("type");

		ObjectLayout * layout = NEW ObjectLayout;
		layout->offset = _size;
		layout->module = module;
		layout->name = name;
		if (!strcmp(typeStr, "s8")) {
			layout->size = sizeof(s8);
			layout->type = DTYPE_INT8;
		}
		else if (!strcmp(typeStr, "s16")) {
			layout->size = sizeof(s16);
			layout->type = DTYPE_INT16;
		}
		else if (!strcmp(typeStr, "s32")) {
			layout->size = sizeof(s32);
			layout->type = DTYPE_INT32;
		}
		else if (!strcmp(typeStr, "s64")) {
			layout->size = sizeof(s64);
			layout->type = DTYPE_INT64;
		}
		else if (!strcmp(typeStr, "float")) {
			layout->size = sizeof(float);
			layout->type = DTYPE_FLOAT;
		}
		else if (!strcmp(typeStr, "string")) {
			layout->size = props[i].GetAttributeInt32("size");
			layout->type = DTYPE_STRING;
		}
		else if (!strcmp(typeStr, "struct")) {
			layout->size = props[i].GetAttributeInt32("size");
			layout->type = DTYPE_STRUCT;
		}
		else if (!strcmp(typeStr, "blob")) {
			layout->size = props[i].GetAttributeInt32("size");
			layout->type = DTYPE_BLOB;
		}
		else {
			OASSERT(false, "what's this");
			return false;
		}

		layout->setting = 0;
		for (auto itr = defines.begin(); itr != defines.end(); ++itr) {
			if (props[i].HasAttribute(itr->first.c_str()) && props[i].GetAttributeBoolean(itr->first.c_str()))
				layout->setting |= itr->second;
		}
		_layouts.push_back(layout);
		_size += layout->size;

		const IProp * prop = ObjectMgr::Instance()->SetObjectProp(module, name, _typeId, layout);
		_props.push_back(prop);
		_selfProps.push_back(prop);
	}

	return true;
}

const IProp * ObjectDescriptor::AddProp(const char * module, const char * name, s8 type, s32 size, s32 setting, bool self) {
	ObjectLayout * layout = NEW ObjectLayout;
	layout->offset = _size;
	layout->module = module ? module : "";
	layout->name = name;
	layout->size = size;
	layout->type = type;
	layout->setting = setting;

	const IProp * prop = ObjectMgr::Instance()->SetObjectProp(module, name, _typeId, layout);
	_props.push_back(prop);
	if (self)
		_selfProps.push_back(prop);

	return prop;
}

void ObjectDescriptor::SetupInitial(const IProp * prop, const PropFunc& init, const PropFunc& reset, const PropFunc& uninit) {
	const ObjectLayout * layout = ((ObjectProp*)prop)->GetLayout(_typeId);

	_propInits.push_back({ layout, init, reset, uninit });
}

bool ObjectDescriptor::LoadTables(const char * module, const olib::IXmlObject& tables) {
	for (s32 i = 0; i < tables.Count(); ++i) {
		TableDescriptor * tableModel = NEW TableDescriptor();
		const char * name = tables[i].GetAttributeString("name");
		if (!tableModel->LoadFrom(tables[i]))
			return false;

		s64 nameId = (((s64)tools::CalcStringUniqueId(module)) << 32) | tools::CalcStringUniqueId(name);
		TableInfo info = { nameId, module, name, tableModel };
		ITable * table = ObjectMgr::Instance()->SetTableProp(module, name, _typeId, (s32)_tables.size());
		_tables.push_back(info);

		tableModel->SetupColumn(table, _typeId);
	}
	return true;
}
