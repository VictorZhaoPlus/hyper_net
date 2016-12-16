/* 
 * File:   ObjectMgr.h
 * Author: alax
 *
 * Created on March 3, 2015, 10:44 AM
 */
#ifndef __ObjectMgr_h__
#define __ObjectMgr_h__
#include <unordered_map>
#include "OString.h"
#include "singleton.h"
#include "IObjectMgr.h"

class MMObject;
class IIdMgr;
class ObjectFactory;
class ObjectMgr : public IObjectMgr, public OHolder<ObjectMgr> {
    struct TableInfo {
        const s32 name;
        const TableColumnInfo * columnInfo;
    };
    typedef std::vector<const TableInfo *> TABLE_MODEL_LIST;

    typedef std::unordered_map<olib::OString<MAX_MODEL_NAME_LEN>, ObjectFactory *,  olib::OStringHash<MAX_MODEL_NAME_LEN>> OBJECT_MODEL_MAP;
    typedef std::unordered_map<olib::OString<MAX_MODEL_NAME_LEN>, olib::OString<MAX_PATH>, olib::OStringHash<MAX_MODEL_NAME_LEN>> NAME_PATH_MAP;
	typedef std::unordered_map<olib::OString<MAX_MODEL_NAME_LEN>, IProp *, olib::OStringHash<MAX_MODEL_NAME_LEN>> PROP_MAP;

    struct ObjectCreateInfo {
        MMObject * object;
        const olib::OString<128> file;
        const s32 line;
    };

    typedef std::unordered_map<s64, ObjectCreateInfo> OBJCET_MAP;
	typedef std::unordered_map<s32, ITableControl *> TABLE_MAP;
	typedef std::unordered_map<olib::OString<MAX_MODEL_NAME_LEN>, s32, olib::OStringHash<MAX_MODEL_NAME_LEN>> PROP_DEFINES;

public:
    ObjectMgr() {}
	virtual ~ObjectMgr() {}

    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

    virtual IObject * Create(const char * file, const s32 line, const char * name, bool shadow);
    virtual IObject * CreateObjectByID(const char * file, const s32 line, const char * name, const s64 id, bool shadow);
    virtual IObject * FindObject(const s64 id);
    virtual void Recove(IObject * object);

	virtual const IProp * CalcProp(const char * name);
	virtual s32 CalcTableName(const char * name);

	virtual const std::vector<const IProp*>* GetPropsInfo(const char * type, bool noFather = false) const;

    //创建对象类型静态表(同类型对象共有)
    virtual ITableControl * CreateStaticTable(const char* name, const char * model, const char * file, const s32 line);
	virtual void RecoverStaticTable(ITableControl * table);

	IKernel * GetKernel() const { return _kernel; }

private:
	ObjectFactory * QueryTemplate(IKernel * pKernel, const char * name);
	ObjectFactory * CreateTemplate(IKernel * pKernel, const char * name);

private:
    NAME_PATH_MAP _namePathMap;
    OBJECT_MODEL_MAP _propMap;
    OBJCET_MAP _objects;
    TABLE_MAP _tableMap;
	PROP_DEFINES _defines;
	PROP_MAP _props;

	IKernel * _kernel;
	IIdMgr * _idMgr;
};

#endif //define __ObjectMgr_h__
