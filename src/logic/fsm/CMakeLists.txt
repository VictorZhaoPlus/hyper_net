
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(FSM
    "${PROJECT_SOURCE_DIR}/src/logic/fsm/FSM.h"
    "${PROJECT_SOURCE_DIR}/src/logic/fsm/FSM.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/fsm/ObjectFsm.h"
    "${PROJECT_SOURCE_DIR}/src/logic/fsm/ObjectFsm.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/fsm/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${FSM})

ADD_LIBRARY(FSM SHARED
        ${FSM}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

#SET_TARGET_PROPERTIES(FSM PROPERTIES FOLDER "core")
SET_TARGET_PROPERTIES(FSM PROPERTIES FOLDER "framework")
#SET_TARGET_PROPERTIES(FSM PROPERTIES FOLDER "logic")
