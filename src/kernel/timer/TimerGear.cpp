#include "TimerGear.h"
#include "TimerList.h"
#include "TimerMgr.h"
#include "TimerBase.h"

TimerGear::TimerGear(int maxMoveDst, TimerGear* nextGear)
	: _timerVec(nullptr)
	, _nextGear(nextGear)
	, _curMoveDst(0)
	, _maxMoveDst(maxMoveDst) {

	_timerVec = NEW TimerList[_maxMoveDst];
}

TimerGear::~TimerGear() {
	if (_timerVec) {
		DEL[] _timerVec;
		_timerVec = nullptr;
	}
}

TimerList * TimerGear::GetTimerList(s32 index) {
	OASSERT(index < _maxMoveDst, "index out of range");

	return _timerVec + index;
}

void TimerGear::CheckHighGear() {
	if (_curMoveDst >= _maxMoveDst)
		_curMoveDst = 0;

	if (_curMoveDst == 0) {
		if(_nextGear){
			_nextGear->UpdateToLowGear();
		}
	}
}

void TimerGear::Update() {
	if (_curMoveDst >= _maxMoveDst )
		_curMoveDst = 0;

	TimerList* currentList = &_timerVec[_curMoveDst];
	if (!currentList) {
		_curMoveDst++;
		return;
	}

	while (!currentList->empty()) {
		TimerBase * base = currentList->PopFront();
		if (!base)
			continue;

		TimerMgr::Instance()->MoveToRunning(base);
	}

	++_curMoveDst;
	if (_curMoveDst == _maxMoveDst)
		_curMoveDst = 0;
}

void TimerGear::UpdateToLowGear() {
	if (_curMoveDst >= _maxMoveDst)
		_curMoveDst = 0;

	if (_curMoveDst == 0) {
		if (_nextGear)
			_nextGear->UpdateToLowGear();
	}

	TimerList* currentList = &_timerVec[_curMoveDst];
	if (!currentList) {
		++_curMoveDst;
		return;
	}

	while (!currentList->empty()) {
		TimerBase * base = currentList->PopFront();
		if (!base)
			continue;

		TimerMgr::Instance()->Schedule(base);
	}

	++_curMoveDst;
}
