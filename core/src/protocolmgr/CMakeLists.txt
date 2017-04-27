SET(PROTOCOLMGR
    "${PROJECT_SOURCE_DIR}/src/protocolmgr/ProtocolMgr.h"
    "${PROJECT_SOURCE_DIR}/src/protocolmgr/ProtocolMgr.cpp"
    "${PROJECT_SOURCE_DIR}/src/protocolmgr/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${PROTOCOLMGR})

ADD_LIBRARY(ProtocolMgr SHARED
        ${PROTOCOLMGR}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

IF(WIN32)
TARGET_LINK_LIBRARIES(ProtocolMgr
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(ProtocolMgr PROPERTIES FOLDER "core")