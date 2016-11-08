/* 
 * File:   ObjectStruct.h
 * Author: Alax
 *
 * Created on March 3, 2015, 10:46 AM
 */
#ifndef __ObjectStruct_h__
#define __ObjectStruct_h__
#include "util.h"
#include <list>
#include <unordered_map>
#include "singleton.h"
#include "IObjectMgr.h"

typedef s32 PropIndex;
typedef s32 MemOffset;
class TableControl;
class ObjectPropInfo {
    typedef std::unordered_map<s32, PropInfo> PROP_MAP;
public:
	ObjectPropInfo() : _memorySize(0) {

    }

	explicit ObjectPropInfo(const ObjectPropInfo * pInfo) : _propIndex(pInfo->_propIndex), _propMap(pInfo->_propMap), _memorySize(pInfo->_memorySize) {

    }

    const PropInfo * AddProp(const s32 name, const s32 size, const s8 type, const s32 setting) {
        if (_propMap.find(name) != _propMap.end()) {
            OASSERT(false, "column index exists");
            return NULL;
        }

		PropInfo info = { _memorySize, size, type, setting };
        _propMap.insert(std::make_pair(name, info));
		_propIndex.push_back(info);
		_propIndexWithOutParent.push_back(info);
		_memorySize += size;
        return &_propIndex[_propIndex.size() - 1];
    }

    const PropInfo * QueryPropInfo(const s32 name) const {
        PROP_MAP::const_iterator itor = _propMap.find(name);
        if (itor == _propMap.end()) {
            OASSERT(false, "can't query column by name %d", name);
            return NULL;
        }

        return &(itor->second);
    }

    inline s32 CalcMemorySize() const {
        return _memorySize;
    }

    inline const PROP_INDEX & GetPropsInfo(const bool noParent) const {
        if (noParent) {
            return _propIndexWithOutParent;
        }
        return _propIndex;
    }

private:
    PROP_INDEX _propIndex;
    PROP_INDEX _propIndexWithOutParent;
    PROP_MAP _propMap;
    s32 _memorySize;
};

class ObjectPropsPool;
class ObjectProps {
public:
    const PropInfo *  SetValue(const s32 name, const s8 type, const void * pValue, const s32 size) {
        const PropInfo * info = _propInfo->QueryPropInfo(name);
        if (NULL == info) {
            OASSERT(false, "wtf");
            return NULL;
        }

        OASSERT(info->type == type, "value type error");
        OASSERT(info->size >= size, "value size over flow");

        SafeMemcpy(_buff + info->offset, info->size, pValue, size);
        return info;
    }

    const void * GetValue(const s32 name, const s8 type, s32 & size) const {
        const PropInfo * info = _propInfo->QueryPropInfo(name);
        if (NULL == info) {
            OASSERT(false, "wtf");
            return NULL;
        }

        OASSERT(info->type == type, "value type error");

        size = info->size;
        return _buff + info->offset;
    }

    inline s32 GetSize() { return _buffSize; }
    inline void Clear() { SafeMemset(_buff, _buffSize, 0, _buffSize); }

private:
    friend ObjectPropsPool;
    inline bool SetPropInfo(const ObjectPropInfo * propInfo) {
        if (propInfo->CalcMemorySize() != _buffSize && _buffSize != 0) {
            OASSERT(false, "memory size diff");
            return false;
        }

        _propInfo = propInfo;
        return true;
    }

    ObjectProps(const ObjectPropInfo * propInfo) : _propInfo(propInfo), _buff(NULL), _buffSize(propInfo->CalcMemorySize()) {
        OASSERT(propInfo && propInfo->CalcMemorySize() > 0, "nullptr PropInfo");
		_buff = NEW char[_buffSize];
        SafeMemset(_buff, propInfo->CalcMemorySize(), 0, propInfo->CalcMemorySize());
    }

    virtual ~ObjectProps() {
        DEL[] _buff;
    }


private:
    const ObjectPropInfo * _propInfo;
    char * _buff;
    const s32 _buffSize;
};

class ObjectPropsPool : public OSingleton<ObjectPropsPool> {
	friend class OSingleton<ObjectPropsPool>;
public:
	bool Ready() { return true; }
    void Release() {
        auto itrPool = _propsPool.begin();
        while (itrPool != _propsPool.end()) {
            auto itr = itrPool->second.begin();
            while (itr != itrPool->second.end()) {
                --_totalCount;
                OASSERT(_totalCount >= 0, "wtf");
                DEL *itr;
				itr = itrPool->second.erase(itr);
            }

			++itrPool;
        }
    }

    ObjectProps * Create(const ObjectPropInfo * objectPropInfo) {
        s32 size = objectPropInfo->CalcMemorySize();
        OASSERT(size > 0, "wtf");
        auto itr = _propsPool.find(size);
        if (itr == _propsPool.end() || itr->second.empty()) {
			++_totalCount;
            return NEW ObjectProps(objectPropInfo);
        }

        ObjectProps * objectProps = *(itr->second.begin());
		itr->second.pop_front();
		objectProps->SetPropInfo(objectPropInfo);
        return objectProps;
    }

    void Recover(ObjectProps * objectProps) {
        auto itr = _propsPool.find(objectProps->GetSize());
        if (itr == _propsPool.end()) {
			_propsPool.insert(make_pair(objectProps->GetSize(), std::list<ObjectProps *>()));
			itr = _propsPool.find(objectProps->GetSize());
        }
		itr->second.push_back(objectProps);
    }

private:
    ObjectPropsPool() {
		_totalCount = 0;
    }

    ~ObjectPropsPool() {

    }

private:
	std::unordered_map<s32, std::list<ObjectProps *>> _propsPool;
    s32 _totalCount;
};


#endif //defined __ObjectStruct_h__
