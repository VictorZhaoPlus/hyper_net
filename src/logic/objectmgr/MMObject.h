/* 
 * File:   main.cpp
 * Author: alax
 *
 * Created on March 3, 2015, 10:44 AM
 */

#ifndef __MMObject_h__
#define __MMObject_h__
#include "ObjectStruct.h"
#include "TCallBack.h"
#include "TableControl.h"
#include <map>
#include <list>
#include "OCStatus.h"
#include "TString.h"

class MMObject : public IObject {
    struct AnyCallInfo {
        AnyCallInfo(const prop_cb _cb, const char * _name) : cb(_cb), name(_name) {}
        const prop_cb cb;
        const tlib::TString<128> name;
    };
    typedef tlib::TPool<MMObject> OBJECT_POOL;
    typedef tlib::TCallBack<tlib::TString<game::MAX_NAME_LEN>, prop_cb, IKernel *, IObject *, const char *, const char *, const PropInfo *, const bool> PROP_CB_POOL;
    typedef tlib::TCallBack<s32, status_cb, IKernel *, IObject *> STATUS_CB_POOL;
    typedef tlib::TCallBack<s32, status_judeg_cb, IKernel *, IObject *> STATUS_JUDEGCB_POOL;
    typedef std::list<AnyCallInfo> ANY_CALL_LIST;
public:
    static MMObject * Create(const char * name, const s64 oObjectID, const ObjectPropInfo * pInfo) {
		return CREATE_FROM_POOL(s_oObjectPool, name, oObjectID, pInfo);
    }

    void Release() {
        s_oObjectPool.Recover(this);
    }

    virtual const s64 GetID() const;
    virtual const char * GetTypeName() const;
    virtual const bool IsShadow() const {
        return m_bShadow;
    }

    void SetShadow(const bool b) {
        m_bShadow = b;
    }

    virtual const PROP_INDEX & GetPropsInfo(bool noParent) const;

    virtual bool SetPropInt8(const char * prop, const s8 value, const bool sync);
    virtual bool SetPropInt16(const char * prop, const s16 value, const bool sync);
    virtual bool SetPropInt32(const char * prop, const s32 value, const bool sync);
    virtual bool SetPropInt64(const char * prop, const s64 value, const bool sync);
    virtual bool SetPropFloat(const char * prop, const float value, const bool sync);
    virtual bool SetPropString(const char * prop, const char * value, const bool sync);
    virtual bool SetPropStruct(const char * prop, const void * value, const s32 len, const bool sync);

    virtual s8 GetPropInt8(const char * prop) const;
    virtual s16 GetPropInt16(const char * prop) const;
    virtual s32 GetPropInt32(const char * prop) const;
    virtual s64 GetPropInt64(const char * prop) const;
    virtual float GetPropFloat(const char * prop) const;
    virtual const char * GetPropString(const char * prop) const;
	virtual const void * GetPropStruct(const char * prop, s32& len) const;

    //增加临时属性
    virtual bool AddTempInt8(const char * temp);
    virtual bool AddTempInt16(const char * temp);
    virtual bool AddTempInt32(const char * temp);
    virtual bool AddTempInt64(const char * temp);
    virtual bool AddTempFloat(const char * temp);
    virtual bool AddTempString(const char * temp, const s32 maxlen);
    virtual bool AddTempStruct(const char * temp, const s32 size);
    virtual bool AddTempBlob(const char * temp, const s32 size);
    virtual bool FormingTemp();

    //设置临时属性值
    virtual bool SetTempInt8(const char * temp, const s8 value);
    virtual bool SetTempInt16(const char * temp, const s16 value);
    virtual bool SetTempInt32(const char * temp, const s32 value);
    virtual bool SetTempInt64(const char * temp, const s64 value);
    virtual bool SetTempFloat(const char * temp, const float value);
    virtual bool SetTempString(const char * temp, const char * value);
    virtual bool SetTempStruct(const char * temp, void * pValue, const s32 size);
    virtual bool SetTempBlob(const char * temp, const void * pValue, const s32 size);

