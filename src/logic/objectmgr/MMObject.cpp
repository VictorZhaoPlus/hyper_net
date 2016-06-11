#include "MMObject.h"

tlib::TPool<MMObject> MMObject::s_oObjectPool;

const s64 MMObject::GetID() const {
    return m_oObjectID;
}

const char * MMObject::GetTypeName() const {
    return m_oName.GetString();
}

void MMObject::PropAnyCBCall(const char * prop, const PropInfo * pInfo, const bool sync) {
    ANY_CALL_LIST::const_iterator itor = m_oAnyCallList.begin();
    ANY_CALL_LIST::const_iterator iend = m_oAnyCallList.end();
    while (itor != iend) {
        s64 lTick = tools::GetTimeMillisecond();
        (itor->cb)(g_pKernel, this, m_oName.GetString(), prop, pInfo, sync);
        lTick = tools::GetTimeMillisecond() - lTick;
        if (lTick >= 1) {
//            ECHO_TRACE("call call call... long time call %s %lld", itor->name, lTick);
        }
        itor++;
    }
}

const PROP_INDEX & MMObject::GetPropsInfo(bool noFather) const {
    return m_pObjectPropInfo->GetPropsInfo(noFather);
}

void MMObject::PropCall(const char * prop, const PropInfo * pInfo, const bool sync) {
    m_oPropCBPool.Call(prop, g_pKernel, this, m_oName.GetString(), prop, pInfo, sync);
    PropAnyCBCall(prop, pInfo, sync);
}

bool MMObject::SetPropInt8(const char * prop, const s8 value, const bool sync) {
    const PropInfo * pInfo = m_pObjectProps->SetValue(prop, DTYPE_INT8, &value, sizeof(value));
    if (!pInfo) {
        return false;
    }

    //OASSERT(!(!pInfo->setting.visable && sync), "wtf");
    PropCall(prop, pInfo, sync);
    return true;
}

bool MMObject::SetPropInt16(const char * prop, const s16 value, const bool sync) {
    const PropInfo * pInfo = m_pObjectProps->SetValue(prop, DTYPE_INT16, &value, sizeof(value));
    if (!pInfo) {
        return false;
    }

    //OASSERT(!(!pInfo->setting.visable && sync), "wtf");
    PropCall(prop, pInfo, sync);
    return true;
}

bool MMObject::SetPropInt32(const char * prop, const s32 value, const bool sync) {
    const PropInfo * pInfo = m_pObjectProps->SetValue(prop, DTYPE_INT32, &value, sizeof(value));
    if (!pInfo) {
        return false;
    }

    //OASSERT(!(!pInfo->setting.visable && sync), "wtf");
    PropCall(prop, pInfo, sync);
    return true;
}

bool MMObject::SetPropInt64(const char * prop, const s64 value, const bool sync) {
    const PropInfo * pInfo = m_pObjectProps->SetValue(prop, DTYPE_INT64, &value, sizeof(value));
    if (!pInfo) {
        return false;
    }

    PropCall(prop, pInfo, sync);
    return true;
}

bool MMObject::SetPropFloat(const char * prop, const float value, const bool sync) {
    const PropInfo * pInfo = m_pObjectProps->SetValue(prop, DTYPE_FLOAT, &value, sizeof(value));
    if (!pInfo) {
        return false;
    }

    PropCall(prop, pInfo, sync);
    return true;
}

bool MMObject::SetPropString(const char * prop, const char * value, const bool sync) {
    const PropInfo * pInfo = m_pObjectProps->SetValue(prop, DTYPE_STRING, value, strlen(value) + 1);
    if (!pInfo) {
        return false;
    }

    //OASSERT(!(!pInfo->setting.visable && sync), "wtf");
    PropCall(prop, pInfo, sync);
    return true;
}

bool MMObject::SetPropStruct(const char * prop, const void * value, const s32 len, const bool sync) {
    const PropInfo * pInfo = m_pObjectProps->SetValue(prop, DTYPE_STRUCT, value, len);
    if (!pInfo) {
        return false;
    }

    //OASSERT(!(!pInfo->setting.visable && sync), "wtf");
    PropCall(prop, pInfo, sync);
    return true;
}

