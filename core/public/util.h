#ifndef __util_h__
#define __util_h__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef WIN32
#ifndef _WINSOCK2API_
#include <WinSock2.h>
#else
#include <Windows.h>
#endif
#include <Shlwapi.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "shlwapi.lib")
#else
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

typedef unsigned char u8;
typedef unsigned short u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef char s8;
typedef short s16;
typedef int32_t s32;
typedef int64_t s64;

#ifdef WIN32
#ifdef KERNEL_EXPORT
#define KERNEL_API __declspec(dllexport)
#else
#define KERNEL_API __declspec(dllimport)
#endif
#else
#define KERNEL_API
#endif

#ifdef __cplusplus
extern "C" {
#endif
KERNEL_API void __OAssert(const char * file, s32 line, const char * funname, const char * debug);
#ifdef __cplusplus
};
#endif

#define MALLOC(size) malloc(size)
#define FREE(p) free(p)
#define REALLOC(p, size) realloc(p, size)

#ifdef __cplusplus
#define NEW new
#define DEL delete
#endif

#define SafeSprintf snprintf

#define OASSERT(p, format, ...) { \
    char debug[4096]; \
    SafeSprintf(debug, sizeof(debug), format, ##__VA_ARGS__); \
    ((p) ? (void)0 : (void)__OAssert(__FILE__, __LINE__, __FUNCTION__, debug)); \
}

#ifdef WIN32
#define CSLEEP(t) Sleep(t)
#else
#define CSLEEP(t) usleep((t) * 1000)
#endif

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

#ifndef MAX_PATH
#define MAX_PATH 260
#endif //MAX_PATH

#ifdef __cplusplus
#define ALLOCATOR(T) std::allocator<T>

#include <functional>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <list>
#include <set>
#include <map>

constexpr s64 CalcUniqueId(s64 hash, const char * str) {
	return *str ? CalcUniqueId((hash * 131 + (*str)) % 4294967295, str + 1) : hash;
}
#endif
#endif //__util_h__
