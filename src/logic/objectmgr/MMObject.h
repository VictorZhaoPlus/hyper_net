/* 
 * File:   main.cpp
 * Author: alax
 *
 * Created on March 3, 2015, 10:44 AM
 */

#ifndef __MMObject_h__
#define __MMObject_h__
#include "TableControl.h"
#include "IObjectMgr.h"
#include <unordered_map>
#include "OCallback.h"
#include "Memory.h"
#include "ObjectDescriptor.h"
#include "ObjectMgr.h"
#include "ObjectProp.h"

class MMObject : public IObject {
    typedef olib::CallbackType<IProp *, PropCallback>::type PROP_CB_POOL;
	typedef std::unordered_map<s32, TableControl*> TABLE_MAP;
public:
	MMObject(const char * type, ObjectDescriptor * descriptor);
	~MMObject();

	virtual const s64 GetID() const { return _objectId; }
	void SetID(s64 id) { _objectId = id; }

	virtual const char * GetTypeName() const { return _type.c_str(); }
	inline ObjectDescriptor * GetDescriptor() const { return _descriptor; }

	virtual const bool IsShadow() const { return _isShadow; }
	void SetShadow(const bool b) { _isShadow = b; }

	virtual const std::vector<const IProp*>& GetPropsInfo(bool noParent) const;

	inline bool Set(const IProp * prop, const s8 type, const void * data, const s32 size, const bool sync) {
		const ObjectLayout * layout = ((ObjectProp*)prop)->GetLayout(_descriptor->GetTypeId());
		OASSERT(layout, "wtf");
		if (layout != nullptr) {
			OASSERT(layout->type == type && layout->size >= size, "wtf");

			if (layout->type == type && layout->size >= size) {
				_memory->Set(layout, data, size);
				PropCall(prop, sync);
				return true;
			}
		}
		return false;
	}
	virtual bool SetPropInt8(const IProp * prop, const s8 value, const bool sync) { return Set(prop, DTYPE_INT8, &value, sizeof(s8), sync); }
	virtual bool SetPropInt16(const IProp * prop, const s16 value, const bool sync) { return Set(prop, DTYPE_INT16, &value, sizeof(s16), sync); }
	virtual bool SetPropInt32(const IProp * prop, const s32 value, const bool sync) { return Set(prop, DTYPE_INT32, &value, sizeof(s32), sync); }
	virtual bool SetPropInt64(const IProp * prop, const s64 value, const bool sync) { return Set(prop, DTYPE_INT64, &value, sizeof(s64), sync); }
	virtual bool SetPropFloat(const IProp * prop, const float value, const bool sync) { return Set(prop, DTYPE_FLOAT, &value, sizeof(float), sync); }
	virtual bool SetPropString(const IProp * prop, const char * value, const bool sync) { return Set(prop, DTYPE_STRING, value, (s32)strlen(value) + 1, sync); }
	virtual bool SetPropStruct(const IProp * prop, const void * value, const s32 size, const bool sync) { return Set(prop, DTYPE_STRUCT, value, size, sync); }
	virtual bool SetPropBlob(const IProp * prop, const void * value, const s32 size, const bool sync) { return Set(prop, DTYPE_BLOB, value, size, sync); }

	inline const void * Get(const IProp * prop, const s8 type, s32& size) const {
		const ObjectLayout * layout = ((ObjectProp*)prop)->GetLayout(_descriptor->GetTypeId());
		OASSERT(layout, "wtf");
		if (layout != nullptr) {
			OASSERT(layout->type == type && layout->size >= size, "wtf");

			if (layout->type == type && layout->size >= size) {
				size = layout->size;
				return _memory->Get(layout);
			}
		}
		return nullptr;
	}
	virtual s8 GetPropInt8(const IProp * prop) const { s32 size = sizeof(s8); return *(s8*)Get(prop, DTYPE_INT8, size); }
    virtual s16 GetPropInt16(const IProp * prop) const { s32 size = sizeof(s16); return *(s8*)Get(prop, DTYPE_INT16, size); }
    virtual s32 GetPropInt32(const IProp * prop) const { s32 size = sizeof(s32); return *(s8*)Get(prop, DTYPE_INT32, size); }
    virtual s64 GetPropInt64(const IProp * prop) const { s32 size = sizeof(s64); return *(s8*)Get(prop, DTYPE_INT64, size); }
    virtual float GetPropFloat(const IProp * prop) const { s32 size = sizeof(float); return *(s8*)Get(prop, DTYPE_FLOAT, size); }
    virtual const char * GetPropString(const IProp * prop) const { s32 size = 0; return (const char *)Get(prop, DTYPE_STRING, size); }
	virtual const void * GetPropStruct(const IProp * prop, const s32 size) const { s32 tmpSize = size; return (const char *)Get(prop, DTYPE_STRUCT, tmpSize); }
	virtual const void * GetPropBlob(const IProp * prop, s32& size) const { size = 0; return (const char *)Get(prop, DTYPE_BLOB, size); }

    virtual void RgsPropChangeCB(const IProp * prop, const PropCallback& cb, const char * info) { _propCBPool.Register(prop, cb, info); }

    virtual ITableControl * FindTable(const s32 name) const ;

private:
	inline void PropCall(const IProp * prop, const bool sync) {
		_propCBPool.Call(prop, ObjectMgr::Instance()->GetKernel(), this, _type.c_str(), prop, sync);
		_propCBPool.Call(nullptr, ObjectMgr::Instance()->GetKernel(), this, _type.c_str(), prop, sync);
	}

private:
    const std::string _type;
    s64 _objectId;
	bool _isShadow;

	Memory * _memory;
	ObjectDescriptor * _descriptor;
	std::unordered_map<s32, TableControl*> _tables;

	olib::CallbackType<const IProp *, PropCallback>::type _propCBPool;
};

#endif //define __MMObject_h__
