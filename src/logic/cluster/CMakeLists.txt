
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
    )

SET(CLUSTER_MAIN
    "${PROJECT_SOURCE_DIR}/src/logic/cluster/main.cpp"
    )
	
SET(CLUSTER
    "${PROJECT_SOURCE_DIR}/src/logic/cluster/cluster/Cluster.h"
    "${PROJECT_SOURCE_DIR}/src/logic/cluster/cluster/Cluster.cpp"
    )
	
SET(MASTER
    "${PROJECT_SOURCE_DIR}/src/logic/cluster/master/Master.h"
    "${PROJECT_SOURCE_DIR}/src/logic/cluster/master/Master.cpp"
    )
	
SET(SLAVE
    "${PROJECT_SOURCE_DIR}/src/logic/cluster/slave/Slave.h"
    "${PROJECT_SOURCE_DIR}/src/logic/cluster/slave/Slave.cpp"
    )
	
SET(STARTER
    "${PROJECT_SOURCE_DIR}/src/logic/cluster/starter/Starter.h"
    "${PROJECT_SOURCE_DIR}/src/logic/cluster/starter/Starter.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/cluster/starter/StartNodeTimer.h"
    "${PROJECT_SOURCE_DIR}/src/logic/cluster/starter/StartNodeTimer.cpp"
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

SET_TARGET_PROPERTIES(Cluster PROPERTIES FOLDER "core")
