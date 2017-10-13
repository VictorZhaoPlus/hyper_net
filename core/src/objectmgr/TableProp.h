/* 
 * File:   TableProp.h
 * Author: Alax
 *
 * Created on March 3, 2015, 10:46 AM
 */
#ifndef __TABLEPROP_H__
#define __TABLEPROP_H__
#include "IObjectMgr.h"

class ColumnProp : public IColumn {
public:
	ColumnProp(const char * realName, s32 size) : _realName(realName), _size(size) {
		_layouts = (s32*)MALLOC((size + 1) * sizeof(s32));
		SafeMemset(_layouts, (size + 1) * sizeof(s32), 0, (size + 1) * sizeof(s32));
	}

	virtual ~ColumnProp() {
		FREE(_layouts);
	}

	virtual const char * GetRealName() const { return _realName.c_str(); }

	inline void SetLayout(const s32 objectTypeId, s32 layout) {
		OASSERT(objectTypeId >= 0 && objectTypeId <= _size, "wtf");

		_layouts[objectTypeId] = layout + 1;
	}

	inline const s32 GetLayout(const s32 objectTypeId) {
		OASSERT(objectTypeId >= 0 && objectTypeId <= _size, "wtf");
		if (objectTypeId >= 0 && objectTypeId <= _size)
			return _layouts[objectTypeId];

		return 0;
	}

private:
	std::string _realName;
	s32* _layouts;
	s32 _size;
};

class TableProp : public ITable {
public:
	TableProp(const char * realName, s32 size) : _realName(realName), _size(size) {
		_layouts = (s32*)MALLOC((size + 1) * sizeof(s32));
		SafeMemset(_layouts, (size + 1) * sizeof(s32), 0, (size + 1) * sizeof(s32));
	}

	virtual ~TableProp() {
		FREE(_layouts);
	}

	virtual const char * GetRealName() const { return _realName.c_str(); }
	virtual const IColumn * GetCol(const char *) const;
	IColumn * AddColumn(const char * name, s32 typeId, s32 idx);

	inline void SetLayout(const s32 objectTypeId, s32 layout) {
		OASSERT(objectTypeId >= 0 && objectTypeId <= _size, "wtf");

		_layouts[objectTypeId] = layout + 1;
	}

	inline const s32 GetLayout(const s32 objectTypeId) {
		OASSERT(objectTypeId >= 0 && objectTypeId <= _size, "wtf");
		if (objectTypeId >= 0 && objectTypeId <= _size)
			return _layouts[objectTypeId];

		return 0;
	}
	

private:
	std::string _realName;
	s32* _layouts;
	s32 _size;

	std::unordered_map<std::string, ColumnProp *> _columns;
};

#endif //__OBJECTPROP_H__

