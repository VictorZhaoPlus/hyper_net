#ifndef SINGLETON_H_INCLUDED
#define SINGLETON_H_INCLUDED
#include "util.h"

template <typename T>
class OSingleton {
public:
    OSingleton() {}
    virtual ~OSingleton() {}

    inline static T * Instance() {
        static T * instance = nullptr;

        if (instance == nullptr) {
            instance = NEW T;
            if (!instance->Ready()) {
                delete instance;
                instance = nullptr;
            }
        }

        return instance;
    }
};

template <typename T>
class OHolder {
public:
	OHolder() { s_instance = (T*)this; }
	virtual ~OHolder() {}

	inline static T * Instance() { return s_instance; }

private:
	static T * s_instance;
};

template <typename T>
T * OHolder<T>::s_instance = nullptr;

#endif // SINGLETON_H_INCLUDED