s8 MMObject::GetPropInt8(const char * prop) const {
    s32 nSize = 0;
    s8 value = *(s8 *)m_pObjectProps->GetValue(prop, DTYPE_INT8, nSize);
    OASSERT(nSize == sizeof(value), "fuck");
    return value;
}

s16 MMObject::GetPropInt16(const char * prop) const {
    s32 nSize = 0;
    s16 value = *(s16 *)m_pObjectProps->GetValue(prop, DTYPE_INT16, nSize);
    OASSERT(nSize == sizeof(value), "fuck");
    return value;
}

s32 MMObject::GetPropInt32(const char * prop) const {
    s32 nSize = 0;
    s32 value = *(s32 *)m_pObjectProps->GetValue(prop, DTYPE_INT32, nSize);
    OASSERT(nSize == sizeof(value), "fuck");
    return value;
}

s64 MMObject::GetPropInt64(const char * prop) const {
    s32 nSize = 0;
    s64 value = *(s64 *)m_pObjectProps->GetValue(prop, DTYPE_INT64, nSize);
    OASSERT(nSize == sizeof(value), "fuck");
    return value;
}

float MMObject::GetPropFloat(const char * prop) const {
    s32 nSize = 0;
    float value = *(float *)m_pObjectProps->GetValue(prop, DTYPE_FLOAT, nSize);
    OASSERT(nSize == sizeof(value), "fuck");
    return value;
}

const char * MMObject::GetPropString(const char * prop) const {
    s32 nSize = 0;
    return (const char *)m_pObjectProps->GetValue(prop, DTYPE_STRING, nSize);
}

const void * MMObject::GetPropStruct(const char * prop, s32& len) const {
	const void * pValue = m_pObjectProps->GetValue(prop, DTYPE_STRUCT, len);
    return pValue;
}

bool MMObject::AddTempInt8(const char * temp) {
    OASSERT(m_pObjectTemps == NULL, "wtf");
    if (m_pObjectTemps) {
        return false;
    }

    return NULL != m_oTempInfo.AddProp(temp, sizeof(s8), DTYPE_INT8, false, false, false, false, false);
}

bool MMObject::AddTempInt16(const char * temp) {
    OASSERT(m_pObjectTemps == NULL, "wtf");
    if (m_pObjectTemps) {
        return false;
    }

    return NULL != m_oTempInfo.AddProp(temp, sizeof(s16), DTYPE_INT16, false, false, false, false, false);
}

bool MMObject::AddTempInt32(const char * temp) {
    OASSERT(m_pObjectTemps == NULL, "wtf");
    if (m_pObjectTemps) {
        return false;
    }

    return NULL != m_oTempInfo.AddProp(temp, sizeof(s32), DTYPE_INT32, false, false, false, false, false);
}

bool MMObject::AddTempInt64(const char * temp) {
    OASSERT(m_pObjectTemps == NULL, "wtf");
    if (m_pObjectTemps) {
        return false;
    }

    return NULL != m_oTempInfo.AddProp(temp, sizeof(s64), DTYPE_INT64, false, false, false, false, false);
}

bool MMObject::AddTempFloat(const char * temp) {
    OASSERT(m_pObjectTemps == NULL, "wtf");
    if (m_pObjectTemps) {
        return false;
    }

    return NULL != m_oTempInfo.AddProp(temp, sizeof(float), DTYPE_FLOAT, false, false, false, false, false);
}

bool MMObject::AddTempString(const char * temp, const s32 maxlen) {
    OASSERT(m_pObjectTemps == NULL, "wtf");
    if (m_pObjectTemps) {
        return false;
    }

    return NULL != m_oTempInfo.AddProp(temp, maxlen, DTYPE_STRING, false, false, false, false, false);
}

bool MMObject::AddTempStruct(const char * temp, const s32 size) {
    OASSERT(m_pObjectTemps == NULL, "wtf");
    if (m_pObjectTemps) {
        return false;
    }

    return NULL != m_oTempInfo.AddProp(temp, size, DTYPE_STRUCT, false, false, false, false, false);
}

