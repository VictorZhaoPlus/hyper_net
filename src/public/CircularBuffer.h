#ifndef __CircularBuffer_h__
#define __CircularBuffer_h__
#include "util.h"

namespace olib {
	class CircularBuffer {
	public:
		CircularBuffer(s32 size) : _writeIndex(0), _readIndex(0) {
			OASSERT(size != 0, "size must not zero");
			_size = size + 1;
			_buff = (char*)MALLOC(_size);
		}

		~CircularBuffer() { FREE(_buff); }

		bool Write(const void * buff, const s32 size) {
			if (_size - GetLength() <= size)
				return false;

			if (_writeIndex >= _readIndex) {
				if (_size - _writeIndex >= size) 
					memcpy(_buff + _writeIndex, buff, size);
				else {
					memcpy(_buff + _writeIndex, buff, _size - _writeIndex);
					memcpy(_buff, (char *)buff + _size - _writeIndex, size - _size + _writeIndex);
				}
			}
			else 
				memcpy(_buff + _writeIndex, buff, size);

			_writeIndex = (_writeIndex + size) % _size;

			return true;
		}

		const void * Read(void * temp, s32 & size) {
			if (_writeIndex == _readIndex) {
				size = 0;
				return NULL;
			}

			size = GetLength();
			if (_writeIndex > _readIndex)
				return _buff + _readIndex;

			if (_writeIndex < _readIndex) {
				memcpy(temp, _buff + _readIndex, _size - _readIndex);
				memcpy((char *)temp + _size - _readIndex, _buff, _writeIndex);
				return temp;
			}

			return nullptr;
		}

		inline void Out(s32 size) { _readIndex = (_readIndex + size) % _size; }

		inline s32 GetLength() const {
			if (_writeIndex >= _readIndex) 
				return _writeIndex - _readIndex;
			return _size - _readIndex + _writeIndex;
		}

		inline s32 MaxSize() const { return _size; }

	private:
		char * _buff;
		s32 _size;
		s32 _writeIndex;
		s32 _readIndex;
	};
}

#endif //__CircularBuffer_h__
