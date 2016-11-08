
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(IDMGR
    "${PROJECT_SOURCE_DIR}/src/logic/idmgr/IdMgr.h"
    "${PROJECT_SOURCE_DIR}/src/logic/idmgr/IdMgr.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/idmgr/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${IDMGR})

ADD_LIBRARY(IdMgr SHARED
        ${IDMGR}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

SET_TARGET_PROPERTIES(IdMgr PROPERTIES FOLDER "core")
#SET_TARGET_PROPERTIES(IdMgr PROPERTIES FOLDER "framework")
#SET_TARGET_PROPERTIES(IdMgr PROPERTIES FOLDER "logic")
