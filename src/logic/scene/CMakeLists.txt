
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(SCENE
    "${PROJECT_SOURCE_DIR}/src/logic/scene/SceneMgr.h"
    "${PROJECT_SOURCE_DIR}/src/logic/scene/SceneMgr.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/scene/SceneClient.h"
    "${PROJECT_SOURCE_DIR}/src/logic/scene/SceneClient.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/scene/Cell.h"
    "${PROJECT_SOURCE_DIR}/src/logic/scene/Cell.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/scene/CellInterface.h"
    "${PROJECT_SOURCE_DIR}/src/logic/scene/CellInterface.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/scene/Watcher.h"
    "${PROJECT_SOURCE_DIR}/src/logic/scene/Watcher.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/scene/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${SCENE})

ADD_LIBRARY(Scene SHARED
        ${SCENE}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

#SET_TARGET_PROPERTIES(Scene PROPERTIES FOLDER "core")
SET_TARGET_PROPERTIES(Scene PROPERTIES FOLDER "framework")
#SET_TARGET_PROPERTIES(Scene PROPERTIES FOLDER "logic")
