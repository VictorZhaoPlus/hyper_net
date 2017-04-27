#ifndef __OMAP_H__
#define __OMAP_H__
#include "util.h"
#include <unordered_map>

class OBMap {
	template <typename T> struct Trait {};
public:
	OBMap(const void* context, const s32 size) {
		s32 upSize = *(s32*)context;
		_separate = (const char *)context + sizeof(s32) + upSize;

		ParseArray(upSize);
	}

	void ParseArray(s32 size) {
		s32 offset = 0;
		while (offset < size) {
			OASSERT(offset + sizeof(s32) * 2 <= size, "wtf");
			if (offset + sizeof(s32) * 2 > size)
				break;
			offset += sizeof(s32);
			s32 key = *(s32*)(_separate - offset);

			offset += sizeof(s32);
			_indexes[key] = *(s32*)(_separate - offset);
		}
	}

	inline s8 GetDataInt8(const s32 key) const { return GetDataT(key, Trait<s8>()); }
	inline s16 GetDataInt16(const s32 key) const { return GetDataT(key, Trait<s16>()); }
	inline s32 GetDataInt32(const s32 key) const { return GetDataT(key, Trait<s32>()); }
	inline s64 GetDataInt64(const s32 key) const { return GetDataT(key, Trait<s64>()); }
	inline float GetDataFloat(const s32 key) const { return GetDataT(key, Trait<float>()); }
	inline const char * GetDataString(const s32 key) const {
		const void * p = GetData(key);
		if (p)
			return (const char *)p;
		return "";
	}

private:
	template <typename T>
	T GetDataT(const s32 key, const Trait<T>&) const {
		const void * p = GetData(key);
		if (p)
			return *(T*)p;
		return T();
	}

	const void * GetData(const s32 key) const {
		auto itr = _indexes.find(key);
		if (itr != _indexes.end())
			return _separate + itr->second;
		return nullptr;
	}
	
private:
	const char * _separate;
	std::unordered_map<s32, s32> _indexes;
};

template <s32 upSize, s32 downSize>
class IBMap {
public:
	IBMap() : _fixed(false), _context(nullptr), _upOffset(0), _downOffset(0), _size(0) {
		_separate = _data + sizeof(s32) + upSize;
	}

	inline IBMap& WriteInt8(const s32 key, s8 val) { WriteDataT(key, val); return *this; }
	inline IBMap& WriteInt16(const s32 key, s16 val) { WriteDataT(key, val); return *this; }
	inline IBMap& WriteInt32(const s32 key, s32 val) { WriteDataT(key, val); return *this; }
	inline IBMap& WriteInt64(const s32 key, s64 val) { WriteDataT(key, val); return *this; }
	inline IBMap& WriteFloat(const s32 key, float val) { WriteDataT(key, val); return *this; }
	inline IBMap& WriteString(const s32 key, const char * val) {
		s32 len = (s32)strlen(val);
		if (_downOffSet + len + 1 <= downSize) {
			SafeMemcpy(_separate + _downOffset, downSize - _downOffset, val, len);
			_separate[_downOffset + len] = 0;
			WriteDesc(key, _downOffset);
			_downOffset += len + 1;
		}
		return *this; 
	}

	inline void Fix() {
		OASSERT(!_fixed, "wtf");
		_fixed = true;
		s32& reserve = *(s32*)(_data + upSize + sizeof(s32) - _upOffset - sizeof(s32));
		reserve = _upOffset;

		_size = sizeof(s32) + _upOffset + _downOffset;
		_context = &reserve;
	}

	const void * GetContext() const {
		OASSERT(_fixed, "must fix first");
		return _context;
	}

	const s32 GetSize() const {
		OASSERT(_fixed, "must fix first");
		return _size;
	}

private:
	void WriteDesc(s32 key, s32 offset) {
		OASSERT(!_fixed, "wtf");
		OASSERT(_upOffset + sizeof(s32) * 2 <= upSize, "wtf");
		if (_upOffset + sizeof(s32) * 2 <= upSize) {
			_upOffset += sizeof(s32);
			SafeMemcpy(_separate - _upOffset, upSize - _upOffset, &key, sizeof(s32));

			_upOffset += sizeof(s32);
			SafeMemcpy(_separate - _upOffset, upSize - _upOffset, &offset, sizeof(s32));
		}
	}

	template <typename T>
	void WriteDataT(const s32 key, const T& t) {
		OASSERT(!_fixed, "wtf");
		OASSERT(_downOffset + sizeof(T) <= downSize, "wtf");
		if (_downOffset + sizeof(T) <= downSize) {
			SafeMemcpy(_separate + _downOffset, downSize - _downOffset, &t, sizeof(T));
			WriteDesc(key, _downOffset);
			_downOffset += sizeof(T);
		}
	}

private:
	char _data[upSize + downSize + sizeof(s32)];
	s32 _upOffset;
	s32 _downOffset;
	char * _separate;

	bool _fixed;
	void * _context;
	s32 _size;
};

#endif //__OMAP_H__
