#ifndef __SCENE_H__
#define __SCENE_H__
#include "util.h"
#include "IScene.h"
#include "singleton.h"
#include <unordered_map>

class OArgs;
class IHarbor;
class IObjectMgr;
class IObject;
class ICapacityPublisher;
class IProp;
class CellInterface;
class Cell : public ICell, public ICellVisibleChecker, public OHolder<Cell> {
public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual bool IsVisiable(IObject * object, IObject * test) { return true; }

	void CreateCell(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args);
	void RecoverCell(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args);

	void EnterCell(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args);
	void LeaveCell(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args);

	void UpdateCell(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args);
	
	void ReadProps(IKernel * kernel, IObject * object, const OBuffer& buf);

private:
    IKernel * _kernel;
	IHarbor * _harbor;
	IObjectMgr * _objectMgr;
	ICapacityPublisher * _capacityPublisher;
	ICellVisibleChecker * _cellVisibleChecker;

	s32 _updateInterval;

	const IProp * _propId;

	std::unordered_map<s32, CellInterface*> _cells;
};

#endif //__SCENE_H__

