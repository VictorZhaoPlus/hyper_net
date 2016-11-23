#include "TimerBase.h"
#include "kernel.h"
#include "tools.h"

TimerBase::TimerBase(ITimer * timer, s32 count, s64 interval) {
	_list = nullptr;
	_next = nullptr;
	_prev = nullptr;

	_timer = timer;
	_valid = true;
	_polling = false;

	_expire = 0;
	_interval = interval;
	_count = count;
	_started = false;

	_pauseTick = 0;
	_paused = false;
}

void TimerBase::OnTimer() {
	OASSERT(_timer, "where is timer ???");

	_polling = true;
	if (!_started) {
		_timer->OnStart(Kernel::Instance(), tools::GetTimeMillisecond());
		_started = true;
	}
	else {
		_timer->OnTimer(Kernel::Instance(), tools::GetTimeMillisecond());

		if (_valid) {
			if (_count > 0)
				--_count;
		}
	}
	_polling = false;
	_expire += _interval;

	if (_count == 0) {
		_valid = false;
		_timer->OnEnd(Kernel::Instance(), true, tools::GetTimeMillisecond());
	}
}

void TimerBase::ForceEnd() {
	OASSERT(_valid, "timer is already invalid");

	_valid = false;
	_timer->OnEnd(Kernel::Instance(), false, tools::GetTimeMillisecond());
}

void TimerBase::Pause(u64 jiffies) {
	if (_valid) {
		OASSERT(_paused, "timer already is paused");

		_pauseTick = jiffies;
		_paused = true;
		_timer->OnPause(Kernel::Instance(), tools::GetTimeMillisecond());
	}
}

void TimerBase::Resume(u64 jiffies) {
	if (_valid) {
		OASSERT(!_paused, "timer is not paused");

		_expire = jiffies + _expire - _pauseTick;
		_paused = false;
		_timer->OnResume(Kernel::Instance(), tools::GetTimeMillisecond());
	}
}

void TimerBase::Release() {
	OASSERT(_timer, "where is timer ???");

	_timer->SetBase(nullptr);
	_timer = nullptr;
}

void TimerBase::AdjustExpire(u64 now) {
	s64 live = (s64)(_expire - now);
	if (live < 0 && abs(live) > (s64)_interval) {
		_expire += (abs(live) / _interval) * _interval;
	}
}
