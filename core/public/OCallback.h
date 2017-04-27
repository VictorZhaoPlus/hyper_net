#ifndef __OCALLBACK_H__
#define __OCALLBACK_H__
#include "util.h"
#include <list>
#include <unordered_map>

namespace olib {
	template <typename T, const s32 infoSize = 256>
	struct CallInfo {
		T t;
		char info[infoSize];

		CallInfo(const T& val, const char * i) {
			t = val;
			SafeSprintf(info, sizeof(info), "%s", i);
		}

		CallInfo& operator=(const CallInfo& rhs) {
			t = rhs.t;
			SafeSprintf(info, sizeof(info), "%s", rhs.info);
			return *this;
		}

		bool operator==(const CallInfo& rhs) const {
			return strcmp(info, rhs.info) == 0;
		}
	};

	template <typename T, typename R, typename... Args>
	class Callback {
		typedef std::function<R(Args...)> FuncType;
		typedef std::list<CallInfo<FuncType>> FuncList;
	public:
		Callback() {}
		~Callback() {}

		void Register(const T id, const FuncType& f, const char * info) {
			auto itr = _funces.find(id);
			if (itr == _funces.end()) {
				_funces[id];
				itr = _funces.find(id);
			}
#ifdef _DEBUG
			auto funcItr = std::find(itr->second.begin(), itr->second.end(), CallInfo<FuncType>(f, info));
			OASSERT(funcItr == itr->second.end(), "function is exist");
#endif
			itr->second.push_back(CallInfo<FuncType>(f, info));
		}

		void Unregister(const T id, const FuncType& f, const char * info) {
			auto itr = _funces.find(id);
			if (itr != _funces.end()) {
				std::remove(itr->second.begin(), itr->second.end(), CallInfo<FuncType>(f, info));
			}
		}

		inline bool Unregister(const T id) { _funces.erase(id); }
		inline void Clear() { _funces.clear(); }

		void Call(const T id, Args... a) {
			auto itr = _funces.find(id);
			if (itr != _funces.end()) {
				for (const auto& cb : itr->second) {
					cb.t(a...);
				}
			}
		}

		R Call(const T id, const R& r, Args... a) {
			auto itr = _funces.find(id);
			if (itr != _funces.end()) {
				for (const auto& cb : itr->second) {
					if (r == cb.t(a...))
						return r;
				}
			}
			return r;
		}

	private:
		std::unordered_map<T, FuncList> _funces;
	};

	template <typename T, typename... Args>
	class Callback<T, void, Args...> {
		typedef std::function<void(Args...)> FuncType;
		typedef std::list<CallInfo<FuncType>> FuncList;
	public:
		Callback() {}
		~Callback() {}

		void Register(const T id, const FuncType& f, const char * info) {
			auto itr = _funces.find(id);
			if (itr == _funces.end()) {
				_funces.insert(std::make_pair(id, FuncList()));
				itr = _funces.find(id);
			}
#ifdef _DEBUG
			auto funcItr = std::find(itr->second.begin(), itr->second.end(), CallInfo<FuncType>(f, info));
			OASSERT(funcItr == itr->second.end(), "function is exist");
#endif
			itr->second.push_back(CallInfo<FuncType>(f, info));
		}

		void Unregister(const T id, const FuncType& f, const char * info) {
			auto itr = _funces.find(id);
			if (itr != _funces.end()) {
				std::remove(itr->second.begin(), itr->second.end(), CallInfo<FuncType>(f, info));
			}
		}

		inline bool Unregister(const T id) { _funces.erase(id); }
		inline void Clear() { _funces.clear(); }

		void Call(const T id, Args... a) {
			auto itr = _funces.find(id);
			if (itr != _funces.end()) {
				for (const auto& cb : itr->second) {
					cb.t(a...);
				}
			}
		}

	private:
		std::unordered_map<T, FuncList> _funces;
	};

	template <typename T, typename C>
	struct CallbackType {};

	template <typename T, typename R, typename... Args>
	struct CallbackType<T, std::function<R(Args...)>> {
		typedef Callback<T, R, Args...> type;
	};
}
#endif //__OCALLBACK_H__
