#include "TableProp.h"
#include "ObjectMgr.h"

const IColumn * TableProp::GetCol(const char * col) const {
	auto itr = _columns.find(col);
	if (itr != _columns.end())
		return itr->second;
	return nullptr;
}

IColumn * TableProp::AddColumn(const char * name, s32 typeId, s32 idx) {
	ColumnProp * col = nullptr;
	auto itr = _columns.find(name);
	if (itr != _columns.end())
		col = itr->second;
	else {
		col = NEW ColumnProp(name, ObjectMgr::Instance()->CountType());
		_columns[name] = col;
	}

	col->SetLayout(typeId, idx);

	return col;
}