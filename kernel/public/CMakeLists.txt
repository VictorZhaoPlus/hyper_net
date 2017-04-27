
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    )

SET(PUBLIC
    "${PROJECT_SOURCE_DIR}/src/public/util.h"
    "${PROJECT_SOURCE_DIR}/src/public/util.cpp"
    "${PROJECT_SOURCE_DIR}/src/public/tools.h"
	"${PROJECT_SOURCE_DIR}/src/public/tools.cpp"
    "${PROJECT_SOURCE_DIR}/src/public/singleton.h"
	"${PROJECT_SOURCE_DIR}/src/public/XmlReader.h"
	"${PROJECT_SOURCE_DIR}/src/public/XmlReader.cpp"
	"${PROJECT_SOURCE_DIR}/src/public/CycleQueue.h"
	"${PROJECT_SOURCE_DIR}/src/public/OMap.h"
	"${PROJECT_SOURCE_DIR}/src/public/Fsm.h"
	"${PROJECT_SOURCE_DIR}/src/public/OPool.h"
	"${PROJECT_SOURCE_DIR}/src/public/OArgs.h"
	"${PROJECT_SOURCE_DIR}/src/public/OBuffer.h"
	"${PROJECT_SOURCE_DIR}/src/public/ORingBuffer.h"
	"${PROJECT_SOURCE_DIR}/src/public/ORingBuffer.c"
	"${PROJECT_SOURCE_DIR}/src/public/base64.h"
	"${PROJECT_SOURCE_DIR}/src/public/base64.c"
	"${PROJECT_SOURCE_DIR}/src/public/spin_mutex.h"
	"${PROJECT_SOURCE_DIR}/src/public/Md5.h"
	"${PROJECT_SOURCE_DIR}/src/public/Md5.cpp"
    )

SET(PUBLIC_XML
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml/tinystr.cpp"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml/tinystr.h"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml/tinyxml.cpp"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml/tinyxmlerror.cpp"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml/tinyxml.h"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml/tinyxmlparser.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${AGENT})

ADD_LIBRARY(Agent SHARED
        ${AGENT}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )
SET_TARGET_PROPERTIES(Agent PROPERTIES FOLDER "core")
