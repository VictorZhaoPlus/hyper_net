#cmake file for project kernel
#author:T
#created:2015-01-12 15:07:04

SET(MAIN
    "${PROJECT_SOURCE_DIR}/src/objectmgr/ObjectMgr.h"
    "${PROJECT_SOURCE_DIR}/src/objectmgr/ObjectMgr.cpp"
	"${PROJECT_SOURCE_DIR}/src/objectmgr/Memory.h"
	"${PROJECT_SOURCE_DIR}/src/objectmgr/MemoryPool.h"
	"${PROJECT_SOURCE_DIR}/src/objectmgr/MemoryPool.cpp"
    "${PROJECT_SOURCE_DIR}/src/objectmgr/MMObject.h"
    "${PROJECT_SOURCE_DIR}/src/objectmgr/MMObject.cpp"
	"${PROJECT_SOURCE_DIR}/src/objectmgr/ObjectDescriptor.h"
    "${PROJECT_SOURCE_DIR}/src/objectmgr/ObjectDescriptor.cpp"
    "${PROJECT_SOURCE_DIR}/src/objectmgr/ObjectProp.h"
    "${PROJECT_SOURCE_DIR}/src/objectmgr/ObjectProp.cpp"
    "${PROJECT_SOURCE_DIR}/src/objectmgr/TableControl.h"
    "${PROJECT_SOURCE_DIR}/src/objectmgr/TableControl.cpp"
    "${PROJECT_SOURCE_DIR}/src/objectmgr/TableRow.h"
    "${PROJECT_SOURCE_DIR}/src/objectmgr/TableRow.cpp"
    "${PROJECT_SOURCE_DIR}/src/objectmgr/main.cpp"
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
	
IF(WIN32)
TARGET_LINK_LIBRARIES(ObjectMgr
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(ObjectMgr PROPERTIES FOLDER "core")
