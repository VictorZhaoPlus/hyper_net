#ifndef __OARGS_H__
#define __OARGS_H__
#include "util.h"

enum ArgType {
	TYPE_INT8 = 0,
	TYPE_INT16,
	TYPE_INT32,
	TYPE_INT64,
	TYPE_FLOAT,
	TYPE_STRING,
	TYPE_BOOL,
	TYPE_STUCT,

	TYPE_COUNT,
};

#pragma pack(push, 1)
struct ArgInfo {
	s8 type;
	s16 offset;
};

template <s32 maxCount, s32 maxSize>
struct Header {
	s8 reserve;
	ArgInfo args[maxCount];
	s8 countArgs;
	s16 offset;
	char data[maxSize];
};
#pragma pack(pop)


class OArgs {
public:
	OArgs(const void *context, s32 size) : _context(context), _size(size) {
		_count = (s8*)_context;
		if (*_count == 0) {
			OASSERT(_size == sizeof(s8) + sizeof(s8) + sizeof(s16), "invalid args buffer");
		}
		else {
			OASSERT(_size > sizeof(s8) + sizeof(ArgInfo) * (*_count) + sizeof(s8) + sizeof(s16), "invalid args buffer");
		}

		_args = (ArgInfo *)((const char*)_context + sizeof(s8));
		_data = (char *)((const char*)_context + sizeof(s8) + sizeof(ArgInfo) * (*_count) + sizeof(s8) + sizeof(s16));
		_len = size - (sizeof(s8) + sizeof(ArgInfo) * (*_count) + sizeof(s8) + sizeof(s16));
	}

	s8 GetType(s8 index) const {
		const ArgInfo& arg = GetArgInfo(index);
		return arg.type;
	}

	bool GetDataBool(s8 index) const {
		const ArgInfo& arg = GetArgInfo(index);
		OASSERT(arg.type == TYPE_BOOL, "is not a s8 type");

		return *(bool*)(_data + arg.offset);
	}

	s8 GetDataInt8(s8 index) const {
		const ArgInfo& arg = GetArgInfo(index);
		OASSERT(arg.type == TYPE_INT8, "is not a s8 type");

		return *(s8*)(_data + arg.offset);
	}

	s16 GetDataInt16(s8 index) const {
		const ArgInfo& arg = GetArgInfo(index);
		OASSERT(arg.type == TYPE_INT16, "is not a s16 type");

		return *(s16*)(_data + arg.offset);
	}

	s32 GetDataInt32(s8 index) const {
		const ArgInfo& arg = GetArgInfo(index);
		OASSERT(arg.type == TYPE_INT32, "is not a s32 type");

		return *(s32*)(_data + arg.offset);
	}

	s64 GetDataInt64(s8 index) const {
		const ArgInfo& arg = GetArgInfo(index);
		OASSERT(arg.type == TYPE_INT64, "is not a s64 type");

		return *(s64*)(_data + arg.offset);
	}

	float GetDataFloat(s8 index) const {
		const ArgInfo& arg = GetArgInfo(index);
		OASSERT(arg.type == TYPE_FLOAT, "is not a float type");

		return *(float*)(_data + arg.offset);
	}

	const char * GetDataString(s8 index) const {
		const ArgInfo& arg = GetArgInfo(index);
		OASSERT(arg.type == TYPE_STRING, "is not a string type");

		return _data + arg.offset;
	}

	const void * GetContext() const { return _context; }
	const s32 GetSize() const { return _size; }

	const s8 Count() const { return *_count; }

protected:
	const ArgInfo& GetArgInfo(s8 index) const {
		OASSERT(index < *_count, "out of range");

		return _args[*_count - 1 - index];
	}

private:
	const void * _context;
	const s32 _size;

	s8 * _count;
	ArgInfo * _args;
	const char * _data;
	s32 _len;
};


template <s32 maxCount, s32 maxSize>
class IArgs {
public:
	IArgs() : _fixed(false), _context(nullptr), _size(0){
		_header.countArgs = 0;
		_header.offset = 0;
	}

	IArgs& operator<< (const bool value) { return Write(TYPE_BOOL, &value, sizeof(bool)); }
	IArgs& operator<< (const s8 value) { return Write(TYPE_INT8, &value, sizeof(s8)); }
	IArgs& operator<< (const s16 value) { return Write(TYPE_INT16, &value, sizeof(s16)); }
	IArgs& operator<< (const s32 value) { return Write(TYPE_INT32, &value, sizeof(s32)); }
	IArgs& operator<< (const s64 value) { return Write(TYPE_INT64, &value, sizeof(s64)); }
	IArgs& operator<< (const float value) { return Write(TYPE_FLOAT, &value, sizeof(float)); }

	IArgs& operator<< (const char* value) {
		s32 size = strlen(value);
		OASSERT(!_fixed && _header.countArgs < maxCount && _header.offset + size + 1 <= maxSize, "args over flow");
		if (_fixed || _header.countArgs >= maxCount || _header.offset + size + 1 > maxSize)
			return *this;

		ArgInfo& info = _header.args[maxCount - 1 - _header.countArgs];
		info.type = TYPE_STRING;
		info.offset = _header.offset;
		SafeSprintf(&(_header.data[_header.offset]), size + 1, "%s", value);
		_header.offset += size + 1;
		++_header.countArgs;

		return *this;
	}

	inline void Fix() {
		OASSERT(!_fixed, "can't fix twice");
		_fixed = true;

		s8& reserve = *(s8*)((char*)(&_header.args[maxCount - _header.countArgs]) - sizeof(s8));
		reserve = _header.countArgs;

		_size = sizeof(s8) + sizeof(ArgInfo) * _header.countArgs + sizeof(s8) + sizeof(s16) + sizeof(s8) * _header.offset;
		_context = &reserve;
	}

	OArgs Out() {
		OASSERT(_fixed, "must fix first");
		if (!_fixed)
			Fix();

		return OArgs(_context, _size);
	}

	const char * GetContext() { return _context; }

	s32 GetLen() { return _size; }

private:
	IArgs & Write(s32 type, const void * buf, s32 size) {
		OASSERT(!_fixed && _header.countArgs < maxCount && _header.offset + size <= maxSize, "args over flow");
		if (_fixed || _header.countArgs >= maxCount || _header.offset + size > maxSize)
			return *this;

		ArgInfo& info = _header.args[maxCount - 1 - _header.countArgs];
		info.type = type;
		info.offset = _header.offset;
		SafeMemcpy(&(_header.data[_header.offset]), maxSize - _header.offset, buf, size);
		_header.offset += size;
		++_header.countArgs;

		return *this;
	}

private:
	Header<maxCount, maxSize> _header;

	bool _fixed;
	const void * _context;
	s32 _size;
};

#endif //__OARGS_H__
