/* 
 * File:   main.cpp
 * Author: alax
 *
 * Created on March 3, 2015, 10:44 AM
 */

#ifndef __MMObject_h__
#define __MMObject_h__
#include "ObjectStruct.h"
#include "TableControl.h"
#include "OString.h"
#include "IObjectMgr.h"
#include <unordered_map>
#include <list>
#include "OCallback.h"
#include "OPool.h"

#define MAX_TYPE_LEN 64
class MMObject : public IObject {
    typedef olib::Pool<MMObject> OBJECT_POOL;
    typedef olib::CallbackType<s32, PropCallback>::type PROP_CB_POOL;
    typedef olib::CallbackType<s32, StatusCallback>::type STATUS_CB_POOL;
    typedef olib::CallbackType<s32, StatusJudgeCallback>::type STATUS_JUDEGCB_POOL;
	typedef std::unordered_map<s32, TableControl*> TABLE_MAP;
public:
    static MMObject * Create(const char * name, const s64 oObjectID, const ObjectPropInfo * pInfo) {
		return CREATE_FROM_POOL(s_objectPool, name, oObjectID, pInfo);
    }

    void Release() {
		s_objectPool.Recover(this);
    }

	virtual const s64 GetID() const { return _objectId; }
	virtual const char * GetTypeName() const { return _name.GetString(); }

	virtual const bool IsShadow() const { return _isShadow; }
	void SetShadow(const bool b) { _isShadow = b; }

	virtual const PROP_INDEX & GetPropsInfo(bool noParent) const {
		return _objectPropInfo->GetPropsInfo(noParent);
	}

    virtual bool SetPropInt8(const s32 prop, const s8 value, const bool sync);
    virtual bool SetPropInt16(const s32 prop, const s16 value, const bool sync);
    virtual bool SetPropInt32(const s32 prop, const s32 value, const bool sync);
    virtual bool SetPropInt64(const s32 prop, const s64 value, const bool sync);
    virtual bool SetPropFloat(const s32 prop, const float value, const bool sync);
    virtual bool SetPropString(const s32 prop, const char * value, const bool sync);
    virtual bool SetPropStruct(const s32 prop, const void * value, const s32 len, const bool sync);

    virtual s8 GetPropInt8(const s32 prop) const;
    virtual s16 GetPropInt16(const s32 prop) const;
    virtual s32 GetPropInt32(const s32 prop) const;
    virtual s64 GetPropInt64(const s32 prop) const;
    virtual float GetPropFloat(const s32 prop) const;
    virtual const char * GetPropString(const s32 prop) const;
	virtual const void * GetPropStruct(const s32 prop, s32& len) const;

    virtual void RgsPropChangeCB(const s32 name, const PropCallback& cb, const char * info);

    virtual ITableControl * CreateTable(const s32 name, const char * file, const s32 line);
    virtual ITableControl * FindTable(const s32 name) const ;
    virtual bool RemoveTable(const s32 name);

    //×´Ì¬»ú
    virtual void RgsEntryCB(const s32 status, const StatusCallback& cb, const char * debuginfo);
    virtual void RgsLeaveCB(const s32 status, const StatusCallback& cb, const char * debuginfo);
    virtual void RgsEntryJudegCB(const s32 status, const StatusJudgeCallback& cb, const char * debuginfo);
    virtual void RgsLeaveJudegCB(const s32 status, const StatusJudgeCallback& cb, const char * debuginfo);
    virtual bool EntryStatus(const s32 status, const bool b);
	virtual const s32 GetStatus() const { return _status; }

private:
    MMObject(const char * name, const s64 objectId, const ObjectPropInfo * info) 
		: _name(name), _objectId(objectId), _isShadow(false), _objectPropInfo(info), _status(0) {
        _objectProps = ObjectPropsPool::Instance()->Create(info);
		_objectProps->Clear();
    }

    ~MMObject() {
		_propCBPool.Clear();
		_statusEntryCBPool.Clear();
		_statusLeaveCBPool.Clear();
        _statusEntryJudegCBPool.Clear();
        _statusLeaveJudegCBPool.Clear();

        ObjectPropsPool::Instance()->Recover(_objectProps);

		for (auto itr = _tableMap.begin(); itr != _tableMap.end(); ++itr)
			itr->second->Release();
        _tableMap.clear();
    }

    void PropCall(const s32 prop, const PropInfo * info, const bool sync);

private:
    const olib::OString<MAX_TYPE_LEN> _name;
    const s64 _objectId;
	bool _isShadow;

    const ObjectPropInfo * _objectPropInfo;
    ObjectProps * _objectProps;
	PROP_CB_POOL _propCBPool;
	
	s32 _status;
    STATUS_CB_POOL _statusEntryCBPool;
    STATUS_CB_POOL _statusLeaveCBPool;
    STATUS_JUDEGCB_POOL _statusEntryJudegCBPool;
    STATUS_JUDEGCB_POOL _statusLeaveJudegCBPool;
    
	TABLE_MAP _tableMap;

    friend OBJECT_POOL;
    static OBJECT_POOL s_objectPool;
};

#endif //define __MMObject_h__
