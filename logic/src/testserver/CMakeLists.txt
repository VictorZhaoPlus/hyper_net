
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/interface"
    )

SET(TESTSERVER
    "${PROJECT_SOURCE_DIR}/src/testserver/TestServer.h"
    "${PROJECT_SOURCE_DIR}/src/testserver/TestServer.cpp"
    "${PROJECT_SOURCE_DIR}/src/testserver/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${TESTSERVER})

ADD_LIBRARY(TestServer SHARED
	${TESTSERVER}
	${API}
	${LOGIC_INTERFACE}
	${PUBLIC}
	${PUBLIC_XML}
	)

IF(WIN32)
TARGET_LINK_LIBRARIES(TestServer
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(TestServer PROPERTIES FOLDER "logic")
