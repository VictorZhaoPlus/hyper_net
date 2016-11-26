#ifndef __MD5_H__
#define __MD5_H__
#include "util.h"
#include <string>
class MD5 {
public:
	MD5();
	MD5(const void* input, size_t length);
	MD5(const std::string& str);
	void update(const void* input, size_t length);
	void update(const std::string& str);
	const u8* digest();
	std::string toString();
	void reset();

private:
	void update(const u8* input, size_t length);
	void final();
	void transform(const u8 block[64]);
	void encode(const u32* input, u8* output, size_t length);
	void decode(const u8* input, u32* output, size_t length);
	std::string bytesToHexString(const u8* input, size_t length);

	/* class uncopyable */
	MD5(const MD5&);
	MD5& operator=(const MD5&);

private:
	u32 _state[4];	/* state (ABCD) */
	u32 _count[2];	/* number of bits, modulo 2^64 (low-order word first) */
	u8 _buffer[64];	/* input buffer */
	u8 _digest[16];	/* message digest */
	bool _finished;		/* calculate finished ? */

	static const u8 PADDING[64];	/* padding for calculate */
	static const s8 HEX[16];
	enum { BUFFER_SIZE = 1024 };
};

#endif /*MD5_H*/

