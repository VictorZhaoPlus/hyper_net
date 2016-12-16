#include "TableControl.h"
#include "TableRow.h"
#include "ObjectMgr.h"

TableControl::TableControl(TableDescriptor * descriptor, IObject * host)
	: _descriptor(descriptor)
	, _host(host)
{
}

TableControl::~TableControl() {
	for (s32 i = 0; i < (s32)_rows.size(); ++i) {
		DEL _rows[i];
	}
	_rows.clear();

	_updateCallPool.clear();
	_addCallPool.clear();
	_delCallPool.clear();
	_swapCallPool.clear();
}

void TableControl::ClearRows() {
	for (s32 i = 0; i < (s32)_rows.size(); ++i) {
		DEL _rows[i];
	}
	_rows.clear();

	_intKeyMap.clear();
	_stringKeyMap.clear();
}

IRow * TableControl::FindRow(const s64 key) const {
	OASSERT(_descriptor->GetKeyType() == DTYPE_INT8
		|| _descriptor->GetKeyType() == DTYPE_INT16
		|| _descriptor->GetKeyType() == DTYPE_INT32
		|| _descriptor->GetKeyType() == DTYPE_INT64, "wtf");
	if (_descriptor->GetKeyType() != DTYPE_STRING && _descriptor->GetKeyType() < DTYPE_CANT_BE_KEY) {
		auto itr = _intKeyMap.find(key);
		if (itr != _intKeyMap.end())
			return GetRow(itr->second);
	}
	return nullptr;
}

IRow * TableControl::FindRow(const char * key) const {
	OASSERT(_descriptor->GetKeyType() == DTYPE_STRING, "wtf");
	if (_descriptor->GetKeyType() == DTYPE_STRING) {
		auto itr = _stringKeyMap.find(key);
		if (itr != _stringKeyMap.end())
			return GetRow(itr->second);
	}
	return nullptr;
}

IRow * TableControl::GetRow(const s32 index) const {
	OASSERT(index >= 0 && index < (s32)_rows.size(), "wtf");
	if (index >= 0 && index < (s32)_rows.size()) 
		return _rows[index];
	return nullptr;
}

IRow * TableControl::AddRow() {
	OASSERT(_descriptor->GetKeyType() == DTYPE_CANT_BE_KEY, "wtf");
	if (_descriptor->GetKeyType() == DTYPE_CANT_BE_KEY) {
		TableRow * row = NEW TableRow(this, _descriptor);
		_rows.push_back(row);
		AddCallBack(ObjectMgr::Instance()->GetKernel(), row, nullptr, 0, DTYPE_CANT_BE_KEY);
		return row;
	}
	return nullptr;
}

IRow * TableControl::AddRowKeyInt(s8 type, const void * data, const s32 size, s64 key) {
	OASSERT(_descriptor->GetKeyType() == type, "wtf");
	if (_descriptor->GetKeyType() == type) {
		TableRow * row = NEW TableRow(this, _descriptor);
		_intKeyMap.insert(std::make_pair(key, (s32)_rows.size()));
		row->Set(_descriptor->GetKeyCol(), type, data, size, false);
		_rows.push_back(row);
		AddCallBack(ObjectMgr::Instance()->GetKernel(), row, data, size, type);
		return row;
	}
	return nullptr;
}

IRow * TableControl::AddRowKeyString(const char * key) {
	OASSERT(_descriptor->GetKeyType() == DTYPE_STRING, "wtf");
	if (_descriptor->GetKeyType() == DTYPE_STRING) {
		TableRow * row = NEW TableRow(this, _descriptor);
		_stringKeyMap.insert(std::make_pair(key, (s32)_rows.size()));
		row->Set(_descriptor->GetKeyCol(), DTYPE_STRING, key, strlen(key), false);
		_rows.push_back(row);
		AddCallBack(ObjectMgr::Instance()->GetKernel(), row, key, strlen(key) + 1, DTYPE_STRING);
		return row;
	}

	return nullptr;
}

