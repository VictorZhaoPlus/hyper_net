SET(HARBOR
    "${PROJECT_SOURCE_DIR}/src/harbor/Harbor.h"
    "${PROJECT_SOURCE_DIR}/src/harbor/Harbor.cpp"
    "${PROJECT_SOURCE_DIR}/src/harbor/NodeSession.h"
    "${PROJECT_SOURCE_DIR}/src/harbor/NodeSession.cpp"
    "${PROJECT_SOURCE_DIR}/src/harbor/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${HARBOR})

ADD_LIBRARY(Harbor SHARED
	${HARBOR}
	${API}
	${LOGIC_INTERFACE}
	${PUBLIC}
	${PUBLIC_XML}
	)
		
IF(WIN32)
TARGET_LINK_LIBRARIES(Harbor
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(Harbor PROPERTIES FOLDER "core")
