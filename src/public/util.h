#ifndef __util_h__
#define __util_h__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <limits.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef char s8;
typedef short s16;
typedef int32_t s32;
typedef int64_t s64;

#define MALLOC malloc
#define FREE free
#define REALLOC realloc
#define NEW new
#define DEL delete
#define ALLOCATOR(T) std::allocator<T>

#define SafeSprintf snprintf

#ifdef __cplusplus
extern "C" {
#endif
    void __OAssert(const char * file, int line, const char * funname, const char * debug);
#ifdef __cplusplus
};
#endif

#define OASSERT(p, format, a...) { \
    char debug[4096] = {0}; \
    SafeSprintf(debug, sizeof(debug), format, ##a); \
    ((p) ? (void)0 : (void)__OAssert(__FILE__, __LINE__, __FUNCTION__, debug)); \
}

#define CSLEEP(t) usleep(t)

#ifdef __cplusplus
extern "C" {
#endif
	inline void __OMemcpy(void * dst, const s32 maxSize, const void * src, const s32 size) {
		OASSERT(size <= maxSize, "memcpy out of range");

		const s32 len = (size > maxSize ? maxSize : size);
		memcpy(dst, src, len);
	}

	inline void __OMemset(void * dst, const s32 maxSize, const s32 val, const s32 size) {
		OASSERT(size <= maxSize, "memset out of range");

		const s32 len = (size > maxSize ? maxSize : size);
		memset(dst, val, len);
	}
#ifdef __cplusplus
};
#endif

#define SafeMemcpy __OMemcpy
#define SafeMemset __OMemset

#define MAX_PATH 260

#endif //__util_h__
