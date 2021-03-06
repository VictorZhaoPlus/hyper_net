/* 
 * File:   Memory.h
 * Author: Alax
 *
 * Created on March 3, 2015, 10:46 AM
 */
#ifndef __MEMORY_H__
#define __MEMORY_H__
#include "util.h"
#include "MemoryPool.h"

struct Layout {
	s32 offset;
	s32 size;
};

class Memory {
public:
	Memory(s32 size) 
		: _buff(nullptr)
		, _size(size) {
		OASSERT(size > 0, "nullptr PropInfo");

		_buff = (char*)MemoryPool::Instance()->Malloc(_size);
		SafeMemset(_buff, _size, 0, _size);
	}

	virtual ~Memory() {
		MemoryPool::Instance()->Free(_buff, _size);
	}

    inline void Set(const Layout * info, const void * data, const s32 size) {
        OASSERT(info->offset + info->size <= _size, "wtf");
		OASSERT(size <= info->size, "wtf");

        SafeMemcpy(_buff + info->offset, info->size, data, size);
    }

	inline void * Get(const Layout * info) {
		OASSERT(info->offset + info->size <= _size, "wtf");
        return _buff + info->offset;
    }

    inline void Clear() { SafeMemset(_buff, _size, 0, _size); }

private:
    char * _buff;
    const s32 _size;
};

#endif // __MEMORY_H__

