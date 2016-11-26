#ifndef __OSTRING_H__
#define __OSTRING_H__
#include "util.h"

namespace olib {
	template <s16 size = 16>
	class OString {
	public:
		OString() { SafeMemset(_buff, sizeof(_buff), 0, sizeof(_buff)); }
		OString(const char * str) {
			SafeMemset(_buff, sizeof(_buff), 0, sizeof(_buff));

			s32 len = (s32)strlen(str);
			len = (len > size) ? size : len;
			SafeMemcpy(_buff, sizeof(_buff) - 1, str, len);
		}

		inline const char * GetString() const { return _buff; }
		inline void Clear() { SafeMemset(_buff, sizeof(_buff), 0, sizeof(_buff)); }

		OString & operator = (const char * str) {
			SafeMemset(_buff, sizeof(_buff), 0, sizeof(_buff));

			s32 len = (s32)strlen(str);
			len = (len > size) ? size : len;
			SafeMemcpy(_buff, sizeof(_buff) - 1, str, len);
			return *this;
		}

		bool operator == (const char * str) const { return !strcmp(_buff, str); }
		bool operator != (const char * str) const { return strcmp(_buff, str); }
		bool operator == (const OString & str) const { return !strcmp(_buff, str._buff); }
		bool operator != (const OString & str) const { return strcmp(_buff, str._buff); }
		bool operator < (const OString & str) const { return strcmp(_buff, str._buff) < 0; }

		inline s32 Length() const { return (s32)strlen(_buff); }

		s64 Hash() const {
			s64 hash = 0;
			s32 count = (s32)strlen(_buff);
			for (s32 i = 0; i < count; ++i) {
				hash = hash * 33 + _buff[i];
			}
			return hash;
		}

	private:
		char _buff[size + 1];
	};

	template <s16 size = 16>
	struct OStringHash {
		size_t operator()(const OString<size>& src) const {
			return (size_t)src.Hash();
		}
	};
}
#endif //__OSTRING_H__

