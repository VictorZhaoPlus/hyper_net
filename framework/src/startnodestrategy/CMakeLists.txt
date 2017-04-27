SET(STARTNODESTRATEGY
    "${PROJECT_SOURCE_DIR}/src/startnodestrategy/StartNodeStrategy.h"
    "${PROJECT_SOURCE_DIR}/src/startnodestrategy/StartNodeStrategy.cpp"
    "${PROJECT_SOURCE_DIR}/src/startnodestrategy/main.cpp"
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

IF(WIN32)
TARGET_LINK_LIBRARIES(StartNodeStrategy
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(StartNodeStrategy PROPERTIES FOLDER "framework")
