#ifndef __OPOOL_H__
#define __OPOOL_H__
#include "util.h"
#include <list>

namespace olib {
	template <typename T, s32 chunkSize = 64, s32 chunkCount = 1>
	class Pool {
	public:
		Pool() {
			
		}
		
		~Pool() {
			
		}
	
		T * Create(const char * file, const s32 line) {
			return NEW T();
		}
		
		template <typename... Args>
		T * Create(const char * file, const s32 line, Args... args) {
			return new T(args...);
		}
		
		void Recover(T * t) {
			DEL t;
		}
	private:	
	};
}

#define CREATE_FROM_POOL(pool, ...) pool.Create(__FILE__, __LINE__, __VA_ARGS__);

#endif //__OPOOL_H__
