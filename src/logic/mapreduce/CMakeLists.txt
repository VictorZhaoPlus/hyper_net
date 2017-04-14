
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(MAPREDUCE
    "${PROJECT_SOURCE_DIR}/src/logic/mapreduce/MapReduce.h"
    "${PROJECT_SOURCE_DIR}/src/logic/mapreduce/MapReduce.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/mapreduce/AsyncMapReduceBase.h"
    "${PROJECT_SOURCE_DIR}/src/logic/mapreduce/AsyncMapReduceBase.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/mapreduce/AsyncMapTask.h"
    "${PROJECT_SOURCE_DIR}/src/logic/mapreduce/AsyncMapTask.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/mapreduce/main.cpp"
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

SET_TARGET_PROPERTIES(MapReduce PROPERTIES FOLDER "core")
#SET_TARGET_PROPERTIES(MapReduce PROPERTIES FOLDER "framework")
#SET_TARGET_PROPERTIES(MapReduce PROPERTIES FOLDER "logic")
