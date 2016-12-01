#ifndef __OASSERT_h__
#define __OASSERT_h__
#include <string.h>
#include <stdio.h>

void __OAssert(const char * file, int line, const char * funname, const char * debug);

#define SafeSprintf snprintf

#define OASSERT(p, format, ...) { \
    char debug[4096]; \
    SafeSprintf(debug, sizeof(debug), format, __VA_ARGS__); \
    ((p) ? (void)0 : (void)__OAssert(__FILE__, __LINE__, __FUNCTION__, debug)); \
}

inline void __OMemcpy(void * dst, const int maxSize, const void * src, const int size) {
	OASSERT(size <= maxSize, "memcpy out of range");

	const int len = (size > maxSize ? maxSize : size);
	memcpy(dst, src, len);
}

inline void __OMemset(void * dst, const int maxSize, const int val, const int size) {
	OASSERT(size <= maxSize, "memset out of range");

	const int len = (size > maxSize ? maxSize : size);
	memset(dst, val, len);
}

#define SafeMemcpy __OMemcpy
#define SafeMemset __OMemset

#endif //__OASSERT_h__
