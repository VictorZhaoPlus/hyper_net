/* 
 * File:   ObjectFactory.h
 * Author: ooeyusea
 *
 * Created on Dec 6, 2016, 10:46 AM
 */

#ifndef __OBJECT_FACTORY_H__
#define __OBJECT_FACTORY_H__
#include "util.h"
#include <vector>
#include <list>
#include "OString.h"
#include "IObjectMgrExt.h"
#include "XmlReader.h"
#include <unordered_map>

class MMObject;
class MemoryLayout;
class TableColumnInfo;
class ObjectFactory {
	struct TableInfo {
		const s32 name;
		const TableColumnInfo * columnInfo;
	};
	typedef std::vector<const TableInfo *> TableModelList;

	struct PtrInfo {
		void * ptr;
		s32 size;
	};

public:
	struct Ext {
		const PropInfo * info;
		CreateOrReocverCB creator;
		CreateOrReocverCB resetor;
		CreateOrReocverCB recover;
	};

public:
	ObjectFactory(const char * name, ObjectFactory * parent);
	~ObjectFactory();

	const PROP_INDEX * GetPropsInfo(bool noFather = false) const;
	inline const std::vector<Ext>& GetExts() const { return _exts; }

	bool LoadFrom(const olib::IXmlObject& root, const std::unordered_map<olib::OString<MAX_MODEL_NAME_LEN>, s32, olib::OStringHash<MAX_MODEL_NAME_LEN>>& defines);

	MMObject * Create(s64 id, bool shadow);
	void Recover(MMObject * object);

private:
	void Alloc();

	bool LoadProps(const olib::IXmlObject& props, const std::unordered_map<olib::OString<MAX_MODEL_NAME_LEN>, s32, olib::OStringHash<MAX_MODEL_NAME_LEN>>& defines);
	bool LoadSections(const olib::IXmlObject& sections);
	bool LoadTables(const olib::IXmlObject& tables);

private:
	olib::OString<MAX_MODEL_NAME_LEN> _type;
	MemoryLayout * _layout;
	std::vector<Ext> _exts;
	TableModelList * _tables;

	s32 _nextSize;
	std::list<MMObject*> _objects;
	std::vector<PtrInfo> _ptrs;
};

#endif //__OBJECT_FACTORY_H__

