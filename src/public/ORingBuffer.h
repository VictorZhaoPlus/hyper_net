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

	char * RingBufferWrite(struct RingBuffer * buf, u32 * size);
	void RingBufferIn(struct RingBuffer * buf, const u32 size);
	s8 RingBufferWriteBlock(struct RingBuffer * buf, const void * content, const u32 size);

	char * RingBufferReadTemp(struct RingBuffer * buf, char * temp, u32 tempSize, u32 * size);
	char * RingBufferRead(struct RingBuffer * buf, u32 * size);
	void RingBufferOut(struct RingBuffer * buf, const u32 size);

	struct RingBuffer * RingBufferAlloc(u32 size);
	void RingBufferDestroy(struct RingBuffer * buf);

#ifdef __cplusplus
};
#endif

#endif //__ORINGBUFFER_h__
