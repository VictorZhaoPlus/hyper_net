
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(PROTOCOLMGR
    "${PROJECT_SOURCE_DIR}/src/logic/protocolmgr/ProtocolMgr.h"
    "${PROJECT_SOURCE_DIR}/src/logic/protocolmgr/ProtocolMgr.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/protocolmgr/main.cpp"
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

SET_TARGET_PROPERTIES(ProtocolMgr PROPERTIES FOLDER "core")
#SET_TARGET_PROPERTIES(ProtocolMgr PROPERTIES FOLDER "framework")
#SET_TARGET_PROPERTIES(ProtocolMgr PROPERTIES FOLDER "logic")
