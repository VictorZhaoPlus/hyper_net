#ifndef __CONFIGMGR_H__
#define __CONFIGMGR_H__
#include "util.h"
#include "singleton.h"
#include <string>
#include <unordered_map>

class ConfigMgr : public OSingleton<ConfigMgr> {
    friend class OSingleton<ConfigMgr>;
public:
    bool Ready();
    bool Initialize(int argc, char ** argv);
    void Destroy();

    const char * GetCmdArg(const char * name);

	inline s32 GetMaxOpenFile() const { return _maxOpenFile; }
	inline s32 GetStackSize() const { return _stackSize; }

    inline s32 GetFrameDuration() const { return _frameDuration; }

    inline s32 GetNetFrameTick() const { return _netFrameTick; }
    inline s32 GetNetFrameWaitTick() const { return _netFrameWaitTick; }
    inline s32 GetNetSupportSize() const { return _netSupportSize; }
	inline s32 GetNetThreadCount() const { return _netThreadCount; }
	inline s32 GetNetMaxPacketSize() const { return _netMaxPacketSize; }

	inline s32 GetAsyncThreadCount() const { return _asyncThreadCount; }
	inline s32 GetAsyncTick() const { return _asyncTick; }

	inline const char * GetLoggerPath() const { return _loggerPath; }
	inline bool IsLogToConsole() const { return _logToConsole; }

private:
    bool parse(int argc, char ** argv);

private:
    ConfigMgr();
    virtual ~ConfigMgr();

    std::unordered_map<std::string, std::string> _args;

	s32 _maxOpenFile;
	s32 _stackSize;

    s32 _frameDuration;

    s32 _netFrameTick;
    s32 _netFrameWaitTick;
    s32 _netSupportSize;
	s32 _netThreadCount;
	s32 _netMaxPacketSize;

	s32 _asyncThreadCount;
	s32 _asyncTick;

	char _loggerPath[MAX_PATH];
	bool _logToConsole;
};

#endif // __CONFIGMGR_H__
