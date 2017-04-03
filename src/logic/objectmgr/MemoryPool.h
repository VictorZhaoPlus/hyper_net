/* 
 * File:   MemoryPool.h
 * Author: Alax
 *
 * Created on March 3, 2015, 10:46 AM
 */
#ifndef __MEMORYPOOL_H__
#define __MEMORYPOOL_H__
#include "util.h"
#include "OPool.h"
#include <unordered_map>
#include "singleton.h"

class MemoryPool : public OSingleton<MemoryPool>{
public:
	MemoryPool() {}
	virtual ~MemoryPool() {
		for (auto itr = _pools.begin(); itr != _pools.end(); ++itr) {
			DEL itr->second;
		}
		_pools.clear();
	}

	bool Ready() { return true; }

	template <typename T, typename... Args>
	inline T * Create(const char * file, const s32 line, Args... args) {
		auto * pool = FindPool(sizeof(T));
		void * p = pool->Create();

		return new(p) T(args...);
	}

	template <typename T>
	inline void Recover(T* t) {
		t->~T();

		auto * pool = FindPool(sizeof(T));
		pool->Recover(t);
	}

	inline void * Malloc(const s32 size) {
		auto * pool = FindPool(size);
		return pool->Create();
	}

	inline void Free(void * p, s32 size) {
		auto * pool = FindPool(size);
		pool->Recover(p);
	}

protected:
	inline olib::Pool<> * FindPool(const s32 size) {
		auto itr = _pools.find(size);
		if (itr == _pools.end()) {
			_pools[size] = NEW olib::Pool<>(size);
		}

		return _pools[size];
	}

private:
	std::unordered_map<s32, olib::Pool<> *> _pools;
};

#endif // __MEMORYPOOL_H__

