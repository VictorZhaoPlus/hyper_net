#include "MMObject.h"
#include "ObjectMgr.h"

olib::Pool<MMObject> MMObject::s_objectPool;

void MMObject::PropCall(const s32 prop, const PropInfo * info, const bool sync) {
    _propCBPool.Call(prop, ObjectMgr::Instance()->GetKernel(), this, _name.GetString(), prop, info, sync);
	_propCBPool.Call(ANY_CALL, ObjectMgr::Instance()->GetKernel(), this, _name.GetString(), prop, info, sync);
}

bool MMObject::SetPropInt8(const s32 prop, const s8 value, const bool sync) {
    const PropInfo * pInfo = _objectProps->SetValue(prop, DTYPE_INT8, &value, sizeof(value));
    if (!pInfo) {
        return false;
    }

    //OASSERT(!(!pInfo->setting.visable && sync), "wtf");
    PropCall(prop, pInfo, sync);
    return true;
}

bool MMObject::SetPropInt16(const s32 prop, const s16 value, const bool sync) {
    const PropInfo * pInfo = _objectProps->SetValue(prop, DTYPE_INT16, &value, sizeof(value));
    if (!pInfo) {
        return false;
    }

    //OASSERT(!(!pInfo->setting.visable && sync), "wtf");
    PropCall(prop, pInfo, sync);
    return true;
}

bool MMObject::SetPropInt32(const s32 prop, const s32 value, const bool sync) {
    const PropInfo * pInfo = _objectProps->SetValue(prop, DTYPE_INT32, &value, sizeof(value));
    if (!pInfo) {
        return false;
    }

    //OASSERT(!(!pInfo->setting.visable && sync), "wtf");
    PropCall(prop, pInfo, sync);
    return true;
}

bool MMObject::SetPropInt64(const s32 prop, const s64 value, const bool sync) {
    const PropInfo * pInfo = _objectProps->SetValue(prop, DTYPE_INT64, &value, sizeof(value));
    if (!pInfo) {
        return false;
    }

    PropCall(prop, pInfo, sync);
    return true;
}

bool MMObject::SetPropFloat(const s32 prop, const float value, const bool sync) {
    const PropInfo * pInfo = _objectProps->SetValue(prop, DTYPE_FLOAT, &value, sizeof(value));
    if (!pInfo) {
        return false;
    }

    PropCall(prop, pInfo, sync);
    return true;
}

bool MMObject::SetPropString(const s32 prop, const char * value, const bool sync) {
    const PropInfo * pInfo = _objectProps->SetValue(prop, DTYPE_STRING, value, strlen(value) + 1);
    if (!pInfo) {
        return false;
    }

    //OASSERT(!(!pInfo->setting.visable && sync), "wtf");
    PropCall(prop, pInfo, sync);
    return true;
}

bool MMObject::SetPropStruct(const s32 prop, const void * value, const s32 len, const bool sync) {
    const PropInfo * pInfo = _objectProps->SetValue(prop, DTYPE_STRUCT, value, len);
    if (!pInfo) {
        return false;
    }

    //OASSERT(!(!pInfo->setting.visable && sync), "wtf");
    PropCall(prop, pInfo, sync);
    return true;
}

s8 MMObject::GetPropInt8(const s32 prop) const {
    s32 size = 0;
    s8 value = *(s8 *)_objectProps->GetValue(prop, DTYPE_INT8, size);
    OASSERT(size == sizeof(value), "fuck");
    return value;
}

s16 MMObject::GetPropInt16(const s32 prop) const {
    s32 size = 0;
    s16 value = *(s16 *)_objectProps->GetValue(prop, DTYPE_INT16, size);
    OASSERT(size == sizeof(value), "fuck");
    return value;
}

s32 MMObject::GetPropInt32(const s32 prop) const {
    s32 size = 0;
    s32 value = *(s32 *)_objectProps->GetValue(prop, DTYPE_INT32, size);
    OASSERT(size == sizeof(value), "fuck");
    return value;
}

s64 MMObject::GetPropInt64(const s32 prop) const {
    s32 size = 0;
    s64 value = *(s64 *)_objectProps->GetValue(prop, DTYPE_INT64, size);
    OASSERT(size == sizeof(value), "fuck");
    return value;
}

float MMObject::GetPropFloat(const s32 prop) const {
    s32 size = 0;
    float value = *(float *)_objectProps->GetValue(prop, DTYPE_FLOAT, size);
    OASSERT(size == sizeof(value), "fuck");
    return value;
}

const char * MMObject::GetPropString(const s32 prop) const {
    s32 size = 0;
    return (const char *)_objectProps->GetValue(prop, DTYPE_STRING, size);
}

const void * MMObject::GetPropStruct(const s32 prop, s32& len) const {
	const void * pValue = _objectProps->GetValue(prop, DTYPE_STRUCT, len);
    return pValue;
}

void MMObject::RgsPropChangeCB(const s32 name, const PropCallback& cb, const char * debug_info) {
	_propCBPool.Register(name, cb, debug_info);
}

ITableControl * MMObject::CreateTable(const s32 name, const char * file, const s32 line) {
    TABLE_MAP::iterator itor = _tableMap.find(name);
    if (itor != _tableMap.end()) {
        OASSERT(false, "table already exists");
        return nullptr;
    }

    TableControl * table = TableControl::Create(name, this, file, line);
    _tableMap.insert(std::make_pair(name, table));
    return table;
}

ITableControl * MMObject::FindTable(const s32 name) const {
    TABLE_MAP::const_iterator itor = _tableMap.find(name);
    if (itor == _tableMap.end())
        return nullptr;
    return itor->second;
}

bool MMObject::RemoveTable(const s32 name) {
    TABLE_MAP::iterator itor = _tableMap.find(name);
    if (itor == _tableMap.end()) {
        OASSERT(false, "table is not exists");
        return false;
    }

    ((TableControl *)itor->second)->Release();
    _tableMap.erase(itor);
    return true;
}


void MMObject::RgsEntryCB(const s32 status, const StatusCallback& cb, const char * debuginfo) {
    _statusEntryCBPool.Register(status, cb, debuginfo);
}

void MMObject::RgsLeaveCB(const s32 status, const StatusCallback& cb, const char * debuginfo) {
	_statusLeaveCBPool.Register(status, cb, debuginfo);
}

void MMObject::RgsEntryJudegCB(const s32 status, const StatusJudgeCallback& cb, const char * debuginfo) {
	_statusEntryJudegCBPool.Register(status, cb, debuginfo);
}

void MMObject::RgsLeaveJudegCB(const s32 status, const StatusJudgeCallback& cb, const char * debuginfo) {
	_statusLeaveJudegCBPool.Register(status, cb, debuginfo);
}

bool MMObject::EntryStatus(const s32 status, const bool b) {
    if (_status != status) {
		if (!_statusLeaveJudegCBPool.Call(_status, b, ObjectMgr::Instance()->GetKernel(), this)
			|| !_statusEntryJudegCBPool.Call(status, b, ObjectMgr::Instance()->GetKernel(), this)) {
            return false;
        }
		_statusLeaveCBPool.Call(_status, ObjectMgr::Instance()->GetKernel(), this);
		_status = status;
		_statusEntryCBPool.Call(_status, ObjectMgr::Instance()->GetKernel(), this);
    } else {
        if (!_statusEntryJudegCBPool.Call(status, b, ObjectMgr::Instance()->GetKernel(), this)) {
            return false;
        }
		_statusEntryCBPool.Call(_status, ObjectMgr::Instance()->GetKernel(), this);
    }

    return true;
}
