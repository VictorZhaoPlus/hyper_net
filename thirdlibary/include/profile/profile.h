#ifndef __MEMORYPROFILE_h__
#define __MEMORYPROFILE_h__

#ifdef WIN32
#define MPEXPORT __declspec(dllexport)
#else
#define MPEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

	typedef void(*MPWRITE) (const char * buf);

	MPEXPORT void MPMarkMalloc(const void * p, const char * file, const int line, const char * function);
	MPEXPORT void MPMarkRealloc(const void * p1, const void * p2);
	MPEXPORT void MPMarkFree(const void * p);

	MPEXPORT void MPCtrl(const char * name, const void * context, const int size);
	MPEXPORT void MPDump(MPWRITE writer);

#ifdef __cplusplus
};
#endif

#endif //__OMEMORYPROFILE_h__
