#ifndef __HASHMAP_h__
#define __HASHMAP_h__

typedef unsigned int (*HashMapHashFuncType)(const void * context, const int size);
typedef char (*HashMapEquipFuncType)(const void * a, const void * b, const int size);
typedef void (*HashMapTraversalFunType)(const void * key, const int keySize, const void * data, const int dataSize);

struct HashKey {
	int reserved;
	struct HashKey * prev;
	struct HashKey * next;
};

struct HashHead {
	struct HashKey * keys;
};

struct HashMap {
	struct HashHead * bucket;

	int keySize;
	int dataSize;
	int bucketSize;
	HashMapHashFuncType hash;
	HashMapEquipFuncType eq;
};

void HashMapInit(struct HashMap * h, int keySize, int dataSize, int bucketSize, HashMapHashFuncType hash, HashMapEquipFuncType eq);
void * HashMapFind(struct HashMap * h, const void * key, const int keySize, const int dataSize);
void * HashMapGet(struct HashMap * h, const void * key, const int keySize, const int dataSize);
void HashMapRemove(struct HashMap * h, const void * key, const int keySize);
void HashMapClear(struct HashMap * h);
void HashMapTraversal(struct HashMap * h, HashMapTraversalFunType f);


#endif //__HASHMAP_h__
