SET(MAPREDUCE
    "${PROJECT_SOURCE_DIR}/src/mapreduce/MapReduce.h"
    "${PROJECT_SOURCE_DIR}/src/mapreduce/MapReduce.cpp"
    "${PROJECT_SOURCE_DIR}/src/mapreduce/AsyncMapReduceBase.h"
    "${PROJECT_SOURCE_DIR}/src/mapreduce/AsyncMapReduceBase.cpp"
    "${PROJECT_SOURCE_DIR}/src/mapreduce/AsyncMapTask.h"
    "${PROJECT_SOURCE_DIR}/src/mapreduce/AsyncMapTask.cpp"
    "${PROJECT_SOURCE_DIR}/src/mapreduce/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${MAPREDUCE})

ADD_LIBRARY(MapReduce SHARED
        ${MAPREDUCE}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

IF(WIN32)
TARGET_LINK_LIBRARIES(MapReduce
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(MapReduce PROPERTIES FOLDER "core")
