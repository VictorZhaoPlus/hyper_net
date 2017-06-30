/* 
 * File:   ObjectDescriptor.h
 * Author: ooeyusea
 *
 * Created on Dec 6, 2016, 10:46 AM
 */

#ifndef __OBJECTDESCRIPTOR_H__
#define __OBJECTDESCRIPTOR_H__
#include "util.h"
#include "IObjectMgr.h"
#include "Memory.h"
#include "XmlReader.h"
#include <vector>
#include <unordered_map>

struct ObjectLayout : public Layout {
	s8 type;
	s32 setting;
	std::string name;
};

class TableDescriptor;
class IProp;
class ObjectDescriptor {
	struct TableInfo {
		s32 name;
		TableDescriptor * tableModel;
	};

	struct PropInit {
		const Layout * layout;
		PropFunc init;
		PropFunc uninit;
	};

public:
	ObjectDescriptor(s32 typeId, const char * name, ObjectDescriptor * parent);
	~ObjectDescriptor() {}

	inline const std::vector<const IProp*>& GetPropsInfo(bool noFather = false) const { return noFather ? _selfProps : _props; }
	inline s32 CalcMemorySize() const { return _size; }
	inline s32 GetTypeId() const { return _typeId; }

	bool LoadFrom(const olib::IXmlObject& root, const std::unordered_map<std::string, s32>& defines);
	
	inline void QueryTableModel(const std::function<void(const s32 name, const TableDescriptor * model)>& f) {
		for (const auto& info : _tables)
			f(info.name, info.tableModel);
	}

	inline void InitObject(Memory * memory, IObject * object) {
		for (auto& u : _propInits) {
			if (u.init) {
				void * p = memory->Get(u.layout);
				u.init(p, u.layout->size);
			}
		}

		for (auto& f : _inits)
			f(object);
	}

	inline void DeInitObject(Memory * memory, IObject * object) {
		for (auto& f : _uninits)
			f(object);

		for (auto& u : _propInits) {
			if (u.uninit) {
				void * p = memory->Get(u.layout);
				u.uninit(p, u.layout->size);
			}
		}
	}

	const IProp * AddProp(const char * name, s8 type, s32 size, s32 setting, bool self);
	void SetupInitial(const IProp * prop, const PropFunc& init, const PropFunc& uninit);

	inline void AddTable(s32 name, TableDescriptor * tableModel) { _tables.push_back({ name, tableModel }); }

	void AddInit(const ObjectCRCB& init) { _inits.push_back(init); }
	void AddDeinit(const ObjectCRCB& uninit) { _uninits.push_back(uninit); }

private:
	bool LoadProps(const olib::IXmlObject& props, const std::unordered_map<std::string, s32>& defines);
	bool LoadTables(const olib::IXmlObject& tables);

private:
	s32 _typeId;
	std::string _type;
	std::vector<ObjectLayout*> _layouts;
	std::vector<const IProp*> _props;
	std::vector<const IProp*> _selfProps;
	s32 _size;
	std::vector<TableInfo> _tables;
	std::vector<PropInit> _propInits;
	std::vector<ObjectCRCB> _inits;
	std::vector<ObjectCRCB> _uninits;
};

#endif //__OBJECTDESCRIPTOR_H__

