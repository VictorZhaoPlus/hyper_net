#ifndef __SAVE_H__
#define __SAVE_H__
#include "util.h"
#include "ISave.h"
#include "singleton.h"

class IProp;
class Save : public ISave, public OHolder<Save> {
	class ObjectSavor {
		struct PartSavor {
			std::function<void(IKernel * kernel, IObject * object, IBuffer& buf)> f;
			std::string _debug;
		};
	public:
		ObjectSavor(s32 delay) : _delay(delay) {}
		~ObjectSavor() {}

		void AddPartSavor(const std::function<void(IKernel * kernel, IObject * object, IBuffer& buf)>& f, const char * debug) {
			_partSavors.push_back({ f, debug });
		}

		void OnCreate(IKernel * kernel, IObject * object);
		void OnRecover(IKernel * kernel, IObject * object);

		void Save(IKernel * kernel, IObject * object, const char * name, const IProp * prop, const bool sync);
		void OnSave(IKernel * kernel, IObject * object);

	private:
		std::list<PartSavor> _partSavors;
		s32 _delay;
	};

	class SaveTime : public ITimer {
	public:
		SaveTime(IObject * object, ObjectSavor * savor) : _object(object), _savor(savor) {}
		virtual ~SaveTime() {}

		virtual void OnStart(IKernel * kernel, s64 tick) {}
		virtual void OnTimer(IKernel * kernel, s32 beatCount, s64 tick);
		virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick);

	private:
		IObject * _object;
		ObjectSavor * _savor;
	};

public:
	virtual bool Initialize(IKernel * kernel);
	virtual bool Launched(IKernel * kernel);
	virtual bool Destroy(IKernel * kernel);

	virtual void RegObjectPartSavor(const char * type, const std::function<void(IKernel * kernel, IObject * object, IBuffer& buf)>& f, const char * debug) {
		auto itr = _savors.find(type);
		OASSERT(itr != _savors.find(type), "wtf");
		if (itr != _savors.end()) {
			itr->second->AddPartSavor(f, debug);
		}
	}

private:
    IKernel * _kernel;

	std::unordered_map<std::string, ObjectSavor*> _savors;
};

#endif //__SAVE_H__

