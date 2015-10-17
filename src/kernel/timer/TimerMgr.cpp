#include "TimerMgr.h"
#include "TimerBase.h"
#include "TimerGear.h"
#include "TimerList.h"
#include "tools.h"

bool TimerMgr::Ready() {
	return true;
}

bool TimerMgr::Initialize() {
	_timerGear[4] = NEW TimerGear(TQ_TVN_SIZE, 0);
	_timerGear[3] = NEW TimerGear(TQ_TVN_SIZE, _timerGear[4]);
	_timerGear[2] = NEW TimerGear(TQ_TVN_SIZE, _timerGear[3]);
	_timerGear[1] = NEW TimerGear(TQ_TVN_SIZE, _timerGear[2]);
	_timerGear[0] = NEW TimerGear(TQ_TVR_SIZE, _timerGear[1]);

	_running = NEW TimerList;
	_suspended = NEW TimerList;
	return true;
}

void TimerMgr::Loop() {
	static s64 last = tools::GetTimeMillisecond();
	s64 tick = tools::GetTimeMillisecond();

	s32 count = (s32)(tick - last) / JIFFIES_INTERVAL;
	for (s32 i = 0; i < count; ++i)
		Update();
	last += count * JIFFIES_INTERVAL;

	while (!_running->empty()) {
		TimerBase * base = _running->PopFront();
		OASSERT(base, "where is timer base");

		base->OnTimer();
		if (!base->IsValid())
			Delete(base);
		else if (base->IsPaused())
			_suspended->PushBack(base);
		else
			Schedule(base);
	}
}

void TimerMgr::Destroy() {
	for (s32 i = 0; i < 5; ++i) {
		DEL _timerGear[i];
	}
	DEL _running;
	DEL _suspended;
	DEL this;
}

void TimerMgr::StartTimer(ITimer * timer, s64 delay, s32 count, s64 interval) {
	TimerBase * base = (TimerBase*)timer->GetBase();
	if (base != nullptr) {
		OASSERT(false, "already has timer base");
		return;
	}

	if (interval < JIFFIES_INTERVAL)
		interval = JIFFIES_INTERVAL;
	
	base = NEW TimerBase(timer, count, interval / JIFFIES_INTERVAL);
	if (delay > 0) {
		if (delay < JIFFIES_INTERVAL)
			delay = JIFFIES_INTERVAL;
	}
	base->SetExpire(_jiffies + delay / JIFFIES_INTERVAL);
	Schedule(base);
}

void TimerMgr::KillTimer(ITimer * timer) {
	TimerBase * base = (TimerBase*)timer->GetBase();
	if (base == nullptr) {
		OASSERT(false, "where is timer base");
		return;
	}

	base->ForceEnd();
	if (!base->IsPolling()) {
		OASSERT(base->GetList(), "base is not in a timer list");

		base->GetList()->Remove(base);
		Delete(base);
	}
}

void TimerMgr::PauseTimer(ITimer * timer) {
	TimerBase * base = (TimerBase*)timer->GetBase();
	if (base == nullptr) {
		OASSERT(false, "where is timer base");
		return;
	}

	if (!base->IsValid())
		return;

	base->Pause(_jiffies);
	if (base->IsValid() && !base->IsPolling()) {
		OASSERT(base->GetList(), "base is not in a timer list");

		base->GetList()->Remove(base);
		_suspended->PushBack(base);
	}
}

void TimerMgr::ResumeTimer(ITimer * timer) {
	TimerBase * base = (TimerBase*)timer->GetBase();
	if (base == nullptr) {
		OASSERT(false, "where is timer base");
		return;
	}
	OASSERT(base->GetList() == _suspended, "wtf");

	if (!base->IsValid())
		return;

	base->Resume(_jiffies);
	if (base->IsValid() && !base->IsPolling()) {
		base->GetList()->Remove(base);

		Schedule(base);
	}
}

void TimerMgr::Schedule(TimerBase * base) {
	TimerList * vec = FindTimerList(base->GetExpire());
	OASSERT(vec, "find timer list failed");

	vec->PushBack(base);
}

void TimerMgr::MoveToRunning(TimerBase * base) {
	_running->PushBack(base);
}

TimerList* TimerMgr::FindTimerList(u64 expire) {
	u64 live = expire - _jiffies;
	TimerList* vec = 0;
	if (live < TQ_TVR_SIZE)
		vec = _timerGear[0]->GetTimerList(expire & TQ_TVR_MASK);
	else if (live < (1 << (TQ_TVR_BITS + TQ_TVN_BITS)))
		vec = _timerGear[1]->GetTimerList((expire >> TQ_TVR_BITS) & TQ_TVN_MASK);
	else if (live < (1 << (TQ_TVR_BITS + 2 * TQ_TVN_BITS)))
		vec = _timerGear[2]->GetTimerList((expire >> (TQ_TVR_BITS + TQ_TVN_BITS)) & TQ_TVN_MASK);
	else if (live < (1 << (TQ_TVR_BITS + 3 * TQ_TVN_BITS)))
		vec = _timerGear[3]->GetTimerList((expire >> (TQ_TVR_BITS + 2 * TQ_TVN_BITS)) & TQ_TVN_MASK);
	else if ((long long)live < 0)
		vec = _running;
	else
		vec = _timerGear[4]->GetTimerList((expire >> (TQ_TVR_BITS + 3 * TQ_TVN_BITS)) & TQ_TVN_MASK);

	return vec;
}

void TimerMgr::Update() {
	OASSERT(_timerGear[0], "where is timer gear");

	_timerGear[0]->CheckHighGear();
	++_jiffies;
	_timerGear[0]->Update();
}

void TimerMgr::Delete(TimerBase * base) {
	base->Release();
	DEL base;
}

