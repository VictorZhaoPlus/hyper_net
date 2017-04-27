#ifndef __TIMEMGR_H__
#define __TIMEMGR_H__

#include "util.h"
#include "singleton.h"
#include "IKernel.h"
using namespace core;

#define JIFFIES_INTERVAL 20

class TimerList;
class TimerBase;
class TimerGear;
class TimerMgr : public OSingleton<TimerMgr> {
	friend class OSingleton<TimerMgr>;

	enum
	{
		TQ_TVN_BITS = 6,
		TQ_TVR_BITS = 8,
		TQ_TVN_SIZE = 1<<TQ_TVN_BITS,//64
		TQ_TVR_SIZE = 1<<TQ_TVR_BITS,//256
		TQ_TVN_MASK = TQ_TVN_SIZE -1,//63
		TQ_TVR_MASK = TQ_TVR_SIZE -1,//255
	};

public:
	bool Ready();
	bool Initialize();
	void Loop();
	void Destroy();

	void StartTimer(ITimer * timer, s64 delay, s32 count, s64 interval, const char * file, const s32 line);
	void KillTimer(ITimer * timer);
	void PauseTimer(ITimer * timer);
	void ResumeTimer(ITimer * timer);

	void Schedule(TimerBase * base);
	void MoveToRunning(TimerBase * base);

	u64 Jiffies() const { return _jiffies; }

public:
	TimerMgr() : _jiffies(0) {
		memset(_timerGear, 0, sizeof(_timerGear));
	}
	~TimerMgr() {}

	TimerList * FindTimerList(u64 expire);
	void Update();
	void Delete(TimerBase * base);

private:
	u64 _jiffies;
	TimerGear * _timerGear[5];
	TimerList * _running;
	TimerList * _suspended;
};

#endif //__TIMEMGR_H__
