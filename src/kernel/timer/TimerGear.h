#ifndef __TIMEGEAR_H__
#define __TIMEGEAR_H__

#include "util.h"

class TimerList;
class TimerGear {
public:
	TimerGear(int maxMoveDst, TimerGear* nextGear);
	~TimerGear();

	TimerList * GetTimerList(s32 index);
	void CheckHighGear();
	void Update();
	void UpdateToLowGear();

private:
	TimerGear(TimerGear&);
	TimerGear& operator=(TimerGear&);

	TimerList * _timerVec;//
	TimerGear * _nextGear;//
	s32 _curMoveDst;//
	s32 _maxMoveDst;//
};

#endif //__TIMEMGR_H__