bool MMObject::AddTempBlob(const char * temp, const s32 size) {
    OASSERT(m_pObjectTemps == NULL, "wtf");
    if (m_pObjectTemps) {
        return false;
    }

    return NULL != m_oTempInfo.AddProp(temp, size + sizeof(s32), DTYPE_BLOB, false, false, false, false, false);
}

bool MMObject::FormingTemp() {
    OASSERT(m_pObjectTemps == NULL, "wtf");
    if (m_pObjectTemps) {
        return false;
    }

    m_pObjectTemps = ObjectPropsPool::GetInterface()->Create(&m_oTempInfo);
    OASSERT(m_pObjectTemps, "wtf");
    m_pObjectTemps->Clear();
    return true;
}

bool MMObject::SetTempInt8(const char * temp, const s8 value) {
    OASSERT(m_pObjectTemps, "wtf");
    return NULL != m_pObjectTemps->SetValue(temp, DTYPE_INT8, &value, sizeof(value));
}

bool MMObject::SetTempInt16(const char * temp, const s16 value) {
    OASSERT(m_pObjectTemps, "wtf");
    return NULL != m_pObjectTemps->SetValue(temp, DTYPE_INT16, &value, sizeof(value));
}

bool MMObject::SetTempInt32(const char * temp, const s32 value) {
    OASSERT(m_pObjectTemps, "wtf");
    return NULL != m_pObjectTemps->SetValue(temp, DTYPE_INT32, &value, sizeof(value));
}

bool MMObject::SetTempInt64(const char * temp, const s64 value) {
    OASSERT(m_pObjectTemps, "wtf");
    return NULL != m_pObjectTemps->SetValue(temp, DTYPE_INT64, &value, sizeof(value));
}

bool MMObject::SetTempFloat(const char * temp, const float value) {
    OASSERT(m_pObjectTemps, "wtf");
    return NULL != m_pObjectTemps->SetValue(temp, DTYPE_FLOAT, &value, sizeof(value));
}

bool MMObject::SetTempString(const char * temp, const char * value) {
    OASSERT(m_pObjectTemps, "wtf");
    return NULL != m_pObjectTemps->SetValue(temp, DTYPE_STRING, value, strlen(value) + 1);
}

bool MMObject::SetTempStruct(const char * temp, void * pValue, const s32 size) {
    OASSERT(m_pObjectTemps, "wtf");
    return NULL != m_pObjectTemps->SetValue(temp, DTYPE_STRUCT, pValue, size);
}

bool MMObject::SetTempBlob(const char * temp, const void * pValue, const s32 size) {
    OASSERT(m_pObjectTemps, "wtf");
    OASSERT((0 >= size && nullptr == pValue) || (pValue && size > 0), "wtf");
    s32 len = size + sizeof(s32);
    char * data = (char *)alloca(len);
    tools::SafeMemcpy(data, len, &size, sizeof(size));
    if (pValue) {
        tools::SafeMemcpy(data + sizeof(size), len - sizeof(size), pValue, size);
    }
    return NULL != m_pObjectTemps->SetValue(temp, DTYPE_BLOB, data, len);
}

s8 MMObject::GetTempInt8(const char * temp) const {
    s32 nSize = 0;
    s8 value = *(s8 *)m_pObjectTemps->GetValue(temp, DTYPE_INT8, nSize);
    OASSERT(nSize == sizeof(value), "fuck");
    return value;
}

s16 MMObject::GetTempInt16(const char * temp) const {
    s32 nSize = 0;
    s16 value = *(s16 *)m_pObjectTemps->GetValue(temp, DTYPE_INT16, nSize);
    OASSERT(nSize == sizeof(value), "fuck");
    return value;
}

s32 MMObject::GetTempInt32(const char * temp) const {
    s32 nSize = 0;
    s32 value = *(s32 *)m_pObjectTemps->GetValue(temp, DTYPE_INT32, nSize);
    OASSERT(nSize == sizeof(value), "fuck");
    return value;
}

s64 MMObject::GetTempInt64(const char * temp) const {
    s32 nSize = 0;
    s64 value = *(s64 *)m_pObjectTemps->GetValue(temp, DTYPE_INT64, nSize);
    OASSERT(nSize == sizeof(value), "fuck");
    return value;
}

