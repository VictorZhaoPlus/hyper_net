SET(OBJECTTIMER
    "${PROJECT_SOURCE_DIR}/src/objecttimer/ObjectTimer.h"
    "${PROJECT_SOURCE_DIR}/src/objecttimer/ObjectTimer.cpp"
    "${PROJECT_SOURCE_DIR}/src/objecttimer/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${OBJECTTIMER})

ADD_LIBRARY(ObjectTimer SHARED
        ${OBJECTTIMER}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

IF(WIN32)
TARGET_LINK_LIBRARIES(ObjectTimer
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(ObjectTimer PROPERTIES FOLDER "core")