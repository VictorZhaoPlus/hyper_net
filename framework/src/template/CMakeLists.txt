SET(@
    "${PROJECT_SOURCE_DIR}/src/+/&.h"
    "${PROJECT_SOURCE_DIR}/src/+/&.cpp"
    "${PROJECT_SOURCE_DIR}/src/+/main.cpp"
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

IF(WIN32)
TARGET_LINK_LIBRARIES(&
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(& PROPERTIES FOLDER "framework")