float MMObject::GetTempFloat(const char * temp) const {
    s32 nSize = 0;
    float value = *(float *)m_pObjectTemps->GetValue(temp, DTYPE_FLOAT, nSize);
    OASSERT(nSize == sizeof(value), "fuck");
    return value;
}

const char * MMObject::GetTempString(const char * temp) const {
    s32 nSize = 0;
    return (const char *)m_pObjectTemps->GetValue(temp, DTYPE_STRING, nSize);
}

void * MMObject::GetTempStruct(const char * temp, const s32 size) const {
    s32 nSize = 0;
    void * pValue = (void *)m_pObjectTemps->GetValue(temp, DTYPE_STRUCT, nSize);
    OASSERT(nSize == nSize, "fuck");
    return pValue;
}

const void * MMObject::GetTempBlob(const char * temp, OUT s32 & size) const {
    s32 nSize = 0;
    void * pValue = (void *)m_pObjectTemps->GetValue(temp, DTYPE_BLOB, nSize);
    size = *(s32 *)pValue;
    return (char *)pValue + sizeof(s32);
}


bool MMObject::RgsPropChangeCB(const char * name, const prop_cb cb, const char * debug_info) {
    if (ANY_CALL == name) {
        m_oAnyCallList.push_back(AnyCallInfo(cb, debug_info));
        return true;
    }

    return m_oPropCBPool.RegisterCall(name, cb, debug_info);
}

ITableControl * MMObject::CreateTable(const char * name, const char * file, const s32 line) {
    TABLE_MAP::iterator itor = _tableMap.find(name);
    if (itor != _tableMap.end()) {
        OASSERT(false, "table already exists");
        return NULL;
    }

    TableControl * pTable = TableControl::Create(name, this, file, line);
    _tableMap.insert(make_pair(name, pTable));
    return pTable;
}

ITableControl * MMObject::FindTable(const char * name) const {
    TABLE_MAP::const_iterator itor = _tableMap.find(name);
    if (itor == _tableMap.end()) {
        return NULL;
    }

    return itor->second;
}

bool MMObject::RemoveTable(const char * name) {
    TABLE_MAP::iterator itor = _tableMap.find(name);
    if (itor == _tableMap.end()) {
        OASSERT(false, "table is not exists");
        return false;
    }

    ((TableControl *)itor->second)->Release();
    _tableMap.erase(itor);
    return true;
}


bool MMObject::RgsEntryCB(const s32 status, const status_cb cb, const char * debuginfo) {
    return m_oStatusEntryCBPool.RegisterCall(status, cb, debuginfo);
}

bool MMObject::RgsLeaveCB(const s32 status, const status_cb cb, const char * debuginfo) {
    return m_oStatusLeaveCBPool.RegisterCall(status, cb, debuginfo);
}

bool MMObject::RgsEntryJudegCB(const s32 status, const status_judeg_cb cb, const char * debuginfo) {
    return m_oStatusEntryJudegCBPool.RegisterCall(status, cb, debuginfo);
}

bool MMObject::RgsLeaveJudegCB(const s32 status, const status_judeg_cb cb, const char * debuginfo) {
    return m_oStatusLeaveJudegCBPool.RegisterCall(status, cb, debuginfo);
}

bool MMObject::EntryStatus(const s32 status, const bool b) {
    if (m_nStatus != status) {
		if (!m_oStatusLeaveJudegCBPool.CallWithCondition(m_nStatus, b, g_pKernel, this)
			|| !m_oStatusEntryJudegCBPool.CallWithCondition(status, b, g_pKernel, this)) {
            return false;
        }
        m_oStatusLeaveCBPool.Call(m_nStatus, g_pKernel, this);
        m_nStatus = status;
		SetPropInt32("state", m_nStatus, true);
        m_oStatusEntryCBPool.Call(m_nStatus, g_pKernel, this);
    } else {
        if (!m_oStatusEntryJudegCBPool.CallWithCondition(status, b, g_pKernel, this)) {
            return false;
        }
        m_oStatusEntryCBPool.Call(m_nStatus, g_pKernel, this);
    }

    return true;
}
