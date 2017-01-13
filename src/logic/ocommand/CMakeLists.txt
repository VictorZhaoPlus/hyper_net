
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(OCOMMAND
    "${PROJECT_SOURCE_DIR}/src/logic/ocommand/OCommand.h"
    "${PROJECT_SOURCE_DIR}/src/logic/ocommand/OCommand.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/ocommand/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${OCOMMAND})

ADD_LIBRARY(OCommand SHARED
        ${OCOMMAND}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

SET_TARGET_PROPERTIES(OCommand PROPERTIES FOLDER "core")
#SET_TARGET_PROPERTIES(OCommand PROPERTIES FOLDER "framework")
#SET_TARGET_PROPERTIES(OCommand PROPERTIES FOLDER "logic")
