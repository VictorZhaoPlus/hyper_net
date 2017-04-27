SET(CAPACITYPUBLISHER
    "${PROJECT_SOURCE_DIR}/src/monitor/capacitypublisher/CapacityPublisher.h"
    "${PROJECT_SOURCE_DIR}/src/monitor/capacitypublisher/CapacityPublisher.cpp"
    )
	
SET(CAPACITYSUBSCRIBER
    "${PROJECT_SOURCE_DIR}/src/monitor/capacitysubscriber/CapacitySubscriber.h"
    "${PROJECT_SOURCE_DIR}/src/monitor/capacitysubscriber/CapacitySubscriber.cpp"
    )
	
SET(MONITOR
	"${PROJECT_SOURCE_DIR}/src/monitor/main.cpp"
)

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${MONITOR})
SOURCE_GROUP(capacitypublisher FILES ${CAPACITYPUBLISHER})
SOURCE_GROUP(capacitysubscriber FILES ${CAPACITYSUBSCRIBER})

ADD_LIBRARY(Monitor SHARED
	${CAPACITYPUBLISHER}
	${CAPACITYSUBSCRIBER}
	${MONITOR}
	${API}
	${LOGIC_INTERFACE}
	${PUBLIC}
	${PUBLIC_XML}
	)
		
IF(WIN32)
TARGET_LINK_LIBRARIES(Monitor
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(Monitor PROPERTIES FOLDER "core")
