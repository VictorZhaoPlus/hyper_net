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

	bool ReadBlob(VoidType& val, s32& size) const {
		if (!Read(size))
			return false;
		const void * data = GetData(size);
		if (data != nullptr) {
			val = data;
			return true;
		}
		return false;
	}

	bool ReadStruct(VoidType& val, const s32 size) const {
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

	OBuffer Left() const { return OBuffer(_buffer + _offset, _size - _offset); }

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

class IBuffer {
public:
	IBuffer() {}
	virtual ~IBuffer() {}

	virtual IBuffer& operator<<(const s8& t) = 0;
	virtual IBuffer& operator<<(const s16& t) = 0;
	virtual IBuffer& operator<<(const s32& t) = 0;
	virtual IBuffer& operator<<(const s64& t) = 0;
	virtual IBuffer& operator<<(const float& t) = 0;
	virtual IBuffer& operator<<(const char * t) = 0;
	virtual IBuffer& WriteBuffer(const void * context, const s32 size) = 0;
	virtual IBuffer& WriteStruct(const void * context, const s32 size) = 0;
};

namespace olib {
	template<s32 maxSize>
	class Buffer : public IBuffer {
		template <typename T> struct Trait {};
	public:
		Buffer() {}
		virtual ~Buffer() {}

		virtual IBuffer& operator<<(const s8& t) { return Write(t); }
		virtual IBuffer& operator<<(const s16& t) { return Write(t); }
		virtual IBuffer& operator<<(const s32& t) { return Write(t); }
		virtual IBuffer& operator<<(const s64& t) { return Write(t); }
		virtual IBuffer& operator<<(const float& t) { return Write(t); }

		virtual IBuffer& operator<<(const char * str) {
			s32 size = (s32)strlen(str);
			if (_offset + size + sizeof(s32) + 1 <= maxSize) {
				*this << size;
				SafeMemcpy(_buf + _offset, maxSize - _offset, str, size);
				_offset += size;
				_buf[_offset] = 0;
				++_offset;
			}
			return *this;
		}

		template <typename T>
		IBuffer& Write(const T& t) {
			if (_offset + sizeof(T) <= maxSize) {
				SafeMemcpy(_buf + _offset, maxSize - _offset, &t, sizeof(t));
				_offset += sizeof(T);
			}
			return *this;
		}

		virtual IBuffer& WriteBuffer(const void * context, const s32 size) {
			OASSERT(_offset + size + sizeof(s32) <= maxSize, "wtf");
			if (_offset + size + sizeof(s32) <= maxSize) {
				*this << size;
				SafeMemcpy(_buf + _offset, maxSize - _offset, context, size);
				_offset += size;
			}
			return *this;
		}

		virtual IBuffer& WriteStruct(const void * context, const s32 size) {
			OASSERT(_offset + size <= maxSize, "wtf");
			if (_offset + size <= maxSize) {
				SafeMemcpy(_buf + _offset, maxSize - _offset, context, size);
				_offset += size;
			}
			return *this;
		}

		template <typename T>
		T * Reserve() {
			OASSERT(_offset + sizeof(T) <= maxSize, "wtf");
			if (_offset + sizeof(T) <= maxSize) {
				void * ret = _buf + _offset;
				_offset += sizeof(T);
				return (T*)ret;
			}
			return nullptr;
		}

		OBuffer Out() {
			return OBuffer(_buf, _offset);
		}

	private:
		char _buf[maxSize];
		s32 _offset;
	};
}

#endif //__OBUFFER_H_
