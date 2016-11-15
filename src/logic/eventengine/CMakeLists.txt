
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(EVENTENGINE
    "${PROJECT_SOURCE_DIR}/src/logic/eventengine/EventEngine.h"
    "${PROJECT_SOURCE_DIR}/src/logic/eventengine/EventEngine.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/eventengine/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${EVENTENGINE})

ADD_LIBRARY(EventEngine SHARED
        ${EVENTENGINE}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

SET_TARGET_PROPERTIES(EventEngine PROPERTIES FOLDER "core")
#SET_TARGET_PROPERTIES(EventEngine PROPERTIES FOLDER "framework")
#SET_TARGET_PROPERTIES(EventEngine PROPERTIES FOLDER "logic")
