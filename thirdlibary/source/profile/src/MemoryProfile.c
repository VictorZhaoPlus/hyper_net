#include "profile.h"
#include "HashMap.h"
#include <stdio.h>
#include "oassert.h"

#define NAME_LEN 256

#ifdef __cplusplus
extern "C" {
#endif
	char g_memoryProfile = 0;
	char g_memoryProfileInited = 0;
	struct HashMap g_memoryProfileMap;
	struct HashMap g_memoryProfilePtrMap;
	MPWRITE g_memoryProfileWriter;
#ifdef WIN32
	__declspec(thread) int g_mainThread = 0;
#else
	__thread int g_mainThread = 0;
#endif

	struct LineDesc {
		char filename[NAME_LEN];
		char function[NAME_LEN];
		int line;
	};

	int LineHash(const void * context, const int size) {
		OASSERT(size == sizeof(struct LineDesc), "wtf");
		struct LineDesc * desc = (struct LineDesc *)context;

		return desc->line;
	}

	char LineDescEq(const void * a, const void * b, const int size) {
		OASSERT(size == sizeof(struct LineDesc), "wtf");
		struct LineDesc * descA = (struct LineDesc *)a;
		struct LineDesc * descB = (struct LineDesc *)b;

		return strcmp(descA->filename, descB->filename) == 0 && strcmp(descA->function, descB->function) == 0 && descA->line == descB->line;
	}

	int PtrHash(const void * context, const int size) {
		OASSERT(size == sizeof(long long), "wtf");

		return (int)(*(long long*)context);
	}

	char PtrDescEq(const void * a, const void * b, const int size) {
		OASSERT(size == sizeof(long long), "wtf");

		return (*(long long*)a) == (*(long long*)b);
	}


	void MPInit() {
		HashMapInit(&g_memoryProfileMap, sizeof(struct LineDesc), sizeof(int), 1023, LineHash, LineDescEq);
		HashMapInit(&g_memoryProfilePtrMap, sizeof(long long), sizeof(long long), 1023, PtrHash, PtrDescEq);
		g_memoryProfileInited = 1;
		g_mainThread = 1;
	}

	void MPMarkMalloc(const void * p, const char * file, const int line, const char * function) {
		struct LineDesc desc;

		if (!g_memoryProfile || !g_mainThread)
			return;

		SafeSprintf(desc.filename, sizeof(desc.filename), file);
		SafeSprintf(desc.function, sizeof(desc.function), function);
		desc.line = line;

		void * descData = HashMapGet(&g_memoryProfileMap, &desc, sizeof(desc), sizeof(int));
		void * ptrData = HashMapGet(&g_memoryProfilePtrMap, &p, sizeof(p), sizeof(long long));
		*(long long*)ptrData = (long long)descData;
		++(*(int*)descData);
	}

	void MPMarkRealloc(const void * p1, const void * p2) {
		if (!g_memoryProfile || !g_mainThread)
			return;

		void * ptrData1 = HashMapFind(&g_memoryProfilePtrMap, &p1, sizeof(p1), sizeof(long long));
		if (ptrData1) {
			void * ptrData2 = HashMapGet(&g_memoryProfilePtrMap, &p2, sizeof(p2), sizeof(long long));
			*(long long*)ptrData2 = *(long long*)ptrData1;
			HashMapRemove(&g_memoryProfilePtrMap, &p1, sizeof(p1));
		}
	}

	void MPMarkFree(const void * p) {
		if (!g_memoryProfile || !g_mainThread)
			return;

		void * ptrData = HashMapFind(&g_memoryProfilePtrMap, &p, sizeof(p), sizeof(long long));
		if (ptrData) {
			--(*(int*)(*(long long*)ptrData));
			HashMapRemove(&g_memoryProfilePtrMap, &p, sizeof(p));
		}
	}

	void MPClear() {
		if (!g_memoryProfile && g_memoryProfileInited || !g_mainThread)
			return;

		HashMapClear(&g_memoryProfileMap);
		HashMapClear(&g_memoryProfilePtrMap);
	}

	void MPCtrl(const char * name, const void * context, const int size) {
		if (strcmp(name, "open") == 0) {
			OASSERT(size == sizeof(char), "wtf");
			if (size == sizeof(char)) {
				g_memoryProfile = *(char*)context;
				if (!g_memoryProfile)
					MPClear();
				else {
					if (!g_memoryProfileInited)
						MPInit();
				}
			}
		}
		else if (strcmp(name, "clear") == 0) {
			MPClear();
		}
	}

	void MPDumpHelper(const void * key, const int keySize, const void * data, const int dataSize) {
		char msg[1024];
		struct LineDesc * desc = (struct LineDesc *)key;
		int count = *(int*)data;

		SafeSprintf(msg, sizeof(msg), "%d:%s:%s count %d", desc->line, desc->filename, desc->function, count);
		g_memoryProfileWriter(msg);
	}

	void MPDump(MPWRITE writer) {
		if (g_memoryProfile) {
			g_memoryProfileWriter = writer;
			HashMapTraversal(&g_memoryProfileMap, MPDumpHelper);
		}
	}

#ifdef __cplusplus
};
#endif

