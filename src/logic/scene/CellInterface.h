#ifndef __CELLINTERFACE_H__
#define __CELLINTERFACE_H__
#include "IKernel.h"
using namespace core;
#include <unordered_map>

#define MAX_DIMENSION 3

class IObject;
class CellInterface : public ITimer {
	struct CellObject {
		IObject * object;

		struct {
			CellObject * prev;
			CellObject * next;
		}linked[MAX_DIMENSION];
	};

public:
	CellInterface(s64 id) : _id(id) { SafeMemset(_head, sizeof(_head), 0, sizeof(_head)); }
	virtual ~CellInterface() {}

	virtual void OnStart(IKernel * kernel, s64 tick) {}
	virtual void OnTimer(IKernel * kernel, s64 tick);
	virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {}

	virtual void OnPause(IKernel * kernel, s64 tick) {}
	virtual void OnResume(IKernel * kernel, s64 tick) {}

	void Add(s64 id, IObject * object, bool update);
	void Remove(s64 id, bool update);
	void Update(s64 id, IObject * object);

	IObject * Find(s64 id);

private:
	s64 _id;
	std::unordered_map<s64, CellObject> _objects;
	CellObject * _head[MAX_DIMENSION];
};

#endif //__CELLINTERFACE_H__
