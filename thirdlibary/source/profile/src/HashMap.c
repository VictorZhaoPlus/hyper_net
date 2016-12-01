#include "HashMap.h"
#include "oassert.h"
#include <stdlib.h>

struct HashKey * HashMapGetInList(struct HashMap * h, struct HashHead * head, const void * key) {
	struct HashKey * ptr = head->keys;
	while (ptr) {
		if ((h->eq)(((char *)ptr) + sizeof(struct HashKey), key, h->keySize)) {
			break;
		}

		ptr = ptr->next;
	}
	return ptr;
}

struct HashKey * HashMapAddInList(struct HashMap * h, struct HashHead * head, const void * key) {
	struct HashKey * ptr = (struct HashKey *)malloc(sizeof(struct HashKey) + h->keySize + h->dataSize);
	SafeMemset(ptr, sizeof(struct HashKey) + h->keySize + h->dataSize, 0, sizeof(struct HashKey) + h->keySize + h->dataSize);
	SafeMemcpy((char*)ptr + sizeof(struct HashKey), h->keySize, key, h->keySize);

	ptr->next = head->keys;
	ptr->prev = NULL;
	if (head->keys)
		head->keys->prev = ptr;
	head->keys = ptr;

	return ptr;
}

void HashMapRemoveInList(struct HashMap * h, struct HashHead * head, const void * key) {
	struct HashKey * ptr = head->keys;
	struct HashKey * prev, * next;

	while (ptr) {
		if ((h->eq)(((char *)ptr) + sizeof(struct HashKey), key, h->keySize)) {
			break;
		}

		ptr = ptr->next;
	}

	if (ptr) {
		if (head->keys == ptr)
			head->keys = ptr->next;

		prev = ptr->prev;
		next = ptr->next;

		if (prev)
			prev->next = next;
		if (next)
			next->prev = prev;

		free(ptr);
	}
}

void HashMapClearList(struct HashMap * h, struct HashHead * head) {
	struct HashKey * ptr = head->keys;
	struct HashKey * current;
	while (ptr) {
		current = ptr;
		ptr = ptr->next;
		free(current);
	}
	head->keys = NULL;
}

void HashMapTraversalList(struct HashMap * h, struct HashHead * head, HashMapTraversalFunType f) {
	struct HashKey * ptr = head->keys;
	while (ptr) {
		f(((char *)ptr) + sizeof(struct HashKey), h->keySize, ((char *)ptr) + sizeof(struct HashKey) + h->keySize, h->dataSize);
		ptr = ptr->next;
	}
}

void HashMapInit(struct HashMap * h, int keySize, int dataSize, int bucketSize, HashMapHashFuncType hash, HashMapEquipFuncType eq) {
	h->bucket = (struct HashHead*)malloc(sizeof(struct HashHead) * bucketSize);
	SafeMemset(h->bucket, bucketSize * sizeof(struct HashHead), 0, bucketSize * sizeof(struct HashHead));
	h->keySize = keySize;
	h->dataSize = dataSize;
	h->bucketSize = bucketSize;
	h->hash = hash;
	h->eq = eq;
}

void * HashMapFind(struct HashMap * h, const void * key, const int keySize, const int dataSize) {
	unsigned int hash;
	struct HashKey * ptr;

	OASSERT(keySize == h->keySize && h->dataSize == dataSize, "wtf");

	hash = (h->hash)(key, h->keySize);
	ptr = HashMapGetInList(h, &(h->bucket[hash % h->bucketSize]), key);
	if (ptr)
		return ((char *)ptr) + sizeof(struct HashKey) + keySize;
	return NULL;
}

void * HashMapGet(struct HashMap * h, const void * key, const int keySize, const int dataSize) {
	unsigned int hash;
	struct HashKey * ptr;

	OASSERT(keySize == h->keySize && h->dataSize == dataSize, "wtf");

	hash = (h->hash)(key, h->keySize);
	ptr = HashMapGetInList(h, &(h->bucket[hash % h->bucketSize]), key);
	if (!ptr)
		ptr = HashMapAddInList(h, &(h->bucket[hash % h->bucketSize]), key);
	OASSERT(ptr, "wtf");
	if (ptr)
		return ((char *)ptr) + sizeof(struct HashKey) + keySize;
	return NULL;
}

void HashMapRemove(struct HashMap * h, const void * key, const int keySize) {
	unsigned int hash;

	OASSERT(keySize == h->keySize, "wtf");

	hash = (h->hash)(key, h->keySize);
	HashMapRemoveInList(h, &(h->bucket[hash % h->bucketSize]), key);
}

void HashMapClear(struct HashMap * h) {
	int i = 0;
	for (i = 0; i < h->bucketSize; ++i)
		HashMapClearList(h, &(h->bucket[i]));
}

void HashMapTraversal(struct HashMap * h, HashMapTraversalFunType f) {
	int i = 0;
	for (i = 0; i < h->bucketSize; ++i)
		HashMapTraversalList(h, &(h->bucket[i]), f);
}

