
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(HTTPMGR
    "${PROJECT_SOURCE_DIR}/src/logic/httpmgr/HttpMgr.h"
    "${PROJECT_SOURCE_DIR}/src/logic/httpmgr/HttpMgr.cpp"
	"${PROJECT_SOURCE_DIR}/src/logic/httpmgr/HttpBase.h"
    "${PROJECT_SOURCE_DIR}/src/logic/httpmgr/HttpBase.cpp"
	"${PROJECT_SOURCE_DIR}/src/logic/httpmgr/HttpScriptModule.h"
    "${PROJECT_SOURCE_DIR}/src/logic/httpmgr/HttpScriptModule.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/httpmgr/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${HTTPMGR})

ADD_LIBRARY(HttpMgr SHARED
        ${HTTPMGR}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