    //获取临时属性
    virtual s8 GetTempInt8(const char * temp) const;
    virtual s16 GetTempInt16(const char * temp) const;
    virtual s32 GetTempInt32(const char * temp) const;
    virtual s64 GetTempInt64(const char * temp) const;
    virtual float GetTempFloat(const char * temp) const;
    virtual const char * GetTempString(const char * temp) const;
    virtual void * GetTempStruct(const char * temp, const s32 size) const;
    virtual const void * GetTempBlob(const char * temp, OUT s32 & size) const;

    virtual bool RgsPropChangeCB(const char * name, const prop_cb cb, const char * debug_info);


    virtual const TABLE_MAP & GetTables() const {
        return _tableMap;
    }
    virtual ITableControl * CreateTable(const char * name, const char * file, const s32 line);
    virtual ITableControl * FindTable(const char * name) const ;
    virtual bool RemoveTable(const char * name);

    //状态机
    virtual bool RgsEntryCB(const s32 status, const status_cb cb, const char * debuginfo);
    virtual bool RgsLeaveCB(const s32 status, const status_cb cb, const char * debuginfo);
    virtual bool RgsEntryJudegCB(const s32 status, const status_judeg_cb cb, const char * debuginfo);
    virtual bool RgsLeaveJudegCB(const s32 status, const status_judeg_cb cb, const char * debuginfo);
    virtual bool EntryStatus(const s32 status, const bool b);
    virtual const s32 GetStatus() const {
        return m_nStatus;
    }

private:
    MMObject(const char * name, const s64 oObjectID, const ObjectPropInfo * pInfo) : m_oName(name), m_oObjectID(oObjectID), m_pObjectPropInfo(pInfo), m_oTempInfo(), m_pObjectTemps(NULL), m_nStatus(OCStatus::OBJECT_STATUS_IDLE) {
        m_pObjectProps = ObjectPropsPool::GetInterface()->Create(pInfo);
        m_pObjectProps->Clear();
    }

    ~MMObject() {
        m_oPropCBPool.Clear();
        m_oStatusEntryCBPool.Clear();
        m_oStatusLeaveCBPool.Clear();
        m_oStatusEntryJudegCBPool.Clear();
        m_oStatusLeaveJudegCBPool.Clear();
        m_oAnyCallList.clear();

        ObjectPropsPool::GetInterface()->Recover(m_pObjectProps);
        if (m_pObjectTemps) {
            ObjectPropsPool::GetInterface()->Recover(m_pObjectTemps);
        }

        TABLE_MAP::iterator itor = _tableMap.begin();
        TABLE_MAP::iterator iend = _tableMap.end();
        while (itor != iend) {
            ((TableControl *)itor->second)->Release();

            itor++;
        }

        _tableMap.clear();
    }

    void PropAnyCBCall(const char * prop, const PropInfo * pInfo, const bool sync);

    void PropCall(const char * prop, const PropInfo * pInfo, const bool sync);

private:
    const tlib::TString<game::MAX_NAME_LEN> m_oName;
    const s64 m_oObjectID;
    const ObjectPropInfo * m_pObjectPropInfo;
    bool m_bShadow;
    ObjectPropInfo m_oTempInfo;
    ObjectProps * m_pObjectTemps;

    ObjectProps * m_pObjectProps;
    TABLE_MAP _tableMap;

    STATUS_CB_POOL m_oStatusEntryCBPool;
    STATUS_CB_POOL m_oStatusLeaveCBPool;
    STATUS_JUDEGCB_POOL m_oStatusEntryJudegCBPool;
    STATUS_JUDEGCB_POOL m_oStatusLeaveJudegCBPool;
    s32 m_nStatus;

    PROP_CB_POOL m_oPropCBPool;
    ANY_CALL_LIST m_oAnyCallList;

    friend OBJECT_POOL;
    static OBJECT_POOL s_oObjectPool;
};

#endif //define __MMObject_h__
