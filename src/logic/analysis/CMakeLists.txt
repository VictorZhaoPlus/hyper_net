
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(ANALYSIS
    "${PROJECT_SOURCE_DIR}/src/logic/analysis/Analysis.h"
    "${PROJECT_SOURCE_DIR}/src/logic/analysis/Analysis.cpp"
	"${PROJECT_SOURCE_DIR}/src/logic/analysis/AnalysisClient.h"
    "${PROJECT_SOURCE_DIR}/src/logic/analysis/AnalysisClient.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/analysis/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${ANALYSIS})

ADD_LIBRARY(Analysis SHARED
        ${ANALYSIS}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

