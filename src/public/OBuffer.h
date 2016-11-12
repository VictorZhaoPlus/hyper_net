#ifndef __OBUFFER_H_
#define __OBUFFER_H_
#include "util.h"

class OBuffer {
	typedef const char * StringType;
	typedef const void * VoidType;
public:
	OBuffer(const char * buf, const s32 size) : _buffer(buf), _size(size), _offset(0) {}
	~OBuffer() {}

	template<typename T>
	bool Read(T& val) const {
		const void * data = GetData(sizeof(T));
		if (data != nullptr) {
			val = *(T*)data;
			return true;
		}
		return false;
	}

	bool ReadString(StringType& val) const {
		s32 size;
		if (!Read(size))
			return false;
		const void * data = GetData(size + 1);
		if (data != nullptr) {
			val = (const char*)data;
			return true;
		}
		return false;
	}

	bool ReadBlob(VoidType& val, s32& size) {
		if (!Read(size))
			return false;
		const void * data = GetData(size);
		if (data != nullptr) {
			val = data;
			return true;
		}
		return false;
	}

	inline void Reset() { _offset = 0; }

	inline const void * GetContext() const { return _buffer; }
	inline const s32 GetSize() const { return _size; }

private:
	const void * GetData(const s32 size) const {
		OASSERT(_offset + size <= _size, "buffer over flow");
		if (_offset + size <= _size) {
			const void * p = _buffer + _offset;
			_offset += size;
			return p;
		}
		return nullptr;
	}

private:
    const char * _buffer;
    s32 _size;
	mutable s32 _offset;
};

#endif //__OBUFFER_H_
