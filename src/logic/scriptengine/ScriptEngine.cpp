#include "ScriptEngine.h"
#include "tinyxml.h"
#include <string>
#include "serialize.h"
#include "tick.h"
#include "buffer.h"

#define MAX_MODULE_LEN 64
#define MAX_LUA_RESULT 8
#define MAX_CODE_SIZE 256

class LuaReader : public IScriptArgumentReader {
public:
	LuaReader(lua_State * state) : _state(state) {}
	virtual ~LuaReader() {}

	virtual bool Read(const char * format, ...) const  {
		s32 count = lua_gettop(_state);
		s32 index = 1;
		const char * c = format;
		va_list ap;
		va_start(ap, format);
		while (*c != '\0') {
			if (index > count) {
				OASSERT(false, "param is not enough");
				luaL_error(_state, "param is not enough %s", format);
				return false;
			}
			
			switch (*c) {
			case 'c': { s8 * val = va_arg(ap, s8*); *val = (s8)lua_tointeger(_state, index++); } break;
			case 'd': { s16 * val = va_arg(ap, s16*); *val = (s16)lua_tointeger(_state, index++); } break;
			case 'i': { s32 * val = va_arg(ap, s32*); *val = (s32)lua_tointeger(_state, index++); } break;
			case 'l': { s64 * val = va_arg(ap, s64*); *val = lua_tointeger(_state, index++); } break;
			case 'f': { float * val = va_arg(ap, float*); *val = (float)lua_tonumber(_state, index++); } break;
			case 'b': { bool * val = va_arg(ap, bool*); *val = lua_toboolean(_state, index++) != 0; } break;
			case 'P': { void ** val = va_arg(ap, void**); *val = lua_touserdata(_state, index++); } break;
			case 's': { const char ** val = va_arg(ap, const char **); *val = lua_tostring(_state, index++); } break;
			case 'S': {
					const char ** val = va_arg(ap, const char **);
					s32* len = va_arg(ap, s32*);
					size_t size;
					*val = lua_tolstring(_state, index++, &size);
					*len = (s32)size;
				}
				break;
			}
			c++;
		}
		va_end(ap);
		return true;
	}

private:
	lua_State * _state;
};

class LuaWriter : public IScriptResultWriter {
public:
	LuaWriter(lua_State * state) : _state(state), _count(0) {}
	virtual ~LuaWriter() {}

	virtual void Write(const char * format, ...) {
		const char * c = format;
		va_list ap;
		va_start(ap, format);
		while (*c != '\0') {
			switch (*c) {
			case 'c': { s8 val = va_arg(ap, s32); lua_pushinteger(_state, val); ++_count; } break;
			case 'd': { s16 val = va_arg(ap, s32); lua_pushinteger(_state, val); ++_count; } break;
			case 'i': { s32 val = va_arg(ap, s32); lua_pushinteger(_state, val); ++_count; } break;
			case 'l': { s64 val = va_arg(ap, s64); lua_pushinteger(_state, val); ++_count; } break;
			case 'f': { float val = va_arg(ap, float); lua_pushnumber(_state, val); ++_count; } break;
			case 'b': { bool val = va_arg(ap, bool); lua_pushboolean(_state, val ? 1 : 0); ++_count; } break;
			case 'P': { void * val = va_arg(ap, void*); lua_pushlightuserdata(_state, val); ++_count; } break;
			case 's': { const char * val = va_arg(ap, const char *); lua_pushstring(_state, val); ++_count; } break;
			case 'S': { 
					const char * val = va_arg(ap, const char *);
					s32 len = va_arg(ap, s32);
					lua_pushlstring(_state, val, len); 
					++_count;
				} 
				break;
			}
			c++;
		}
		va_end(ap);
	}

	s32 Count() { return _count; }

private:
	lua_State * _state;
	s32 _count;
};

class LuaResult : public IScriptCallResult {
public:
	LuaResult(lua_State * state, s32 count) : _state(state), _count(count) {}
	virtual ~LuaResult() {}

	virtual bool Read(const char * format, ...) const {
		s32 index = 0;
		const char * c = format;
		va_list ap;
		va_start(ap, format);
		while (*c != '\0') {
			if (index >= _count) {
				OASSERT(false, "not so many result");
				luaL_error(_state, "not so many result %s", format);
				return false;
			}

			switch (*c) {
			case 'c': { s8 * val = va_arg(ap, s8*); *val = (s8)lua_tointeger(_state, -MAX_LUA_RESULT + (index++)); } break;
			case 'd': { s16 * val = va_arg(ap, s16*); *val = (s16)lua_tointeger(_state, -MAX_LUA_RESULT + (index++)); } break;
			case 'i': { s32 * val = va_arg(ap, s32*); *val = (s32)lua_tointeger(_state, -MAX_LUA_RESULT + (index++)); } break;
			case 'l': { s64 * val = va_arg(ap, s64*); *val = lua_tointeger(_state, -MAX_LUA_RESULT + (index++)); } break;
			case 'f': { float * val = va_arg(ap, float*); *val = (float)lua_tonumber(_state, -MAX_LUA_RESULT + (index++)); } break;
			case 'b': { bool * val = va_arg(ap, bool*); *val = lua_toboolean(_state, -MAX_LUA_RESULT + (index++)) != 0; } break;
			case 's': { const char ** val = va_arg(ap, const char **); *val = lua_tostring(_state, -MAX_LUA_RESULT + (index++)); } break;
			case 'S': {
					const char ** val = va_arg(ap, const char **);
					s32* len = va_arg(ap, s32*);
					size_t size;
					*val = lua_tolstring(_state, -MAX_LUA_RESULT + (index++), &size);
					*len = (s32)size;
				}
				break;
			}
			c++;
		}
		va_end(ap);
		return true;
	}

private:
	lua_State * _state;
	s32 _count;
};

