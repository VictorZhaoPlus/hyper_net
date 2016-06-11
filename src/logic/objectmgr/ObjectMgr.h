/* 
 * File:   ObjectMgr.h
 * Author: alax
 *
 * Created on March 3, 2015, 10:44 AM
 */
#ifndef __ObjectMgr_h__
#define __ObjectMgr_h__
#include "ObjectStruct.h"
#include "TableStruct.h"
#include <unordered_map>
#include "OString.h"

class MMObject;
class IHarbor;
class ObjectMgr : public IObjectMgr {
    struct TableInfo {
        TableInfo(const char * name_, const TableColumnInfo * columnInfo_) : name(name_), columnInfo(columnInfo_) {
            OASSERT(pName && _pColumnInfo, "wtf");
        }
        const olib::OString<MAX_TABLE_NAME_LEN> name;
        const TableColumnInfo * columnInfo;
    };
    typedef std::vector<const TableInfo *> TABLE_MODEL_LIST;
    struct ObjectModel {
        ObjectModel() : pTableModels(NULL), pObjectPropInfo(NULL) {}
        ObjectModel(const ObjectPropInfo * _pObjectPropInfo, const TABLE_MODEL_LIST * _oTableModels) : pObjectPropInfo(_pObjectPropInfo), pTableModels(_oTableModels) { }
        const TABLE_MODEL_LIST * pTableModels;
        const ObjectPropInfo * pObjectPropInfo;
    };

    typedef std::unordered_map<
		olib::OString<MAX_MODEL_NAME_LEN>,
        const ObjectModel, 
		olib::OStringHash<MAX_MODEL_NAME_LEN>
    > OBJECT_MODEL_MAP;

    typedef std::unordered_map<
		olib::OString<MAX_MODEL_NAME_LEN>,
		olib::OString<MAX_MODEL_PATH_LEN>,
		olib::OStringHash<MAX_MODEL_NAME_LEN>
    > NAME_PATH_MAP;

    struct ObjectCreateInfo {
        ObjectCreateInfo(MMObject * _obj, const char * _file, const s32 _line) : pObject(_obj), file(_file), line(_line) {}
        MMObject * pObject;
        const olib::OString<128> file;
        const s32 line;
    };
    typedef std::unordered_map<s64, ObjectCreateInfo> OBJCET_MAP;

	typedef std::unordered_map<
		olib::OString<MAX_TABLE_NAME_LEN>,
		ITableControl *,
		olib::OStringHash<MAX_MODEL_NAME_LEN>
	> TABLE_MAP;
public:
    ObjectMgr() {}

    virtual bool Initialize(IKernel * pKernel);
    virtual bool Launched(IKernel * pKernel);
    virtual bool Destroy(IKernel * pKernel);

    virtual s64 AllotID();

    virtual IObject * Create(const char * file, const s32 line, const char * name, bool shadow);
    virtual IObject * CreateObjectByID(const char * file, const s32 line, const char * name, const s64 id, bool shadow);
    virtual IObject * FindObject(const s64 id);
    virtual void Recove(IObject * pObject);

    virtual const PROP_INDEX * GetPropsInfo(const char * type, bool noFather = false) const;

    //通过名称获取静态表
    virtual ITableControl * FindStaticTable(const char * name);
    virtual void RecoverStaticTable(ITableControl * pTable);
    //创建对象类型静态表(同类型对象共有)
    virtual ITableControl * CreateStaticTable(const char * name, const char * file, const s32 line);

private:
    const ObjectModel * QueryTemplate(IKernel * pKernel, const char * name);
    const ObjectModel * CreateTemplate(IKernel * pKernel, const char * name);

private:
    NAME_PATH_MAP _namePathMap;
    OBJECT_MODEL_MAP _propMap;
    OBJCET_MAP _objects;
    TABLE_MAP _tableMap;

private:
	static ObjectMgr * s_self;
	static IKernel * s_kernel;
    static IHarbor * s_harbor;
};

#endif //define __ObjectMgr_h__
