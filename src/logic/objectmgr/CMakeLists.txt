#cmake file for project kernel
#author:T
#created:2015-01-12 15:07:04

INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(MAIN
    "${PROJECT_SOURCE_DIR}/src/logic/objectmgr/ObjectMgr.h"
    "${PROJECT_SOURCE_DIR}/src/logic/objectmgr/ObjectMgr.cpp"
	"${PROJECT_SOURCE_DIR}/src/logic/objectmgr/Memory.h"
	"${PROJECT_SOURCE_DIR}/src/logic/objectmgr/MemoryPool.h"
	"${PROJECT_SOURCE_DIR}/src/logic/objectmgr/MemoryPool.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/objectmgr/MMObject.h"
    "${PROJECT_SOURCE_DIR}/src/logic/objectmgr/MMObject.cpp"
	"${PROJECT_SOURCE_DIR}/src/logic/objectmgr/ObjectDescriptor.h"
    "${PROJECT_SOURCE_DIR}/src/logic/objectmgr/ObjectDescriptor.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/objectmgr/ObjectProp.h"
    "${PROJECT_SOURCE_DIR}/src/logic/objectmgr/ObjectProp.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/objectmgr/TableControl.h"
    "${PROJECT_SOURCE_DIR}/src/logic/objectmgr/TableControl.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/objectmgr/TableRow.h"
    "${PROJECT_SOURCE_DIR}/src/logic/objectmgr/TableRow.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/objectmgr/main.cpp"
	)
    
SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${MAIN})

ADD_LIBRARY(ObjectMgr SHARED
	${MAIN}
	${API}
	${LOGIC_INTERFACE}
	${PUBLIC}
	${PUBLIC_XML}
	)
SET_TARGET_PROPERTIES(ObjectMgr PROPERTIES FOLDER "core") 