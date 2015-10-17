#include "RedisSession.h"
#include "Redis.h"

void RedisSession::OnConnected(IKernel * kernel) {
	_connected = true;
}

s32 RedisSession::OnRecv(IKernel * kernel, const void * context, const s32 size) {
	OASSERT(!_sequences.empty(), "where is sequence id");
	
	s32 res = Parse((const char*)context, size);
	if (res > 0) {
		s64 sequenceId = *_sequences.begin();
		_sequences.pop_front();

		Redis::OnRecv(kernel, sequenceId, context, size);
	}
	return res;
}

void RedisSession::OnDisconnected(IKernel * kernel) {
	_connected = false;
	Redis::GetKernel()->StartTimer(this, 0, 1, Redis::GetReconnectInterval());

	for (auto sequenceId : _sequences)
		Redis::OnFailed(kernel, sequenceId);
	_sequences.clear();
}

void RedisSession::OnConnectFailed(IKernel * kernel) {
	Redis::GetKernel()->StartTimer(this, 0, 1, Redis::GetReconnectInterval());
}

void RedisSession::OnTimer(IKernel * kernel, s64 tick) {
	Redis::Reconnect(this);
}

s32 RedisSession::ReadLine(const char * context, const char * sep, const s32 seqLen, const s32 size, s32 * number) {
	s32 pos = 0;
	while (pos + seqLen <= size) {
		if (memcmp(context + pos, sep, seqLen) == 0) {
			if (number != nullptr) {
				char digit[pos + 1];
				memcpy(digit, context, pos);
				digit[pos] = 0;
				*number = tools::StringAsInt(digit);
			}
			return pos + seqLen;
		}
		++pos;
	}
	return 0;
}

s32 RedisSession::Parse36(const char * context, const s32 size) { // $
	s32 number;
	s32 used = ReadLine(context + 1, "\r\n", 2, size - 1, &number);
	if (used == 0)
		return 0;

	if (used + 1 + number + 2 <= size)
		return used + 1 + number + 2;
	return 0;
}

s32 RedisSession::Parse43(const char * context, const s32 size) { // +
	s32 used = ReadLine(context + 1, "\r\n", 2, size - 1);
	if (used == 0)
		return 0;

	return used + 1;
}

s32 RedisSession::Parse45(const char * context, const s32 size) { // -
	s32 used = ReadLine(context + 1, "\r\n", 2, size - 1);
	if (used == 0)
		return 0;

	return used + 1;
}

s32 RedisSession::Parse58(const char * context, const s32 size) { // :
	s32 used = ReadLine(context + 1, "\r\n", 2, size - 1);
	if (used == 0)
		return 0;

	return used + 1;
}

s32 RedisSession::Parse42(const char * context, const s32 size) { // *
	s32 count = 0;
	s32 used = ReadLine(context + 1, "\r\n", 2, size - 1, &count);
	if (used == 0)
		return 0;

	++used;
	for (s32 i = 0; i < count; ++i) {
		s32 ret = Parse(context + used, size - used);
		if (ret == 0)
			return 0;

		used += ret;
	}

	if (used <= size)
		return used;
	return 0;
}

s32 RedisSession::Parse(const char * context, const s32 size) {
	switch (*context) {
	case '$': return Parse36(context, size);
	case '+': return Parse43(context, size);
	case '-': return Parse45(context, size);
	case ':': return Parse58(context, size);
	case '*': return Parse42(context, size);
	default: OASSERT(false, "not support"); break;
	}
	return ON_RECV_FAILED;
}

