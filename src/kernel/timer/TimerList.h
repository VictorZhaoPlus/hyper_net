#ifndef __TIMELIST_H__
#define __TIMELIST_H__

#include "util.h"

class TimerBase;
class TimerList {
public:
	TimerList() : _head(nullptr), _tail(nullptr) {}
	virtual ~TimerList() {}

	void PushBack(TimerBase * p);
	TimerBase* PopFront();
	void Remove(TimerBase* p);

	inline bool empty() { return _head == nullptr; }

private:
	TimerList(TimerList&);
	TimerList& operator=(TimerList &);

	TimerBase * _head;
	TimerBase * _tail;
};

#endif //__TIMELIST_H__
