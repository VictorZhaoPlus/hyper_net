
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(LOGIN
    "${PROJECT_SOURCE_DIR}/src/logic/login/Gate.h"
    "${PROJECT_SOURCE_DIR}/src/logic/login/Gate.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/login/Login.h"
    "${PROJECT_SOURCE_DIR}/src/logic/login/Login.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/login/Distribution.h"
    "${PROJECT_SOURCE_DIR}/src/logic/login/Distribution.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/login/PlayerMgr.h"
    "${PROJECT_SOURCE_DIR}/src/logic/login/PlayerMgr.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/login/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${LOGIN})

ADD_LIBRARY(Login SHARED
        ${LOGIN}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

#SET_TARGET_PROPERTIES(Login PROPERTIES FOLDER "core")
SET_TARGET_PROPERTIES(Login PROPERTIES FOLDER "framework")
#SET_TARGET_PROPERTIES(Login PROPERTIES FOLDER "logic")
