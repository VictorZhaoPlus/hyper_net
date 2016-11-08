#ifndef __OSTREAM_H_
#define __OSTREAM_H_
#include "util.h"

namespace olib {
    class OStream {
    public:
		OStream(const s32 size) {
			_size = size;
			_offset = 0;
			_buffer = (char *)MALLOC(_size);
#ifdef _DEBUG
			_oringalSize = _size;
#endif
		}

		~OStream() {
			if (_buffer)
				FREE(_buffer);
		}

		inline void Expand() {
			_buffer = (char *)REALLOC(_buffer, _size * 2);
			_size *= 2;
		}

		inline void Shrink() {
			OASSERT(_size / 2 > _oringalSize, "wtf");
			_buffer = (char *)REALLOC(_buffer, _size / 2);
			_size /= 2;
		}

		inline void In(const s32 size) {
			OASSERT(_offset + size <= _size, "wtf");
			_offset += size; 
		}

		inline void Out(const s32 size) {
			OASSERT(size <= _offset, "wtf");
			if (_offset > size)
				memmove(_buffer, _buffer + size, _offset - size);
			_offset -= size;
		}

        inline char * GetBuff() { return _buffer; }
		inline char * GetFree() { return _buffer + _offset; }

		inline s32 GetCapacity() { return _size; }
		inline s32 GetSize() const { return _offset; }
		inline s32 GetFreeSize() { return _size - _offset; }

    private:
        char * _buffer;
        s32 _size;
        s32 _offset;

#ifdef _DEBUG
		s32 _oringalSize;
#endif
    };
}

#endif //__OSTREAM_H_
