SET(SCENE
    "${PROJECT_SOURCE_DIR}/src/scene/SceneMgr.h"
    "${PROJECT_SOURCE_DIR}/src/scene/SceneMgr.cpp"
    "${PROJECT_SOURCE_DIR}/src/scene/SceneClient.h"
    "${PROJECT_SOURCE_DIR}/src/scene/SceneClient.cpp"
    "${PROJECT_SOURCE_DIR}/src/scene/Scene.h"
    "${PROJECT_SOURCE_DIR}/src/scene/Scene.cpp"
    "${PROJECT_SOURCE_DIR}/src/scene/Vision.h"
    "${PROJECT_SOURCE_DIR}/src/scene/Vision.cpp"
    "${PROJECT_SOURCE_DIR}/src/scene/Watcher.h"
    "${PROJECT_SOURCE_DIR}/src/scene/Watcher.cpp"
    "${PROJECT_SOURCE_DIR}/src/scene/main.cpp"
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

IF(WIN32)
TARGET_LINK_LIBRARIES(Scene
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(Scene PROPERTIES FOLDER "framework")