bool TableControl::DelRow(const s32 index) {
	TableRow * row = (TableRow *)GetRow(index);
	if (!row)
		return false;

	switch (_descriptor->GetKeyType()) {
	case DTYPE_INT8:{
		s8 key = row->GetDataInt8(_descriptor->GetKeyCol());
		_intKeyMap.erase(key);
		DeleteCallBack(ObjectMgr::Instance()->GetKernel(), row);
		break;
	}
	case DTYPE_INT16: {
		s16 key = row->GetDataInt16(_descriptor->GetKeyCol());
		_intKeyMap.erase(key);
		DeleteCallBack(ObjectMgr::Instance()->GetKernel(), row);
		break;
	}
	case DTYPE_INT32: {
		s32 key = row->GetDataInt32(_descriptor->GetKeyCol());
		_intKeyMap.erase(key);
		DeleteCallBack(ObjectMgr::Instance()->GetKernel(), row);
		break;
	}
	case DTYPE_INT64: {
		s64 key = row->GetDataInt64(_descriptor->GetKeyCol());
		_intKeyMap.erase(key);
		DeleteCallBack(ObjectMgr::Instance()->GetKernel(), row);
		break;
	}
	case DTYPE_STRING: {
		const char * key = row->GetDataString(_descriptor->GetKeyCol());
		_stringKeyMap.erase(key);
		DeleteCallBack(ObjectMgr::Instance()->GetKernel(), row);
		break;
	}
	case DTYPE_CANT_BE_KEY:
		break;
	default:
		OASSERT(false, "wtf");
		break;
	}

	auto itr = _rows.begin() + index;
	_rows.erase(itr);
	DEL row;

	OrderProcIndex(index);
}

bool TableControl::SwapRowIndex(const s32 src, const s32 dst) {
	if (src == dst)
		return true;

	TableRow * rowSrc = (TableRow *)GetRow(src);
	if (!rowSrc)
		return false;

	TableRow * rowDst = (TableRow *)GetRow(dst);
	if (!rowDst)
		return false;

	switch (_descriptor->GetKeyType()) {
	case DTYPE_INT8: {
		s8 keySrc = rowSrc->GetDataInt8(_descriptor->GetKeyCol());
		s8 keyDst = rowDst->GetDataInt8(_descriptor->GetKeyCol());
		_intKeyMap[keySrc] = src;
		_intKeyMap[keyDst] = dst;
		break;
	}
	case DTYPE_INT16: {
		s16 keySrc = rowSrc->GetDataInt16(_descriptor->GetKeyCol());
		s16 keyDst = rowDst->GetDataInt16(_descriptor->GetKeyCol());
		_intKeyMap[keySrc] = src;
		_intKeyMap[keyDst] = dst;
		break;
	}
	case DTYPE_INT32: {
		s32 keySrc = rowSrc->GetDataInt32(_descriptor->GetKeyCol());
		s32 keyDst = rowDst->GetDataInt32(_descriptor->GetKeyCol());
		_intKeyMap[keySrc] = src;
		_intKeyMap[keyDst] = dst;
		break;
	}
	case DTYPE_INT64: {
		s64 keySrc = rowSrc->GetDataInt64(_descriptor->GetKeyCol());
		s64 keyDst = rowDst->GetDataInt64(_descriptor->GetKeyCol());
		_intKeyMap[keySrc] = src;
		_intKeyMap[keyDst] = dst;
		break;
	}
	case DTYPE_STRING: {
		const char * keySrc = rowSrc->GetDataString(_descriptor->GetKeyCol());
		const char * keyDst = rowDst->GetDataString(_descriptor->GetKeyCol());
		_stringKeyMap[keySrc] = src;
		_stringKeyMap[keyDst] = dst;
		break;
	}
	case DTYPE_CANT_BE_KEY:
		break;
	default:
		OASSERT(false, "wtf");
		break;
	}

	_rows[src] = rowDst;
	_rows[dst] = rowSrc;
	rowSrc->SetRowIndex(dst);
	rowDst->SetRowIndex(src);
	SwapCallBack(ObjectMgr::Instance()->GetKernel(), rowSrc, rowDst);
}

void TableControl::OrderProcIndex(const s32 index) {
	for (s32 i = index; i < (s32)_rows.size(); i++) {
		switch (_descriptor->GetKeyType()) {
		case DTYPE_INT8: {
			s8 key = _rows[i]->GetDataInt8(_descriptor->GetKeyCol());
			_intKeyMap[key] = i;
		}
		break;
		case DTYPE_INT16: {
			s16 key = _rows[i]->GetDataInt16(_descriptor->GetKeyCol());
			_intKeyMap[key] = i;
		}
		break;
		case DTYPE_INT32:{
			s32 key = _rows[i]->GetDataInt32(_descriptor->GetKeyCol());
			_intKeyMap[key] = i;
		}
		break;
		case DTYPE_INT64:{
			s64 key = _rows[i]->GetDataInt64(_descriptor->GetKeyCol());
			_intKeyMap[key] = i;
		}
		break;
		case DTYPE_STRING:{
			const char * key = _rows[i]->GetDataString(_descriptor->GetKeyCol());
			_stringKeyMap[key] = i;
		}
		break;
		case DTYPE_CANT_BE_KEY:break;
		default:
			OASSERT(false, "wtf");
			break;
		}
		_rows[i]->SetRowIndex(i);
	}
}
