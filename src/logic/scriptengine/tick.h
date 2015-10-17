#ifndef __TICK_H__
#define __TICK_H__
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

s32 luaopen_tick(lua_State *L);

#endif /* __TICK_H__ */
