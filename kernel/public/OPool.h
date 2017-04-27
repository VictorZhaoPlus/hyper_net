#ifndef __OPOOL_H__
#define __OPOOL_H__
#include "util.h"
#include <unordered_map>

#define MIN_CHUNK_COUNT 20

namespace olib {
	template <s32 pageSize = 4096, s32 bigPageSize = 128 * 4096>
	class Pool {
#pragma pack(push, 1)
		struct ChunkList;
		struct Chunk {
			ChunkList * parent;
			Chunk * next;
			s8 state;
		};

		struct ChunkList {
			s8 recover;
			s32 count;
			ChunkList * prev;
			ChunkList * next;
			Chunk * free;
		};

#pragma pack(pop)
		enum {
			IS_FREE = 0,
			USED,
		};

	public:
		Pool(s32 size) {
			_size = size;

			if ((pageSize - sizeof(ChunkList)) / (size + sizeof(Chunk)) > MIN_CHUNK_COUNT)
				_chunkCount = (pageSize - sizeof(ChunkList)) / (size + sizeof(Chunk)) + 1;
			else
				_chunkCount = (bigPageSize - sizeof(ChunkList)) / (size + sizeof(Chunk)) + 1;

			_head = nullptr;

			Alloc(false);
		}

		~Pool() {
			ChunkList * chunkList = _head;
			while (chunkList) {
				ChunkList * tmp = chunkList;
				chunkList = chunkList->next;
				FREE(tmp);
			}
		}

		inline void * Create() { return Fetch(); }
		inline void Recover(void * p) { Release(p); }

	protected:
		inline void * Fetch() {
			if (_freeCount > 0)
				return Fetch(true);
			else {
				Alloc(true);
				return Fetch(false);
			}
		}

		inline void * Fetch(bool forward) {
			ChunkList * chunkList = forward ? _head : _tail;
			while (chunkList) {
				if (chunkList->free) {
					Chunk * chunk = chunkList->free;
					chunkList->free = chunkList->free->next;
					++chunkList->count;
					OASSERT(chunk->state == IS_FREE, "wtf");

					chunk->next = nullptr;
					chunk->state = USED;
					return (char*)chunk + sizeof(Chunk);
				}
				chunkList = forward ? chunkList->next : chunkList->prev;
			}

			OASSERT(false, "wtf");
			return nullptr;
		}

		inline void Release(void * p) {
			Chunk * chunk = (Chunk *)((char*)p - sizeof(Chunk));
			OASSERT(chunk->state == USED && chunk->parent != nullptr, "wtf");
			ChunkList * chunkList = chunk->parent;

			chunk->next = chunkList->free;
			chunkList->free = chunk;
			chunk->state = IS_FREE;
			--chunkList->count;
			OASSERT(chunkList->count >= 0, "wtf");

			if (chunkList->count == 0 && chunkList->recover) {
				if (chunkList->prev)
					chunkList->prev->next = chunkList->next;
				else
					_head = chunkList->next;

				if (chunkList->next)
					chunkList->next->prev = chunkList->prev;
				else
					_tail = chunkList->prev;

				FREE(chunkList);
			}
		}
		
	private:
		inline void Alloc(bool recover) {
			ChunkList * chunkList = (ChunkList *)MALLOC(sizeof(ChunkList) + _chunkCount * (_size + sizeof(Chunk)));
			chunkList->recover = recover ? 1 : 0;
			chunkList->count = 0;
			chunkList->prev = _tail;
			chunkList->next = nullptr;
			chunkList->free = nullptr;
			_tail = chunkList;
			if (_head == nullptr)
				_head = chunkList;

			char * p = (char *)chunkList + sizeof(ChunkList);
			for (s32 i = 0; i < _chunkCount; ++i) {
				Chunk * chunk = (Chunk *)p;
				chunk->parent = chunkList;
				chunk->next = chunkList->free;
				chunk->state = IS_FREE;

				chunkList->free = chunk;

				p += _size + sizeof(Chunk);
			}
			_freeCount += _chunkCount;
		}

	private:
		s32 _size;
		s32 _chunkCount;
		s32 _freeCount;
		ChunkList * _head;
		ChunkList * _tail;
	};
}

#endif //__OPOOL_H__
