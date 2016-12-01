#ifndef __PROFILE_H__
#define __PROFILE_H__
#include "util.h"
#include "singleton.h"

class IModule;
class Profile : public OSingleton<Profile> {
    friend class OSingleton<Profile>;
public:
    bool Ready();
    bool Initialize();
    void Loop();
    void Destroy();

private:
	Profile() {}
    ~Profile() {}

	s64 _tick;
	s64 _interval;
	bool _open;
};

#endif //__PROFILE_H__
