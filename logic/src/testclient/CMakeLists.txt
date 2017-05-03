
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/interface"
    )

SET(TESTCLIENT
    "${PROJECT_SOURCE_DIR}/src/testclient/TestClient.h"
    "${PROJECT_SOURCE_DIR}/src/testclient/TestClient.cpp"
    "${PROJECT_SOURCE_DIR}/src/testclient/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${TESTCLIENT})

ADD_LIBRARY(TestClient SHARED
	${TESTCLIENT}
	${API}
	${LOGIC_INTERFACE}
	${PUBLIC}
	${PUBLIC_XML}
	)

IF(WIN32)
TARGET_LINK_LIBRARIES(TestClient
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(TestClient PROPERTIES FOLDER "logic")