class ScriptModule : public IScriptModule {
public:
	ScriptModule(const char * name) { SafeSprintf(_name, sizeof(_name), name); }
	virtual ~ScriptModule() {}

	virtual void Release() { DEL this; }

	const char * CStyle() const { return _name; }

private:
	char _name[MAX_MODULE_LEN];
};

bool ScriptEngine::Initialize(IKernel * kernel) {
    _kernel = kernel;

    TiXmlDocument doc;
    std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/server_conf.xml";
    if (!doc.LoadFile(coreConfigPath.c_str())) {
        OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
        return false;
    }

    const TiXmlElement * root = doc.RootElement();
    OASSERT(root != nullptr, "core xml format error");

    const TiXmlElement * p = root->FirstChildElement("lua");
    const char * path = p->Attribute("path");
    const char * name = kernel->GetCmdArg("name");

    if (!LoadLua(path, name)) {
        OASSERT(false, "load lua failed");
        return false;
    }
    return true;
}

bool ScriptEngine::Launched(IKernel * kernel) {
	return ExecuteScriptFile(kernel, "entry");
}

bool ScriptEngine::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

IScriptModule * ScriptEngine::CreateModule(const char * name) {
	char module[256];
	SafeSprintf(module, sizeof(module), "serverd.%s", name);

	lua_getglobal(_state, "package");
	lua_getfield(_state, -1, "loaded");
	lua_pushstring(_state, module);
	lua_newtable(_state);
	lua_rawset(_state, -3);

	lua_pop(_state, 2);

	return NEW ScriptModule(name);
}

bool ScriptEngine::AddModuleFunction(const IScriptModule * m, const char * func, const ScriptFuncType& f) {
	char module[256];
	SafeSprintf(module, sizeof(module), "serverd.%s", ((const ScriptModule*)m)->CStyle());

	lua_getglobal(_state, "package");
	lua_getfield(_state, -1, "loaded");
	lua_pushstring(_state, module);
	lua_rawget(_state, -2);

	OASSERT(lua_istable(_state, -1), "where is module %s", ((const ScriptModule*)m)->CStyle());

	lua_pushstring(_state, func);
	new(lua_newuserdata(_state, sizeof(ScriptFuncType))) ScriptFuncType(f);
	lua_pushcclosure(_state, ScriptEngine::Callback, 1);
	lua_rawset(_state, -3);

	lua_pop(_state, 3);
	return true;
}

bool ScriptEngine::Call(const IScriptModule * module, const char * func, const ScriptResultReadFuncType& f, const char * format, ...) {
	if (!PrepareCall(module, func))
		return false;

	s32 count = 0;
	const char * c = format;
	va_list ap;
	va_start(ap, format);
	while (*c != '\0') {
		switch (*c) {
		case 'c': { s8 val = va_arg(ap, s32); lua_pushinteger(_state, val); ++count; } break;
		case 'd': { s16 val = va_arg(ap, s32); lua_pushinteger(_state, val); ++count; } break;
		case 'i': { s32 val = va_arg(ap, s32); lua_pushinteger(_state, val); ++count; } break;
		case 'l': { s64 val = va_arg(ap, s64); lua_pushinteger(_state, val); ++count; } break;
		case 'f': { float val = va_arg(ap, float); lua_pushnumber(_state, val); ++count; } break;
		case 'b': { bool val = va_arg(ap, bool); lua_pushboolean(_state, val ? 1 : 0); ++count; } break;
		case 'P': { void * val = va_arg(ap, void*); lua_pushlightuserdata(_state, val); ++count; } break;
		case 's': { const char * val = va_arg(ap, const char *); lua_pushstring(_state, val); ++count; } break;
		case 'S': {
				const char * val = va_arg(ap, const char *);
				s32 len = va_arg(ap, s32);
				lua_pushlstring(_state, val, len);
				++count;
			}
			break;
		}
		c++;
	}
	va_end(ap);

	bool ret = ExecuteFunction(_kernel, count, [&f](IKernel * kernel, lua_State * state){
		if (f) {
			s32 count = 0;
			for (s32 i = 0; i < MAX_LUA_RESULT; ++i) {
				if (lua_isnil(state, -MAX_LUA_RESULT + i))
					break;
				++count;
			}

			LuaResult result(state, count);
			f(kernel, &result);
		}
	});

	lua_pop(_state, 3);
	return ret;
}

