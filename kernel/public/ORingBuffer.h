#ifndef __ORINGBUFFER_h__
#define __ORINGBUFFER_h__
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif
	struct RingBuffer {
		char * buffer;
		u32 size;
		u32 in;
		u32 out;
	};

	KERNEL_API u32 RingBufferCalcSize(u32 size);

	KERNEL_API char * RingBufferWrite(struct RingBuffer * buf, u32 * size);
	KERNEL_API void RingBufferIn(struct RingBuffer * buf, const u32 size);
	KERNEL_API s8 RingBufferWriteBlock(struct RingBuffer * buf, const void * content, const u32 size);

	KERNEL_API char * RingBufferReadTemp(struct RingBuffer * buf, char * temp, u32 tempSize, u32 * size);
	KERNEL_API char * RingBufferRead(struct RingBuffer * buf, u32 * size);
	KERNEL_API void RingBufferOut(struct RingBuffer * buf, const u32 size);

	inline u32 RingBufferLength(struct RingBuffer * buf) { return buf->in - buf->out; }

	KERNEL_API struct RingBuffer * RingBufferAlloc(u32 size);
	KERNEL_API void RingBufferRealloc(struct RingBuffer * buf, u32 size);
	KERNEL_API void RingBufferDestroy(struct RingBuffer * buf);

#ifdef __cplusplus
};
#endif

#endif //__ORINGBUFFER_h__
