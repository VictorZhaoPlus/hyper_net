SET(ANALYSIS
    "${PROJECT_SOURCE_DIR}/src/analysis/Analysis.h"
    "${PROJECT_SOURCE_DIR}/src/analysis/Analysis.cpp"
	"${PROJECT_SOURCE_DIR}/src/analysis/AnalysisClient.h"
    "${PROJECT_SOURCE_DIR}/src/analysis/AnalysisClient.cpp"
    "${PROJECT_SOURCE_DIR}/src/analysis/main.cpp"
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
		
IF(WIN32)
TARGET_LINK_LIBRARIES(Analysis
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(Analysis PROPERTIES FOLDER "core")

