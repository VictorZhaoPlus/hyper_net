#ifndef __STRING_UNIQUE_H__
#define __STRING_UNIQUE_H__
#include "util.h"

constexpr s64 CalcUniqueId(s64 hash, const char * str) {
	return *str ? CalcUniqueId((hash * 131 + (*str)) % 4294967295, str + 1) : hash;
}

template <s64 ...>
struct UniqueGetter {
	template <typename T>
	struct Inner {
		template <typename R>
		inline static R Get(const char * name) {
			static R r = T::Get(name);
			return r;
		}
	};
};

#endif // __STRING_UNIQUE_H__
