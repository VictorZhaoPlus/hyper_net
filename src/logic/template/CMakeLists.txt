
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(@
    "${PROJECT_SOURCE_DIR}/src/logic/+/&.h"
    "${PROJECT_SOURCE_DIR}/src/logic/+/&.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/+/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${@})

ADD_LIBRARY(& SHARED
        ${@}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