bool ScriptEngine::LoadLua(const char * path, const char * logic) {
	_state = luaL_newstate();
	OASSERT(_state != nullptr, "init lua state failed");

	luaL_openlibs(_state);
	luaopen_seri(_state);
	luaopen_tick(_state);
	luaopen_buffer(_state);

	char logicPath[MAX_PATH];
	SafeSprintf(logicPath, sizeof(logicPath), "%s/hyper_net", path);
	AddSearchPath(logicPath);

	SafeSprintf(logicPath, sizeof(logicPath), "%s/%s", path, logic);
	AddSearchPath(logicPath);

	return true;
}

void ScriptEngine::AddSearchPath(const char* path) {
    lua_getglobal(_state, "package");                                  /* L: package */
    lua_getfield(_state, -1, "path");                /* get package.path, L: package path */
    const char* cur_path =  lua_tostring(_state, -1);
    lua_pushfstring(_state, "%s;%s/?.lua", cur_path, path);            /* L: package path newpath */
    lua_setfield(_state, -3, "path");          /* package.path = newpath, L: package path */
    lua_pop(_state, 2);                                                /* L: - */
}

s32 ScriptEngine::Callback(lua_State * state) {
	ScriptFuncType func = *(ScriptFuncType*)lua_touserdata(state, lua_upvalueindex(1));
	OASSERT(func, "where is c func");

	LuaReader reader(state);
	LuaWriter writer(state);

	func(ScriptEngine::Instance()->GetKernel(), &reader, &writer);

	return writer.Count();
}

bool ScriptEngine::PrepareCall(const IScriptModule * m, const char * func) {
	char module[256];
	SafeSprintf(module, sizeof(module), "serverd.%s", ((const ScriptModule*)m)->CStyle());

	lua_getglobal(_state, "package");
	lua_getfield(_state, -1, "loaded");
	lua_pushstring(_state, module);
	lua_rawget(_state, -2);
	OASSERT(lua_istable(_state, -1), "where is module %s", ((const ScriptModule*)m)->CStyle());

	lua_pushstring(_state, func);
	lua_rawget(_state, -2);
	if (!lua_isfunction(_state, -1)) {
		OASSERT(lua_isfunction(_state, -1), "is not a function");
		lua_pop(_state, 4);
		return false;
	}

	return true;
}

bool ScriptEngine::ExecuteFunction(IKernel * kernel, int argc, const ResultReader& reader) {
    int functionIndex = -(argc + 1);
    if (!lua_isfunction(_state, functionIndex)) {
        OASSERT(false, "execute lua func but is not a func");
        return false;
    }

    int traceback = 0;
    lua_getglobal(_state, "__G__TRACKBACK__");                         /* L: ... func arg1 arg2 ... G */
    if (!lua_isfunction(_state, -1))
        lua_pop(_state, 1);                                            /* L: ... func arg1 arg2 ... */
    else {
        lua_insert(_state, functionIndex - 1);                         /* L: ... G func arg1 arg2 ... */
        traceback = functionIndex - 1;
    }

    int error = 0;
    error = lua_pcall(_state, argc, MAX_LUA_RESULT, traceback);                  /* L: ... [G] ret */
    if (error) {
		if (traceback == 0) {
			printf("%s\n", lua_tostring(_state , - 1));
			lua_pop(_state, 1); // remove error message from stack
		}
        else                                                            /* L: ... G error */
            lua_pop(_state, 2); // remove __G__TRACKBACK__ and error message from stack
        return false;
    }

    if (reader)
        reader(kernel, _state);
	lua_pop(_state, MAX_LUA_RESULT);                                                /* L: ... [G] */

    if (traceback)
        lua_pop(_state, 1); // remove __G__TRACKBACK__ from stack      /* L: ... */

    return true;
}

bool ScriptEngine::ExecuteGlobalFunction(IKernel * kernel, const char * functionName, const ResultReader& reader)  {
    lua_getglobal(_state, functionName);       /* query function by name, stack: function */
    if (!lua_isfunction(_state, -1)) {
        OASSERT(false, "execute global func but is not a function");
        lua_pop(_state, 1);
        return 0;
    }
    return ExecuteFunction(kernel, 0, reader);
}

bool ScriptEngine::ExecuteString(IKernel * kernel, const char *codes, const ResultReader& reader) {
    luaL_loadstring(_state, codes);
    return ExecuteFunction(kernel, 0, reader);
}

bool ScriptEngine::ExecuteScriptFile(IKernel * kernel, const char * module) {
    char code[MAX_CODE_SIZE];
    SafeSprintf(code, sizeof(code), "require '%s'", module);
    return ExecuteString(kernel, code, nullptr);
}
