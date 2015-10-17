#ifndef __REDISSESSION_H__
#define __REDISSESSION_H__
#include "util.h"
#include "IKernel.h"
using namespace core;
#include "Define.h"
#include <list>

class RedisSession : public ISession, public ITimer {
public:
	RedisSession() { _connected = false; }
	virtual ~RedisSession() {}

	inline void SetConnect(const s32 id, const char * ip, const s32 port) {
		_id = id;
		SafeSprintf(_ip, sizeof(_ip), ip);
		_port = port;
	}
	inline const char * GetConnectIp() const { return _ip; }
	inline s32 GetConnectPort() const { return _port; }

	virtual void OnConnected(IKernel * kernel);
	virtual s32 OnRecv(IKernel * kernel, const void * context, const s32 size);
	virtual void OnError(IKernel * kernel, const s32 error) {}
	virtual void OnDisconnected(IKernel * kernel);
	virtual void OnConnectFailed(IKernel * kernel);

	virtual void OnStart(IKernel * kernel, s64 tick) {}
	virtual void OnTimer(IKernel * kernel, s64 tick);
	virtual void OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {}

	virtual void OnPause(IKernel * kernel, s64 tick) {}
	virtual void OnResume(IKernel * kernel, s64 tick) {}

	void AddSequence(const s64 id) { _sequences.push_back(id); }
	bool IsConnected() { return _connected; }

private:
	s32 ReadLine(const char * context, const char * sep, const s32 seqLen, const s32 size, s32 * number = nullptr);
	s32 Parse36(const char * context, const s32 size);
	s32 Parse43(const char * context, const s32 size);
	s32 Parse45(const char * context, const s32 size);
	s32 Parse58(const char * context, const s32 size);
	s32 Parse42(const char * context, const s32 size);
	s32 Parse(const char * context, const s32 size);

private:
	s32 _id;
	char _ip[MAX_IP_SIZE];
	s32 _port;

	bool _connected;
	std::list<s64> _sequences;
};

#endif //__REDISSESSION_H__

