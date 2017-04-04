
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(OBJECTLOCATOR
    "${PROJECT_SOURCE_DIR}/src/logic/objectlocator/ObjectLocator.h"
    "${PROJECT_SOURCE_DIR}/src/logic/objectlocator/ObjectLocator.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/objectlocator/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${OBJECTLOCATOR})

ADD_LIBRARY(ObjectLocator SHARED
        ${OBJECTLOCATOR}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

#SET_TARGET_PROPERTIES(ObjectLocator PROPERTIES FOLDER "core")
SET_TARGET_PROPERTIES(ObjectLocator PROPERTIES FOLDER "framework")
#SET_TARGET_PROPERTIES(ObjectLocator PROPERTIES FOLDER "logic")
