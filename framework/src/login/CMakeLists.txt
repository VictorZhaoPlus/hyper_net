SET(LOGIN
    "${PROJECT_SOURCE_DIR}/src/login/Gate.h"
    "${PROJECT_SOURCE_DIR}/src/login/Gate.cpp"
    "${PROJECT_SOURCE_DIR}/src/login/Login.h"
    "${PROJECT_SOURCE_DIR}/src/login/Login.cpp"
    "${PROJECT_SOURCE_DIR}/src/login/Distribution.h"
    "${PROJECT_SOURCE_DIR}/src/login/Distribution.cpp"
    "${PROJECT_SOURCE_DIR}/src/login/Logic.h"
    "${PROJECT_SOURCE_DIR}/src/login/Logic.cpp"
    "${PROJECT_SOURCE_DIR}/src/login/PacketSender.h"
    "${PROJECT_SOURCE_DIR}/src/login/PacketSender.cpp"
    "${PROJECT_SOURCE_DIR}/src/login/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${LOGIN})

ADD_LIBRARY(Login SHARED
        ${LOGIN}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

IF(WIN32)
TARGET_LINK_LIBRARIES(Login
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(Login PROPERTIES FOLDER "framework")
