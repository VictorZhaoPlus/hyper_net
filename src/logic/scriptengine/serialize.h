#ifndef LSERI_H_
#define LSERI_H_
#include "util.h"
#ifndef LUAJIT
extern "C" {
	#include "lua/lua.h"
	#include "lua/lualib.h"
	#include "lua/lauxlib.h"
};
#else
#include "luajit/lua.hpp"
#endif

s32 luaopen_seri(lua_State *L);

#endif /* LSERI_H_ */
