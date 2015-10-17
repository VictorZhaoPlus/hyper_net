#ifndef __OARGS_H__
#define __OARGS_H__
#include "util.h"

#pragma pack(push, 1)
struct ArgInfo {
	s8 type;
	s16 offset;
};

template <s32 maxCount>
struct Header {
	s8 reserve;
	ArgInfo args[maxCount];
	s8 count;
	s16 offset;
	char data[1];
};
#pragma pack(pop)

enum ArgType {
	TYPE_INT8 = 0,
	TYPE_INT16,
	TYPE_INT32,
	TYPE_INT64,
	TYPE_FLOAT,
	TYPE_STRING,
	TYPE_STRUCT,

	TYPE_COUNT,
};

template <s32 maxCount, s32 maxSize>
class IArgs {
public:
	TArgs() : _header(*((Header*)&_buf)), _lock(false), _data(nullptr), _len(0){}

	TArgs& operator<<(s8 value) {
		OASSERT(_header.count + 1 < maxCount && _header.offset + sizeof(s8)+sizeof(_header)-1 <= maxSize, "not enough space for a new s8 type");

		s8 index = (maxCount - 1 - _header.count);

		_header.args[index].type = TYPE_INT8;
		_header.args[index].offset = _header.offset;
		*(s8*)(_header.data + _header.offset) = value;
		_header.offset += sizeof(s8);
		++_header.count;
	}

	TArgs& operator<<(s16 value) {
		OASSERT(_header.count + 1 < maxCount && _header.offset + sizeof(s16)+sizeof(_header)-1 <= maxSize, "not enough space for a new s16 type");

		s8 index = (maxCount - 1 - _header.count);

		_header.args[index].type = TYPE_INT16;
		_header.args[index].offset = _header.offset;
		*(s16*)(_header.data + _header.offset) = value;
		_header.offset += sizeof(s16);
		++_header.count;
	}

	TArgs& operator<<(s32 value) {
		OASSERT(_header.count + 1 < maxCount && _header.offset + sizeof(s32)+sizeof(_header)-1 <= maxSize, "not enough space for a new s32 type");

		s8 index = (maxCount - 1 - _header.count);

		_header.args[index].type = TYPE_INT32;
		_header.args[index].offset = _header.offset;
		*(s32*)(_header.data + _header.offset) = value;
		_header.offset += sizeof(s32);
		++_header.count;
	}

	TArgs& operator<<(s64 value) {
		OASSERT(_header.count + 1 < maxCount && _header.offset + sizeof(s64)+sizeof(_header)-1 <= maxSize, "not enough space for a new s64 type");

		s8 index = (maxCount - 1 - _header.count);

		_header.args[index].type = TYPE_INT64;
		_header.args[index].offset = _header.offset;
		*(s64*)(_header.data + _header.offset) = value;
		_header.offset += sizeof(s64);
		++_header.count;
	}

	TArgs& operator<<(float value) {
		OASSERT(_header.count + 1 < maxCount && _header.offset + sizeof(float)+sizeof(_header)-1 <= maxSize, "not enough space for a new float type");

		s8 index = (maxCount - 1 - _header.count);

		_header.args[index].type = TYPE_FLOAT;
		_header.args[index].offset = _header.offset;
		*(float*)(_header.data + _header.offset) = value;
		_header.offset += sizeof(float);
		++_header.count;
	}

	TArgs& operator<<(char* value) {
		OASSERT(_header.count + 1 < maxCount && _header.offset + sizeof(s8)+sizeof(_header) <= maxSize, "not enough space for a new string type");

		s8 index = (maxCount - 1 - _header.count);

		s8 len = (s8)strlen(value);

		_header.args[index].type = TYPE_STRING;
		_header.args[index].offset = _header.offset;
		tools::SafeMemcpy(_header.data + _header.offset, len + 1, value, len);
		_header.offset += len;
		*(s8*)(_header.data + _header.offset) = 0;
		_header.offset += sizeof(s8);
		++_header.count;
	}

	bool lock() {
		OASSERT(!_lock, "can not lock twice");
		_lock = true;

		s8 index = (maxCount - 1 - _header.count);
		_data = &_header.args[index] - sizeof(char);
		*_data = _header.count + 1;
		
		_len = &_header.data[_header.offset] - _data;
	}

	const char * GetBuff() { return _data; }

	s32 GetLen() { return _len; }

private:
	char _buf[maxSize];
	Header<maxCount>& _header;
	bool _lock;
	char * _data;
	s32 _len;
};

class OArgs {
public:
	OArgs(char * buf, s32 len) : _count(*(s8*)buf), _args((ArgInfo*)(buf + sizeof(s8)){
		_data = buf + sizeof(ArgInfo)* _count + sizeof(s8)+sizeof(8) + sizeof(16);
		_len = len - sizeof(ArgInfo)* _count + sizeof(s8)+sizeof(8) + sizeof(16);
	}

	s8 GetType(s8 index) {
		ArgInfo& arg = GetArgInfo(index);
		return arg.type;
	}

	s8 GetDataInt8(s8 index) {
		ArgInfo& arg = GetArgInfo(index);
		OASSERT(arg.type == TYPE_INT8, "is not a s8 type");

		return *(s8*)(_data + arg.offset);
	}

	s16 GetDataInt16(s8 index) {
		ArgInfo& arg = GetArgInfo(index);
		OASSERT(arg.type == TYPE_INT16, "is not a s16 type");

		return *(s16*)(_data + arg.offset);
	}

	s32 GetDataInt32(s8 index) {
		ArgInfo& arg = GetArgInfo(index);
		OASSERT(arg.type == TYPE_INT32, "is not a s32 type");

		return *(s32*)(_data + arg.offset);
	}

	s64 GetDataInt64(s8 index) {
		ArgInfo& arg = GetArgInfo(index);
		OASSERT(arg.type == TYPE_INT64, "is not a s64 type");

		return *(s64*)(_data + arg.offset);
	}

	float GetDataInt32(s8 index) {
		ArgInfo& arg = GetArgInfo(index);
		OASSERT(arg.type == TYPE_FLOAT, "is not a float type");

		return *(float*)(_data + arg.offset);
	}

	const char * GetDataInt32(s8 index) {
		ArgInfo& arg = GetArgInfo(index);
		OASSERT(arg.type == TYPE_STRING, "is not a string type");

		return _data + arg.offset;
	}

protected:
	ArgInfo& GetArgInfo(s8 index) {
		OASSERT(index < _count, "out of range");
	}

private:
	s8& _count;
	ArgInfo * _args;
	char * _data;
	int _len;
};

#endif //__OARGS_H__
