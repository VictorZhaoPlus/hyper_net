#ifndef __BASE64_H__
#define __BASE64_H__
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

	enum {BASE64_OK = 0, BASE64_INVALID};

	#define BASE64_ENCODE_OUT_SIZE(s)	(((s) + 2) / 3 * 4)
	#define BASE64_DECODE_OUT_SIZE(s)	(((s)) / 4 * 3)

	KERNEL_API s32 Base64Encode(const u8 * in, u32 inlen, char * out, u32 * size);
	KERNEL_API s32 Base64Decode(const char * in, u32 inlen, u8 * out, u32 * size);

#ifdef __cplusplus
};
#endif

#endif /* __BASE64_H__ */

