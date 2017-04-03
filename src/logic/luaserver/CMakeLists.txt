
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(COMMAND
    "${PROJECT_SOURCE_DIR}/src/logic/luaserver/command/Command.h"
    "${PROJECT_SOURCE_DIR}/src/logic/luaserver/command/Command.cpp" 
    )
	
SET(HTTP
    "${PROJECT_SOURCE_DIR}/src/logic/luaserver/http/HttpScriptModule.h"
    "${PROJECT_SOURCE_DIR}/src/logic/luaserver/http/HttpScriptModule.cpp" 
    )
	
SET(LUA_SERVER
    "${PROJECT_SOURCE_DIR}/src/logic/luaserver/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${LUA_SERVER})
SOURCE_GROUP(command FILES ${COMMAND})
SOURCE_GROUP(http FILES ${HTTP})

ADD_LIBRARY(LuaServer SHARED
        ${LUA_SERVER}
		${COMMAND}
		${HTTP}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )
SET_TARGET_PROPERTIES(LuaServer PROPERTIES FOLDER "framework")
