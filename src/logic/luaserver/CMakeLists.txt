
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(RPC
    "${PROJECT_SOURCE_DIR}/src/logic/luaserver/rpc/Rpc.h"
    "${PROJECT_SOURCE_DIR}/src/logic/luaserver/rpc/Rpc.cpp" 
    )
	
SET(HTTP
    "${PROJECT_SOURCE_DIR}/src/logic/luaserver/http/HttpScriptModule.h"
    "${PROJECT_SOURCE_DIR}/src/logic/luaserver/http/HttpScriptModule.cpp" 
    )
	
SET(AGENT
    "${PROJECT_SOURCE_DIR}/src/logic/luaserver/agent/AgentModule.h"
    "${PROJECT_SOURCE_DIR}/src/logic/luaserver/agent/AgentModule.cpp" 
    )
	
SET(LUA_SERVER
    "${PROJECT_SOURCE_DIR}/src/logic/luaserver/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${LUA_SERVER})
SOURCE_GROUP(rpc FILES ${RPC})
SOURCE_GROUP(http FILES ${HTTP})
SOURCE_GROUP(agent FILES ${AGENT})

ADD_LIBRARY(LuaServer SHARED
        ${LUA_SERVER}
		${RPC}
		${HTTP}
		${AGENT}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )
SET_TARGET_PROPERTIES(LuaServer PROPERTIES FOLDER "framework")
