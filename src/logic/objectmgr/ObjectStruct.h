/* 
 * File:   ObjectStruct.h
 * Author: Alax
 *
 * Created on March 3, 2015, 10:46 AM
 */
#ifndef __ObjectStruct_h__
#define __ObjectStruct_h__
#include "util.h"
#include "ObjectMgrMacro.h"
#include <list>
#include <unordered_map>

typedef s32 PropIndex;
typedef s32 MemOffset;
class TableControl;
class ObjectPropInfo {
    typedef std::unordered_map<s32, PropInfo> PROP_MAP;
public:
	ObjectPropInfo() : m_nMemorySize(0), m_nIndex(-1) {

    }

	explicit ObjectPropInfo(const ObjectPropInfo * pInfo) : m_oPropIndex(pInfo->m_oPropIndex), _propMap(pInfo->_propMap), m_nMemorySize(pInfo->m_nMemorySize), m_nIndex(-1) {

    }

    const PropInfo * AddProp(const s32 index, const s32 size, const s8 mask, const bool _visable, const bool _share, const bool _save, const bool _signficant, const bool _copy) {
        if (_propMap.find(index) != _propMap.end()) {
            OASSERT(false, "column index exists");
            return NULL;
        }

		PropInfo info(index, m_nMemorySize, size, mask, _visable, _share, _save, _signficant, _copy);
        _propMap.insert(std::make_pair(index, info));
        m_oPropIndex.push_back(info);
        m_oPropIndexWithOutParent.push_back(info);
        m_nMemorySize += size;
        return &m_oPropIndex[m_oPropIndex.size() - 1];
    }

    const PropInfo * QueryPropInfo(const PropIndex index) const {
        if (index >= m_oPropIndex.size()) {
            OASSERT(false, "column index over flow");
            return NULL;
        }
        return &m_oPropIndex[index];
    }

    const PropInfo * QueryPropInfo(const char * name) const {
        PROP_MAP::const_iterator itor = _propMap.find(name);
        if (itor == _propMap.end()) {
            OASSERT(false, "can't query column by name %s", name);
            return NULL;
        }

        return &(itor->second);
    }

    inline s32 CalcMemorySize() const {
        return m_nMemorySize;
    }

    inline const PROP_INDEX & GetPropsInfo(const bool noParent) const {
        if (noParent) {
            return m_oPropIndexWithOutParent;
        }
        return m_oPropIndex;
    }

private:
    PROP_INDEX m_oPropIndex;
    PROP_INDEX m_oPropIndexWithOutParent;
    PROP_MAP _propMap;
    s32 m_nMemorySize;
};

class ObjectPropsPool;
class ObjectProps {
public:
    const PropInfo *  SetValue(const char * name, const s8 mask, const void * pValue, const s32 size) {
        const PropInfo * pInfo = m_propInfo->QueryPropInfo(name);
        if (NULL == pInfo) {
            OASSERT(false, "wtf");
            return NULL;
        }

        OASSERT(pInfo->mask == mask, "value type error");
        OASSERT(pInfo->size >= size, "value size over flow");

        SafeMemcpy(m_pBuff + pInfo->offset, pInfo->size, pValue, size);
        return pInfo;
    }

    const void * GetValue(const char * name, const s8 mask, s32 & size) const {
        const PropInfo * pInfo = m_propInfo->QueryPropInfo(name);
        if (NULL == pInfo) {
            OASSERT(false, "wtf");
            return NULL;
        }

        OASSERT(pInfo->mask == mask, "value type error");

        size = pInfo->size;
        return m_pBuff + pInfo->offset;
    }

    inline s32 GetSize() {return m_nBuffSize;}
    inline void Clear() { SafeMemset(m_pBuff, m_nBuffSize, 0, m_nBuffSize); }

private:
    friend ObjectPropsPool;
    inline bool SetPropInfo(const ObjectPropInfo * propInfo) {
        if (propInfo->CalcMemorySize() != m_nBuffSize && m_nBuffSize != 0) {
            OASSERT(false, "memory size diff");
            return false;
        }

        m_propInfo = propInfo;
        return true;
    }

    ObjectProps(const ObjectPropInfo * propInfo) : m_propInfo(propInfo), m_pBuff(NULL), m_nBuffSize(propInfo->CalcMemorySize()) {
        OASSERT(propInfo && propInfo->CalcMemorySize() > 0, "nullptr PropInfo");
        m_pBuff = NEW char[m_nBuffSize];
        SafeMemset(m_pBuff, propInfo->CalcMemorySize(), 0, propInfo->CalcMemorySize());
    }

    virtual ~ObjectProps() {
        DEL[] m_pBuff;
    }


private:
    const ObjectPropInfo * m_propInfo;
    char * m_pBuff;
    const s32 m_nBuffSize;
};

class ObjectPropsPool {
    typedef std::list<ObjectProps *> OBJECT_PROPS_LIST;
    typedef std::unordered_map<s32, OBJECT_PROPS_LIST> OBJECT_PROPS_MAP;
public:
    static ObjectPropsPool * GetInterface() {
        static ObjectPropsPool * s_pSelf = NULL;
        if (NULL == s_pSelf) {
            s_pSelf = NEW ObjectPropsPool;
        }

        OASSERT(s_pSelf, "wtf");
        return s_pSelf;
    }

    void Release() {
        OBJECT_PROPS_MAP::iterator iMapTor = m_oPropsPool.begin();
        OBJECT_PROPS_MAP::iterator iMapEnd = m_oPropsPool.end();
        while (iMapTor != iMapEnd) {
            OBJECT_PROPS_LIST::iterator itor = iMapTor->second.begin();
            while (itor != iMapTor->second.end()) {
                m_nTotalCount--;
                OASSERT(m_nTotalCount >= 0, "wtf");
                DEL *itor;
                itor = iMapTor->second.erase(itor);
            }

            iMapTor++;
        }

        DEL this;
    }

    ObjectProps * Create(const ObjectPropInfo * pObjectPropInfo) {
        s32 nSize = pObjectPropInfo->CalcMemorySize();
        OASSERT(nSize > 0, "wtf");
        OBJECT_PROPS_MAP::iterator itor = m_oPropsPool.find(nSize);
        if (itor == m_oPropsPool.end() || itor->second.empty()) {
            m_nTotalCount++;
            return NEW ObjectProps(pObjectPropInfo);
        }

        ObjectProps * pObjectProps = *(itor->second.begin());
        itor->second.erase(itor->second.begin());
        pObjectProps->SetPropInfo(pObjectPropInfo);
        return pObjectProps;
    }

    void Recover(ObjectProps * pObjectProps) {
        OBJECT_PROPS_MAP::iterator itor = m_oPropsPool.find(pObjectProps->GetSize());
        if (itor == m_oPropsPool.end()) {
            m_oPropsPool.insert(make_pair(pObjectProps->GetSize(), OBJECT_PROPS_LIST()));
            itor = m_oPropsPool.find(pObjectProps->GetSize());
        }
        itor->second.push_back(pObjectProps);
    }

private:
    ObjectPropsPool() {
        m_nTotalCount = 0;
    }

    ~ObjectPropsPool() {

    }

private:
    OBJECT_PROPS_MAP m_oPropsPool;
    s32 m_nTotalCount;
};


#endif //defined __ObjectStruct_h__
