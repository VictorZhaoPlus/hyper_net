#include "buffer.h"
#include "tools.h"
#include "ScriptEngine.h"

static s32 l_readline(lua_State * L) {
	const char * context = (const char *)lua_touserdata(L, 1);
	s32 offset = (s32)lua_tointeger(L, 2);
	s32 size = (s32)lua_tointeger(L, 3);
	const char * sep = "\n";
	size_t sepLen = strlen(sep);
	if (lua_gettop(L) == 4)
		sep = lua_tolstring(L, 4, &sepLen);

	s32 pos = offset;
	while (pos + sepLen <= (s32)size) {
		if (memcmp(context + pos, sep, sepLen) == 0) {
			lua_pushinteger(L, pos - offset);
			lua_pushlstring(L, context + offset, pos - offset);
			return 2;
		}
		++pos;
	}
	return 0;
}

static s32 l_readstr(lua_State * L) {
	const char * context = (const char *)lua_touserdata(L, 1);
	s32 offset = (s32)lua_tointeger(L, 2);
	s32 len = (s32)lua_tointeger(L, 3);

	lua_pushlstring(L, context + offset, len);
	return 1;
}

static s32 l_readint8(lua_State * L) {
	const char * context = (const char *)lua_touserdata(L, 1);
	s32 offset = (s32)lua_tointeger(L, 2);

	lua_pushinteger(L, *(s8*)(context + offset));
	return 1;
}

static s32 l_readint16(lua_State * L) {
	const char * context = (const char *)lua_touserdata(L, 1);
	s32 offset = (s32)lua_tointeger(L, 2);

	lua_pushinteger(L, *(s16*)(context + offset));
	return 1;
}

static s32 l_readint32(lua_State * L) {
	const char * context = (const char *)lua_touserdata(L, 1);
	s32 offset = (s32)lua_tointeger(L, 2);

	lua_pushinteger(L, *(s32*)(context + offset));
	return 1;
}

static s32 l_readint64(lua_State * L) {
	const char * context = (const char *)lua_touserdata(L, 1);
	s32 offset = (s32)lua_tointeger(L, 2);

	lua_pushinteger(L, *(s64*)(context + offset));
	return 1;
}

static s32 l_readfloat(lua_State * L) {
	const char * context = (const char *)lua_touserdata(L, 1);
	s32 offset = (s32)lua_tointeger(L, 2);

	lua_pushnumber(L, *(float*)(context + offset));
	return 1;
}

static s32 l_readboolean(lua_State * L) {
	const char * context = (const char *)lua_touserdata(L, 1);
	s32 offset = (s32)lua_tointeger(L, 2);

	lua_pushboolean(L, *(bool*)(context + offset));
	return 1;
}

static const luaL_Reg R[] = {
	{ "readline", l_readline },
	{ "readstr", l_readstr },
	{ "readint8", l_readint8 },
	{ "readint16", l_readint16 },
	{ "readint32", l_readint32 },
	{ "readint64", l_readint64 },
	{ "readfloat", l_readfloat },
	{ "readbool", l_readboolean },
	{NULL,	NULL}
};

s32 luaopen_buffer(lua_State *L) {
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "loaded");
	lua_pushstring(L, "serverd.buffer");

	lua_newtable(L);
	luaL_setfuncs(L, R, 0);

	lua_rawset(L, -3);
	lua_pop(L, 2);
	return 0;
}
