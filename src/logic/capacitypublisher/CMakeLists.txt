
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(CAPACITYPUBLISHER
    "${PROJECT_SOURCE_DIR}/src/logic/capacitypublisher/CapacityPublisher.h"
    "${PROJECT_SOURCE_DIR}/src/logic/capacitypublisher/CapacityPublisher.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/capacitypublisher/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${CAPACITYPUBLISHER})

ADD_LIBRARY(CapacityPublisher SHARED
        ${CAPACITYPUBLISHER}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

