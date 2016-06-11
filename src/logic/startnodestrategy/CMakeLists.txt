
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(STARTNODESTRATEGY
    "${PROJECT_SOURCE_DIR}/src/logic/startnodestrategy/StartNodeStrategy.h"
    "${PROJECT_SOURCE_DIR}/src/logic/startnodestrategy/StartNodeStrategy.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/startnodestrategy/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${STARTNODESTRATEGY})

ADD_LIBRARY(StartNodeStrategy SHARED
        ${STARTNODESTRATEGY}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

SET_TARGET_PROPERTIES(StartNodeStrategy PROPERTIES FOLDER "framework") 