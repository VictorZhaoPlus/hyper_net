SET(COMMAND
    "${PROJECT_SOURCE_DIR}/src/luaserver/command/Command.h"
    "${PROJECT_SOURCE_DIR}/src/luaserver/command/Command.cpp" 
    )
	
SET(HTTP
    "${PROJECT_SOURCE_DIR}/src/luaserver/http/HttpScriptModule.h"
    "${PROJECT_SOURCE_DIR}/src/luaserver/http/HttpScriptModule.cpp" 
    )
	
SET(LUA_SERVER
    "${PROJECT_SOURCE_DIR}/src/luaserver/main.cpp"
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
		
IF(WIN32)
TARGET_LINK_LIBRARIES(LuaServer
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(LuaServer PROPERTIES FOLDER "framework")