#include "ORingBuffer.h"
#ifdef __cplusplus
extern "C" {
#endif

	static inline u32 Fls(u32 size) {
		if (0 != size) {
			u32 position = 0;
			for (u32 i = (size >> 1); i != 0; ++position)
				i >>= 1;
			return position + 1;
		}
		else
			return 0;
	}

	static inline u32 RoundupPowOfTwo(u32 size) {
		return 1 << Fls(size - 1);
	}

	char * RingBufferWrite(struct RingBuffer * buf, u32 * size) {
		u32 freeSize = buf->size - buf->in + buf->out;
		if (freeSize == 0)
			return NULL;

		u32 realIn = buf->in & (buf->size - 1);
		u32 realOut = buf->out & (buf->size - 1);
		if (realIn >= realOut)
			*size = buf->size - realIn;
		else
			*size = realOut - realIn;

		return buf->buffer + realIn;
	}

	void RingBufferIn(struct RingBuffer * buf, const u32 size) {
		buf->in += size;
	}

	s8 RingBufferWriteBlock(struct RingBuffer * buf, const void * content, const u32 size) {
		u32 freeSize = buf->size - buf->in + buf->out;
		if (freeSize < size)
			return 0;

		u32 realIn = buf->in & (buf->size - 1);
		u32 realOut = buf->out & (buf->size - 1);
		if (size <= buf->size - realIn) 
			SafeMemcpy(buf->buffer + realIn, buf->size - realIn, content, size);
		else {
			SafeMemcpy(buf->buffer + realIn, buf->size - realIn, content, buf->size - realIn);
			SafeMemcpy(buf->buffer, realOut, (const char*)content + buf->size - realIn, size - (buf->size - realIn));
		}
		buf->in += size;
		return 1;
	}

	char * RingBufferReadTemp(struct RingBuffer * buf, char * temp, u32 tempSize, u32 * size) {
		u32 useSize = buf->in - buf->out;
		if (useSize == 0)
			return NULL;

		u32 realIn = buf->in & (buf->size - 1);
		u32 realOut = buf->out & (buf->size - 1);
		if (realIn > realOut) {
			*size = useSize;
			return buf->buffer + realOut;
		}
		else {
			if (tempSize <= buf->size - realOut) {
				*size = buf->size - realOut;
				return buf->buffer + realOut;
			}

			memcpy(temp, buf->buffer + realOut, buf->size - realOut);

			tempSize -= buf->size - realOut;
			u32 headSize = realIn > tempSize  ? tempSize : realIn;
			memcpy(temp + buf->size - realOut, buf->buffer, headSize);
			*size = buf->size - realOut + headSize;

			return temp;
		}	
	}

	char * RingBufferRead(struct RingBuffer * buf, u32 * size) {
		u32 useSize = buf->in - buf->out;
		if (useSize == 0)
			return NULL;

		u32 realIn = buf->in & (buf->size - 1);
		u32 realOut = buf->out & (buf->size - 1);
		if (realIn > realOut)
			*size = useSize;
		else
			*size = buf->size - realOut;

		return buf->buffer + realOut;
	}

	void RingBufferOut(struct RingBuffer * buf, const u32 size) {
		OASSERT(buf->in - buf->out >= size, "wtf");
		buf->out += size;
	}

	struct RingBuffer * RingBufferAlloc(u32 size) {
		if (size & (size - 1))
			size = RoundupPowOfTwo(size);

		char * buffer = MALLOC(size);
		if (!buffer)
			return NULL;

		struct RingBuffer * ret = (struct RingBuffer *)MALLOC(sizeof(struct RingBuffer));
		if (!ret) {
			FREE(buffer);
			return NULL;
		}

		ret->buffer = buffer;
		ret->in = 0;
		ret->out = 0;
		ret->size = size;
		
		return ret;
	}

	void RingBufferRealloc(struct RingBuffer * buf, u32 size) {

	}

	void RingBufferDestroy(struct RingBuffer * buf) {
		FREE(buf->buffer);
		FREE(buf);
	}

#ifdef __cplusplus
};
#endif

