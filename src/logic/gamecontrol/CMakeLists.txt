
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(GAMECONTROL
    "${PROJECT_SOURCE_DIR}/src/logic/gamecontrol/GameControl.h"
    "${PROJECT_SOURCE_DIR}/src/logic/gamecontrol/GameControl.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/gamecontrol/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${GAMECONTROL})

ADD_LIBRARY(GameControl SHARED
        ${GAMECONTROL}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

#SET_TARGET_PROPERTIES(GameControl PROPERTIES FOLDER "core")
SET_TARGET_PROPERTIES(GameControl PROPERTIES FOLDER "framework")
#SET_TARGET_PROPERTIES(GameControl PROPERTIES FOLDER "logic")
