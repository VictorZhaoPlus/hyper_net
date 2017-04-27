#ifndef __BUFFER_H__
#define __BUFFER_H__
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

s32 luaopen_buffer(lua_State *L);

#endif /* __BUFFER_H__ */
