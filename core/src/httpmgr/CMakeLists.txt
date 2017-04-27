
INCLUDE_DIRECTORIES(
	"${PROJECT_SOURCE_DIR}/thirdlibary/include"
    )

SET(HTTPMGR
    "${PROJECT_SOURCE_DIR}/src/httpmgr/HttpMgr.h"
    "${PROJECT_SOURCE_DIR}/src/httpmgr/HttpMgr.cpp"
	"${PROJECT_SOURCE_DIR}/src/httpmgr/HttpBase.h"
    "${PROJECT_SOURCE_DIR}/src/httpmgr/HttpBase.cpp"
    "${PROJECT_SOURCE_DIR}/src/httpmgr/main.cpp"
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

IF(WIN32)
TARGET_LINK_LIBRARIES(HttpMgr
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/libcurl.lib"
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(HttpMgr PROPERTIES FOLDER "core")
