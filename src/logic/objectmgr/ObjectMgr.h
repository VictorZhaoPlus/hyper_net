/* 
 * File:   ObjectMgr.h
 * Author: alax
 *
 * Created on March 3, 2015, 10:44 AM
 */
#ifndef __ObjectMgr_h__
#define __ObjectMgr_h__
#include <unordered_map>
#include "singleton.h"
#include "IObjectMgr.h"

class MMObject;
class IIdMgr;
struct ObjectLayout;
class ObjectProp;
class ObjectDescriptor;
class TableDescriptor;
class ObjectMgr : public IObjectMgr, public OHolder<ObjectMgr> {
    typedef std::unordered_map<std::string, ObjectDescriptor *> OBJECT_MODEL_MAP;
    typedef std::unordered_map<std::string, std::string> NAME_PATH_MAP;
	typedef std::unordered_map<std::string, ObjectProp *> PROP_MAP;

    struct ObjectCreateInfo {
        MMObject * object;
        const std::string file;
        const s32 line;
    };

	struct TableCreateInfo {
		ITableControl * object;
		const std::string file;
		const s32 line;
	};

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
	virtual s32 CalcPropSetting(const char * setting);

	virtual const std::vector<const IProp*>* GetPropsInfo(const char * type, bool noFather = false) const;

    //创建对象类型静态表(同类型对象共有)
    virtual ITableControl * CreateStaticTable(const char * name, const char * model, const char * file, const s32 line);
	virtual void RecoverStaticTable(ITableControl * table);
	virtual s32 CalcTableName(const char * table);

	const IProp* SetObjectProp(const char* name, const s32 typeId, ObjectLayout * layout);

	IKernel * GetKernel() const { return _kernel; }

private:
	ObjectDescriptor * QueryTemplate(IKernel * pKernel, const char * name);
	ObjectDescriptor * CreateTemplate(IKernel * pKernel, const char * name);

private:
    NAME_PATH_MAP _namePathMap;
    OBJECT_MODEL_MAP _models;
	std::unordered_map<s32, TableDescriptor *> _tableModels;
	std::unordered_map<std::string, s32> _defines;
	s32 _nextTypeId;

	PROP_MAP _props;
	std::unordered_map<s64, ObjectCreateInfo> _objects;
	std::unordered_map<s32, TableCreateInfo> _tableMap;

	IKernel * _kernel;
	IIdMgr * _idMgr;
};

#endif //define __ObjectMgr_h__
