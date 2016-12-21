/* 
 * File:   ObjectProp.h
 * Author: Alax
 *
 * Created on March 3, 2015, 10:46 AM
 */
#ifndef __OBJECTPROP_H__
#define __OBJECTPROP_H__
#include "IObjectMgr.h"

struct ObjectLayout;
class ObjectProp : public IProp {
public:
	ObjectProp(s32 name, s32 size) : _name(name), _size(0) {
		_layouts = (ObjectLayout**)MALLOC(size * sizeof(ObjectLayout*));
		SafeMemset(_layouts, size * sizeof(ObjectLayout*), 0, size * sizeof(ObjectLayout*));
	}

	virtual ~ObjectProp() {
		FREE(_layouts);
	}

	virtual const s32 GetName() const { return _name; }
	virtual const s8 GetType(IObject * object) const;
	virtual const s32 GetSetting(IObject * object) const;

	inline void SetLayout(const s32 objectTypeId, ObjectLayout * layout) {
		OASSERT(objectTypeId > 0 && objectTypeId <= _size, "wtf");

		_layouts[objectTypeId - 1] = layout;
	}

	inline const ObjectLayout * GetLayout(const s32 objectTypeId) {
		OASSERT(objectTypeId > 0 && objectTypeId <= _size, "wtf");
		if (objectTypeId > 0 && objectTypeId <= _size)
			return _layouts[objectTypeId - 1];

		return nullptr;
	}

private:
	s32 _name;
	ObjectLayout ** _layouts;
	s32 _size;
};

#endif //__OBJECTPROP_H__

