
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(CACHEDB
    "${PROJECT_SOURCE_DIR}/src/logic/cachedb/CacheDB.h"
    "${PROJECT_SOURCE_DIR}/src/logic/cachedb/CacheDB.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/cachedb/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${CACHEDB})

ADD_LIBRARY(CacheDB SHARED
        ${CACHEDB}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

SET_TARGET_PROPERTIES(CacheDB PROPERTIES FOLDER "core")
#SET_TARGET_PROPERTIES(CacheDB PROPERTIES FOLDER "framework")
#SET_TARGET_PROPERTIES(CacheDB PROPERTIES FOLDER "logic")
