
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(AGENT
    "${PROJECT_SOURCE_DIR}/src/logic/agent/Agent.h"
    "${PROJECT_SOURCE_DIR}/src/logic/agent/Agent.cpp"
	"${PROJECT_SOURCE_DIR}/src/logic/agent/AgentSession.h"
    "${PROJECT_SOURCE_DIR}/src/logic/agent/AgentSession.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/agent/main.cpp"
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

