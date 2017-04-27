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

    virtual IObject * Create(const char * file, const s32 line, const char * name);
    virtual IObject * CreateObjectByID(const char * file, const s32 line, const char * name, const s64 id);
    virtual IObject * FindObject(const s64 id);
    virtual void Recove(IObject * object);

	virtual const IProp * CalcProp(const char * name);
	virtual const IProp * CalcProp(const s32 name);
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
	std::unordered_map<std::string, std::string> _namePathMap;
	std::unordered_map<std::string, ObjectDescriptor *> _models;
	std::unordered_map<s32, TableDescriptor *> _tableModels;
	std::unordered_map<std::string, s32> _defines;
	s32 _nextTypeId;

	std::unordered_map<std::string, ObjectProp *> _props;
	std::unordered_map<s32, ObjectProp *> _propIds;
	std::unordered_map<s64, ObjectCreateInfo> _objects;
	std::unordered_map<s32, TableCreateInfo> _tableMap;

	IKernel * _kernel;
	IIdMgr * _idMgr;
};

#endif //define __ObjectMgr_h__
