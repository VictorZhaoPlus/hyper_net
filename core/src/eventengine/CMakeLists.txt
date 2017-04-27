SET(EVENTENGINE
    "${PROJECT_SOURCE_DIR}/src/eventengine/EventEngine.h"
    "${PROJECT_SOURCE_DIR}/src/eventengine/EventEngine.cpp"
    "${PROJECT_SOURCE_DIR}/src/eventengine/main.cpp"
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

IF(WIN32)
TARGET_LINK_LIBRARIES(EventEngine
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(EventEngine PROPERTIES FOLDER "core")

