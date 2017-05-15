#include "ObjectTimer.h"
#include "IObjectMgr.h"

class OCTimer : public ITimer {
public:
	OCTimer(IObject * object, const IProp * prop, const char * file, const s32 line) 
		: _id(object->GetID())
		, _object(object)
		, _prop(prop)
		, _file(file)
		, _line(line) {

	}

	inline void SetStart(const StartFunc& f) { _start = f; }
	inline void SetTick(const TickFunc& f) { _tick = f; }
	inline void SetEnd(const EndFunc& f) { _end = f; }

	virtual void OnStart(IKernel * kernel, s64 tick) {
		OASSERT(ObjectTimer::Instance()->GetObjectMgr()->FindObject(_id) == _object, "wtf");
		if (_start)
			_start(kernel, _object, tick);
	}

	virtual void OnTimer(IKernel * kernel, s32 beatCount, s64 tick) {
		OASSERT(ObjectTimer::Instance()->GetObjectMgr()->FindObject(_id) == _object, "wtf");
		if (_tick)
			_tick(kernel, _object, beatCount, tick);
	}

	virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {
		OASSERT(ObjectTimer::Instance()->GetObjectMgr()->FindObject(_id) == _object, "wtf");
		_object->SetPropInt64(_prop, 0);
		if (_end)
			_end(kernel, _object, nonviolent, tick);

		OCTimer * timer = (OCTimer *)_object->GetPropInt64(_prop);
		OASSERT(timer == this, "wtf");
		if (timer == this)
			_object->SetPropInt64(_prop, 0);
		DEL this;
	}

private:
	s64 _id;
	IObject * _object;
	const IProp * _prop;

	std::string _file;
	s32 _line;

	StartFunc _start;
	TickFunc _tick;
	EndFunc _end;
};

bool ObjectTimer::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool ObjectTimer::Launched(IKernel * kernel) {
	FIND_MODULE(_objectMgr, ObjectMgr);
    return true;
}

bool ObjectTimer::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void ObjectTimer::Start(IObject * object, const IProp * prop, s64 delay, s32 count, s64 interval, const char * file, const s32 line, const StartFunc & start, const TickFunc & tick, const EndFunc & end) {
	OCTimer * old = (OCTimer *)object->GetPropInt64(prop);
	if (old)
		_kernel->KillTimer(old);

	OCTimer * timer = NEW OCTimer(object, prop, file, line);
	timer->SetStart(start);
	timer->SetTick(tick);
	timer->SetEnd(end);
	_kernel->StartTimer(timer, delay, count, interval, file, line);
}

void ObjectTimer::Stop(IObject * object, const IProp * prop) {
	OCTimer * timer = (OCTimer *)object->GetPropInt64(prop);
	if (timer)
		_kernel->KillTimer(timer);
}

