
INCLUDE_DIRECTORIES(
	"${PROJECT_SOURCE_DIR}/src/cluster"
    )

SET(CLUSTER_MAIN
    "${PROJECT_SOURCE_DIR}/src/cluster/proto.h"
	"${PROJECT_SOURCE_DIR}/src/cluster/main.cpp"
    )
	
SET(CLUSTER
    "${PROJECT_SOURCE_DIR}/src/cluster/cluster/Cluster.h"
    "${PROJECT_SOURCE_DIR}/src/cluster/cluster/Cluster.cpp"
    )
	
SET(MASTER
    "${PROJECT_SOURCE_DIR}/src/cluster/master/Master.h"
    "${PROJECT_SOURCE_DIR}/src/cluster/master/Master.cpp"
    )
	
SET(SLAVE
    "${PROJECT_SOURCE_DIR}/src/cluster/slave/Slave.h"
    "${PROJECT_SOURCE_DIR}/src/cluster/slave/Slave.cpp"
    )
	
SET(STARTER
    "${PROJECT_SOURCE_DIR}/src/cluster/starter/Starter.h"
    "${PROJECT_SOURCE_DIR}/src/cluster/starter/Starter.cpp"
    "${PROJECT_SOURCE_DIR}/src/cluster/starter/StartNodeTimer.h"
    "${PROJECT_SOURCE_DIR}/src/cluster/starter/StartNodeTimer.cpp"
    )


SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${CLUSTER_MAIN})
SOURCE_GROUP(cluster FILES ${CLUSTER})
SOURCE_GROUP(master FILES ${MASTER})
SOURCE_GROUP(slave FILES ${SLAVE})
SOURCE_GROUP(starter FILES ${STARTER})

ADD_LIBRARY(Cluster SHARED
	${CLUSTER_MAIN}
	${CLUSTER}
	${MASTER}
	${SLAVE}
	${STARTER}
	${API}
	${LOGIC_INTERFACE}
	${PUBLIC}
	${PUBLIC_XML}
	)

IF(WIN32)
TARGET_LINK_LIBRARIES(Cluster
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(Cluster PROPERTIES FOLDER "core")
