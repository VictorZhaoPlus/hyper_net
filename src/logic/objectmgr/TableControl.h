/* 
 * File:   TableControl.h
 * Author: Alax
 *
 * Created on March 3, 2015, 10:46 AM
 */
#ifndef __TableControl_h__
#define __TableControl_h__
#include "IObjectMgr.h"
#include <vector>
#include <unordered_map>
#include <list>

#define MAX_STRING_KEY_LEN 64
class TableRow;
class TableDescriptor;
class TableControl : public ITableControl{
    typedef std::vector<TableRow *> TABLE_ROWS;
    typedef std::unordered_map<s64, s32> KEY_INT_MAP;
    typedef std::unordered_map<std::string, s32> KEY_STRING_MAP;

    typedef std::list<TableUpdateCallback> UPDATE_CB_POOL;
    typedef std::list<TableAddCallback> ADD_CB_POOL;
    typedef std::list<TableDeleteCallback> DELETE_CB_POOL;
    typedef std::list<TableSwapCallback> SWP_CB_POOL;

public:
	TableControl(s32 name, const TableDescriptor * descriptor, IObject * host = nullptr);
	~TableControl();

	virtual void RgsUpdate(TableUpdateCallback cb, const char * debug) { _updateCallPool.push_back(cb); }
	virtual void RgsAdd(TableAddCallback cb, const char * debug) { _addCallPool.push_back(cb); }
	virtual void RgsDelete(TableDeleteCallback cb, const char * debug) { _delCallPool.push_back(cb); }
	virtual void RgsChange(TableSwapCallback cb, const char * debug) { _swapCallPool.push_back(cb); }

	virtual IObject * GetHost() { return _host; }
	s32 GetName() const { return _name; }

	virtual s32 RowCount() const { return (s32)_rows.size(); }
	virtual void ClearRows();

    virtual IRow * FindRow(const s64 key) const;
    virtual IRow * FindRow(const char * key) const;
	virtual IRow * GetRow(const s32 index) const;

    virtual IRow * AddRow();
	IRow * AddRowKeyInt(s8 type, const void * data, const s32 size, s64 key);
    virtual IRow * AddRowKeyInt8(const s8 key) { return AddRowKeyInt(DTYPE_INT8, &key, sizeof(key), key); }
    virtual IRow * AddRowKeyInt16(const s16 key) { return AddRowKeyInt(DTYPE_INT16, &key, sizeof(key), key); }
    virtual IRow * AddRowKeyInt32(const s32 key) { return AddRowKeyInt(DTYPE_INT32, &key, sizeof(key), key); }
    virtual IRow * AddRowKeyInt64(const s64 key) { return AddRowKeyInt(DTYPE_INT64, &key, sizeof(key), key); }
    virtual IRow * AddRowKeyString(const char * key);

    virtual bool DelRow(const s32 index);
    virtual bool SwapRowIndex(const s32 src, const s32 dst);

	void UpdateCallBack(IKernel * pKernel, IRow *  row, const s32 column, const s8 type) {
		for (auto cb : _updateCallPool)
			cb(pKernel, this, row, column, type);
	}

	void AddCallBack(IKernel * pKernel, IRow * row, const void * key, const s32 size, const s8 type) {
		for (auto cb : _addCallPool)
			cb(pKernel, this, row, key, size, type);
	}

	void DeleteCallBack(IKernel * pKernel, IRow * row) {
		for (auto cb : _delCallPool)
			cb(pKernel, this, row);
	}

	void SwapCallBack(IKernel * pKernel, IRow * src, IRow * dst) {
		for (auto cb : _swapCallPool)
			cb(pKernel, this, src, dst);
	}

	void ChangeKey(const s64 oldKey, const s64 newKey, const s8 type);
	void ChangeKey(const char * oldKey, const char * newKey, const s8 type);
    void OrderProcIndex(const s32 index);

private:
    IObject * _host;
	s32 _name;

    UPDATE_CB_POOL _updateCallPool;
    ADD_CB_POOL _addCallPool;
    DELETE_CB_POOL _delCallPool;
    SWP_CB_POOL _swapCallPool;

	const TableDescriptor * _descriptor;

    KEY_INT_MAP _intKeyMap;
    KEY_STRING_MAP _stringKeyMap;
    TABLE_ROWS _rows;
};

#endif //defined __TableControl_h__
