#include "util.h"
#include <assert.h>
#include <jemalloc/jemalloc.h>
#include <profile/profile.h>

#ifdef WIN32
#pragma comment (lib, "jemalloc.lib")
#pragma comment (lib, "profile.lib")
#endif

#ifdef __cplusplus
extern "C" {
#endif

void __OAssert(const char * file, s32 line, const char * func, const char * debug) {
	fflush(stdout);
	fprintf(stderr,
		"\nAsssertion failed : \n=======assert string=======\nfile:%s\nline:%d\nfunction:%s\ndebug:%s\n===========================\n",
		file, line, func, debug);
	fflush(stderr);
	assert(false);
}

void * __Malloc(size_t size, const char * file, s32 line, const char * funname) {
	void * p = je_malloc(size);
	MPMarkMalloc(p, file, line, funname);
	return p;
}

void * __Realloc(void * p, size_t size, const char * file, s32 line, const char * funname) {
	void * p2 = je_realloc(p, size);
	MPMarkRealloc(p, p2);
	return p2;
}

void __Free(void * p, const char * file, s32 line, const char * funname) {
	je_free(p);
	MPMarkFree(p);
}

#ifdef __cplusplus
}
#endif

void * operator new(size_t size) noexcept {
	return je_malloc(size);
}

void * operator new[](size_t size) noexcept {
	return je_malloc(size);
}

void operator delete(void * p) noexcept {
	je_free(p);
	MPMarkFree(p);
}

void operator delete[](void * p) noexcept {
	je_free(p);
	MPMarkFree(p);
}

void * operator new(size_t size, const char * file, s32 line, const char * funname) {
	void * p = je_malloc(size);
	MPMarkMalloc(p, file, line, funname);
	return p;
}

void * operator new[](size_t size, const char * file, s32 line, const char * funname) {
	void * p = je_malloc(size);
	MPMarkMalloc(p, file, line, funname);
	return p;
}
