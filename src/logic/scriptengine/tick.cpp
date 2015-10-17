#include "tick.h"
#include "tools.h"
#include "ScriptEngine.h"

static s32 l_tick(lua_State * L) {
	lua_pushinteger(L, tools::GetTimeMillisecond());
	return 1;
}

static s32 l_log(lua_State * L) {
	const char * msg = nullptr;
	bool sync = false;

	s32 count = lua_gettop(L);
	OASSERT(count == 1 || count == 2, "wtf");
	if (count == 1)
		msg = lua_tostring(L, -1);
	else {
		msg = lua_tostring(L, -2);
		sync = lua_toboolean(L, -1);
	}

	ScriptEngine::GetKernel()->Log(msg, sync);
	return 0;
}

static const luaL_Reg R[] = {
	{"tick", l_tick},
	{"log", l_log},
	{NULL,	NULL}
};

s32 luaopen_tick(lua_State *L) {
	lua_getglobal(L, LUA_OSLIBNAME);
	luaL_setfuncs(L, R, 0);
	lua_pop(L, 1);
	return 0;
}
