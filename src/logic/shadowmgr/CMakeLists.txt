
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(SHADOWMGR
    "${PROJECT_SOURCE_DIR}/src/logic/shadowmgr/ShadowMgr.h"
    "${PROJECT_SOURCE_DIR}/src/logic/shadowmgr/ShadowMgr.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/shadowmgr/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${SHADOWMGR})

ADD_LIBRARY(ShadowMgr SHARED
        ${SHADOWMGR}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

#SET_TARGET_PROPERTIES(ShadowMgr PROPERTIES FOLDER "core")
SET_TARGET_PROPERTIES(ShadowMgr PROPERTIES FOLDER "framework")
#SET_TARGET_PROPERTIES(ShadowMgr PROPERTIES FOLDER "logic")
