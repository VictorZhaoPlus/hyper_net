#ifndef __SCENEMGR_H__
#define __SCENEMGR_H__
#include "util.h"
#include "IScene.h"
#include "singleton.h"
#include "IHarbor.h"
#include <map>
#include <vector>
#include <unordered_set>

class OBuffer;
class SceneMgr : public ISceneMgr, public INodeListener, public OHolder<SceneMgr> {
	struct SceneInfo {
		s32 distribute;
		s32 real;
		s64 id;

		SceneInfo() : distribute(0), real(0), id(0) {}
	};

public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port);
	virtual void OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) {}

	virtual void SetDistributor(ISceneDistributor * distributor) { _distributor = distributor; }

	void OnRecvAppear(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer & args);
	void OnRecvDisappear(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer & args);
	void OnRecvUpdate(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer & args);
	void OnRecvConfirm(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);
	void OnRecvRecover(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);

	std::pair<s32, s64> FindOrCreate(IKernel * kernel, const char * scene, s64 copyId);
	std::pair<s32, s64> Find(IKernel * kernel, const char * scene, s64 copyId);

private:
    IKernel * _kernel;
	ISceneDistributor * _distributor;

	std::map<std::string, std::map<s64, SceneInfo>> _scenes;
	std::vector<s32> _nodes;
};

#endif //__SCENEMGR_H__

