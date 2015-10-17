#ifndef __TIMEBASE_H__
#define __TIMEBASE_H__

#include "util.h"
#include "IKernel.h"
using namespace core;

class TimerList;
class TimerBase : public ITimerBase{
public:
	TimerBase(ITimer * timer, s32 count, s64 interval);
	virtual ~TimerBase() {}

	void OnTimer();
	void ForceEnd();

	void Pause(u64 jiffies);
	void Resume(u64 jiffies);

	void Release();

	inline bool IsValid() const { return _valid; }
	inline bool IsPolling() const { return _polling; }

	inline bool IsPaused() const { return _paused; }

	inline u64 GetExpire() const { return _expire; }
	inline void SetExpire(u64 expire) { _expire = expire; }

	inline void SetTimer(ITimer * timer) {
		OASSERT(timer->GetBase() == nullptr && _timer == nullptr, "timer already has a timer base or base already has a timer");
		_timer = timer;
		timer->SetBase(this);
	}

	inline void SetNext(TimerBase * next) { _next = next; }
	inline TimerBase * GetNext() const { return _next; }

	inline void SetPrev(TimerBase * prev) { _prev = prev; }
	inline TimerBase * GetPrev() const { return _prev; }

	inline void SetList(TimerList * list) { _list = list; }
	inline TimerList * GetList() const { return _list; }

private:
	TimerList * _list;
	TimerBase * _next;
	TimerBase * _prev;

	ITimer * _timer;
	bool _valid;
	bool _polling;

	u64 _expire;
	u64 _interval;
	s32 _count;
	bool _started;

	u64 _pauseTick;
	bool _paused;
};

#endif //__TIMEBASE_H__

