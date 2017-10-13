#ifndef __WATCHER_H__
#define __WATCHER_H__
#include "util.h"
#include "IScene.h"
#include "singleton.h"
#include "OCallback.h"

class OBuffer;
class Watcher : public IWatcher, public OHolder<Watcher> {
	class RuningQuery : public ITimer {
	public:
		RuningQuery(s64 queryId, s64 objectId, const QueryCallback& cb) : _queryId(queryId), _objectId(objectId), _cb(cb) {}
		virtual ~RuningQuery() {}

		s64 GetQueryId() const { return _queryId; }

		void Wait(s64 target) { _targets.insert(target); }
		void Awake(IKernel * kernel, s64 target, s8 batch);

		virtual void OnStart(IKernel * kernel, s64 tick) {}
		virtual void OnTimer(IKernel * kernel, s32 beatCount, s64 tick) {}
		virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick);

	private:
		s64 _queryId;
		s64 _objectId;
		QueryCallback _cb;

		std::set<s64> _targets;
		std::multiset<std::tuple<s8, s64>> _ret;
	};

public:
    virtual bool Initialize(IKernel * kernel);
    virtual bool Launched(IKernel * kernel);
    virtual bool Destroy(IKernel * kernel);

	virtual void Brocast(IObject * object, const s32 msgId, const OBuffer& buf, bool self = false);

	virtual s64 QueryInVision(IObject * object, const s32 type, const void * context, const s32 size, s32 wait, const QueryCallback& cb);
	virtual void StopQuery(const s64 queryId);
	void FinishQuery(const s64 queryId) { _queries.erase(queryId); }
	void Query(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args);
	void QueryAck(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args);

	virtual void RegQuerior(const s32 type, const QueryFunc& f, const char * info) { _queriors[type] = f; }

	void DealInterest(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args);
	void DealWatcher(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args);

	void DisapperWhenDestroy(IKernel * kernel, const void * context, const s32 size);

private:
    IKernel * _kernel;

	std::unordered_map<s32, QueryFunc> _queriors;
	std::unordered_map<s64, RuningQuery *> _queries;
	s32 _nextQueryId;
};

#endif //__SCENE_H__

