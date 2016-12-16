/* 
 * File:   main.cpp
 * Author: alax
 *
 * Created on March 3, 2015, 10:44 AM
 */

#ifndef __MMObject_h__
#define __MMObject_h__
#include "TableControl.h"
#include "OString.h"
#include "IObjectMgrExt.h"
#include <unordered_map>
#include <list>
#include "OCallback.h"

class MemoryLayout;
class Memory;
class ObjectFactory;
class MMObject : public IObjectExt {
    typedef olib::CallbackType<s32, PropCallback>::type PROP_CB_POOL;
	typedef std::unordered_map<s32, TableControl*> TABLE_MAP;
public:
	MMObject(const char * type, const MemoryLayout * info, ObjectFactory * factory);
	~MMObject();

	virtual const s64 GetID() const { return _objectId; }
	void SetID(s64 id) { _objectId = id; }

	virtual const char * GetTypeName() const { return _type.GetString(); }

	virtual const bool IsShadow() const { return _isShadow; }
	void SetShadow(const bool b) { _isShadow = b; }

	virtual const PROP_INDEX & GetPropsInfo(bool noParent) const;

	bool Set(const s32 prop, const s32 type, const void * data, const s32 size, const bool sync);
    virtual bool SetPropInt8(const s32 prop, const s8 value, const bool sync) { return Set(prop, DTYPE_INT8, &value, sizeof(s8), sync); }
    virtual bool SetPropInt16(const s32 prop, const s16 value, const bool sync) { return Set(prop, DTYPE_INT16, &value, sizeof(s16), sync); }
    virtual bool SetPropInt32(const s32 prop, const s32 value, const bool sync) { return Set(prop, DTYPE_INT32, &value, sizeof(s32), sync); }
    virtual bool SetPropInt64(const s32 prop, const s64 value, const bool sync) { return Set(prop, DTYPE_INT64, &value, sizeof(s64), sync); }
    virtual bool SetPropFloat(const s32 prop, const float value, const bool sync) { return Set(prop, DTYPE_FLOAT, &value, sizeof(float), sync); }
    virtual bool SetPropString(const s32 prop, const char * value, const bool sync) { return Set(prop, DTYPE_STRING, value, strlen(value) + 1, sync); }
    virtual bool SetPropStruct(const s32 prop, const void * value, const s32 size, const bool sync) { return Set(prop, DTYPE_STRUCT, value, size, sync); }

	const void * Get(const s32 prop, const s32 type, s32& size) const;
	virtual s8 GetPropInt8(const s32 prop) const { s32 size = sizeof(s8); return *(s8*)Get(prop, DTYPE_INT8, size); }
    virtual s16 GetPropInt16(const s32 prop) const { s32 size = sizeof(s16); return *(s8*)Get(prop, DTYPE_INT16, size); }
    virtual s32 GetPropInt32(const s32 prop) const { s32 size = sizeof(s32); return *(s8*)Get(prop, DTYPE_INT32, size); }
    virtual s64 GetPropInt64(const s32 prop) const { s32 size = sizeof(s64); return *(s8*)Get(prop, DTYPE_INT64, size); }
    virtual float GetPropFloat(const s32 prop) const { s32 size = sizeof(float); return *(s8*)Get(prop, DTYPE_FLOAT, size); }
    virtual const char * GetPropString(const s32 prop) const { s32 size = 0; return (const char *)Get(prop, DTYPE_STRING, size); }
	virtual const void * GetPropStruct(const s32 prop, s32& size) const { size = 0; return (const char *)Get(prop, DTYPE_STRUCT, size); }

	virtual void * GetExtData(const s32 ext, const s32 size);

    virtual void RgsPropChangeCB(const s32 name, const PropCallback& cb, const char * info) { _propCBPool.Register(name, cb, info); }

    virtual ITableControl * CreateTable(const s32 name, const char * file, const s32 line);
    virtual ITableControl * FindTable(const s32 name) const ;
    virtual bool RemoveTable(const s32 name);

	void Clear();
	void Release();

private:
    void PropCall(const s32 prop, const PropInfo * info, const bool sync);

private:
    const olib::OString<MAX_MODEL_NAME_LEN> _type;
    s64 _objectId;
	bool _isShadow;

    const MemoryLayout * _layout;
	Memory * _memory;
	ObjectFactory * _factory;

	PROP_CB_POOL _propCBPool;

	TABLE_MAP _tableMap;
};

#endif //define __MMObject_h__
