#include "oassert.h"
#include <assert.h>
#include <stdio.h>

void __OAssert(const char * file, int line, const char * func, const char * debug) {
	fflush(stdout);
	fprintf(stderr,
		"\nAsssertion failed : \n=======assert string=======\nfile:%s\nline:%d\nfunction:%s\ndebug:%s\n===========================\n",
		file, line, func, debug);
	fflush(stderr);
	assert(0);
}
