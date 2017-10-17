IF(UNIX)
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/API/"
    "${PROJECT_SOURCE_DIR}/public/"
    "${PROJECT_SOURCE_DIR}/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/"
    "${PROJECT_SOURCE_DIR}/src/config/"
    "${PROJECT_SOURCE_DIR}/src/net/epoll/"
    "${PROJECT_SOURCE_DIR}/src/lua/"
    "${PROJECT_SOURCE_DIR}/src/define/"
    "${PROJECT_SOURCE_DIR}/src/logic/"
	"${PROJECT_SOURCE_DIR}/src/logger/"
	"${PROJECT_SOURCE_DIR}/src/timer/"
	"${PROJECT_SOURCE_DIR}/src/async/"
	"${PROJECT_SOURCE_DIR}/src/profile/"
	"${PROJECT_SOURCE_DIR}/src/id/"
    )
ENDIF(UNIX)

IF(WIN32)
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/API/"
    "${PROJECT_SOURCE_DIR}/public/"
    "${PROJECT_SOURCE_DIR}/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/"
    "${PROJECT_SOURCE_DIR}/src/config/"
    "${PROJECT_SOURCE_DIR}/src/net/iocp/"
    "${PROJECT_SOURCE_DIR}/src/lua/"
    "${PROJECT_SOURCE_DIR}/src/define/"
    "${PROJECT_SOURCE_DIR}/src/logic/"
	"${PROJECT_SOURCE_DIR}/src/logger/"
	"${PROJECT_SOURCE_DIR}/src/timer/"
	"${PROJECT_SOURCE_DIR}/src/async/"
	"${PROJECT_SOURCE_DIR}/src/profile/"
	"${PROJECT_SOURCE_DIR}/src/id/"
    )
ENDIF(WIN32)

SET(KERNEL_CONFIG
    "${PROJECT_SOURCE_DIR}/src/config/ConfigMgr.h"
    "${PROJECT_SOURCE_DIR}/src/config/ConfigMgr.cpp"
    )
	
SET(KERNEL_ASYNC
    "${PROJECT_SOURCE_DIR}/src/async/AsyncMgr.h"
    "${PROJECT_SOURCE_DIR}/src/async/AsyncMgr.cpp"
	"${PROJECT_SOURCE_DIR}/src/async/AsyncBase.h"
    "${PROJECT_SOURCE_DIR}/src/async/AsyncBase.cpp"
	"${PROJECT_SOURCE_DIR}/src/async/AsyncThread.h"
    "${PROJECT_SOURCE_DIR}/src/async/AsyncThread.cpp"
    )

IF(UNIX)
SET(KERNEL_NET
    "${PROJECT_SOURCE_DIR}/src/net/epoll/NetEngine.h"
    "${PROJECT_SOURCE_DIR}/src/net/epoll/NetEngine.cpp"
    "${PROJECT_SOURCE_DIR}/src/net/epoll/Connection.h"
    "${PROJECT_SOURCE_DIR}/src/net/epoll/Connection.cpp"
	"${PROJECT_SOURCE_DIR}/src/net/epoll/NetWorker.h"
    "${PROJECT_SOURCE_DIR}/src/net/epoll/NetWorker.cpp" 
    )
ENDIF(UNIX)

IF(WIN32)
SET(KERNEL_NET
    "${PROJECT_SOURCE_DIR}/src/net/iocp/NetEngine.h"
    "${PROJECT_SOURCE_DIR}/src/net/iocp/NetEngine.cpp"
    "${PROJECT_SOURCE_DIR}/src/net/iocp/Iocp.h"
    "${PROJECT_SOURCE_DIR}/src/net/iocp/Iocp.cpp"
    "${PROJECT_SOURCE_DIR}/src/net/iocp/Connection.h"
    "${PROJECT_SOURCE_DIR}/src/net/iocp/Connection.cpp"
    )
ENDIF(WIN32)
	
SET(KERNEL_TIMER
    "${PROJECT_SOURCE_DIR}/src/timer/TimerMgr.h"
    "${PROJECT_SOURCE_DIR}/src/timer/TimerMgr.cpp"
	"${PROJECT_SOURCE_DIR}/src/timer/TimerGear.h"
    "${PROJECT_SOURCE_DIR}/src/timer/TimerGear.cpp"
	"${PROJECT_SOURCE_DIR}/src/timer/TimerList.h"
    "${PROJECT_SOURCE_DIR}/src/timer/TimerList.cpp"
	"${PROJECT_SOURCE_DIR}/src/timer/TimerBase.h"
    "${PROJECT_SOURCE_DIR}/src/timer/TimerBase.cpp"
    )

SET(KERNEL_LOGGER
    "${PROJECT_SOURCE_DIR}/src/logger/Logger.h"
    "${PROJECT_SOURCE_DIR}/src/logger/Logger.cpp"
    )

SET(KERNEL_LOGIC
    "${PROJECT_SOURCE_DIR}/src/logic/LogicMgr.h"
    "${PROJECT_SOURCE_DIR}/src/logic/LogicMgr.cpp"
    )

SET(KERNEL_PROFILE
    "${PROJECT_SOURCE_DIR}/src/profile/Profile.h"
    "${PROJECT_SOURCE_DIR}/src/profile/Profile.cpp"
    )
	
SET(KERNEL_ID
    "${PROJECT_SOURCE_DIR}/src/id/Id.h"
    "${PROJECT_SOURCE_DIR}/src/id/Id.cpp"
    )

SET(KERNEL
    "${PROJECT_SOURCE_DIR}/src/kernel.h"
    "${PROJECT_SOURCE_DIR}/src/kernel.cpp"
    "${PROJECT_SOURCE_DIR}/src/main.cpp"
)

SOURCE_GROUP(config FILES ${KERNEL_CONFIG})
SOURCE_GROUP(net\\iocp FILES ${KERNEL_NET})
SOURCE_GROUP(logger FILES ${KERNEL_LOGGER})
SOURCE_GROUP(logic FILES ${KERNEL_LOGIC})
SOURCE_GROUP(timer FILES ${KERNEL_TIMER})
SOURCE_GROUP(async FILES ${KERNEL_ASYNC})
source_group(profile FILES ${KERNEL_PROFILE})
source_group(id FILES ${KERNEL_ID})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(\\ FILES ${KERNEL})

ADD_EXECUTABLE(serverd
	${API}
	${KERNEL_CONFIG}
	${KERNEL_NET}
	${KERNEL_TIMER}
	${KERNEL_LOGIC}
	${KERNEL_LOGGER}
	${KERNEL_ASYNC}
	${KERNEL_PROFILE}
	${KERNEL_ID}
	${PUBLIC}
	${PUBLIC_XML}
	${KERNEL}
)

SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/../bin")

IF(WIN32)
STRING(REPLACE "/" "\\" PROJECT_SOURCE_WIN_DIR ${PROJECT_SOURCE_DIR})
STRING(REPLACE "/" "\\" EXECUTABLE_OUTPUT_PATH_WIN ${EXECUTABLE_OUTPUT_PATH})

ADD_CUSTOM_COMMAND(TARGET serverd 
	POST_BUILD 
	COMMAND copy /y ${PROJECT_SOURCE_WIN_DIR}\\API\\*.h ${PROJECT_SOURCE_WIN_DIR}\\..\\core\\API
	COMMAND copy /y ${PROJECT_SOURCE_WIN_DIR}\\public\\*.h ${PROJECT_SOURCE_WIN_DIR}\\..\\core\\public
	COMMAND copy /y ${PROJECT_SOURCE_WIN_DIR}\\API\\*.h ${PROJECT_SOURCE_WIN_DIR}\\..\\framework\\API
	COMMAND copy /y ${PROJECT_SOURCE_WIN_DIR}\\public\\*.h ${PROJECT_SOURCE_WIN_DIR}\\..\\framework\\public
	COMMAND copy /y ${PROJECT_SOURCE_WIN_DIR}\\API\\*.h ${PROJECT_SOURCE_WIN_DIR}\\..\\logic\\API
	COMMAND copy /y ${PROJECT_SOURCE_WIN_DIR}\\public\\*.h ${PROJECT_SOURCE_WIN_DIR}\\..\\logic\\public
	COMMAND copy /y ${EXECUTABLE_OUTPUT_PATH_WIN}\\$<CONFIGURATION>\\serverd.lib ${PROJECT_SOURCE_WIN_DIR}\\..\\core\\thirdlibary\\bin\\$<CONFIGURATION>
	COMMAND copy /y ${EXECUTABLE_OUTPUT_PATH_WIN}\\$<CONFIGURATION>\\serverd.lib ${PROJECT_SOURCE_WIN_DIR}\\..\\framework\\thirdlibary\\bin\\$<CONFIGURATION>
	COMMAND copy /y ${EXECUTABLE_OUTPUT_PATH_WIN}\\$<CONFIGURATION>\\serverd.lib ${PROJECT_SOURCE_WIN_DIR}\\..\\logic\\thirdlibary\\bin\\$<CONFIGURATION>
)
ENDIF(WIN32)

IF(UNIX)
ADD_CUSTOM_COMMAND(TARGET serverd 
	POST_BUILD 
	COMMAND cp -f ${PROJECT_SOURCE_DIR}/API/*.h ${PROJECT_SOURCE_DIR}/../core/API
	COMMAND cp -f ${PROJECT_SOURCE_DIR}/public/*.h ${PROJECT_SOURCE_DIR}/../core/public
	COMMAND cp -f ${PROJECT_SOURCE_DIR}/API/*.h ${PROJECT_SOURCE_DIR}/../framework/API
	COMMAND cp -f ${PROJECT_SOURCE_DIR}/public/*.h ${PROJECT_SOURCE_DIR}/../framework/public
	COMMAND cp -f ${PROJECT_SOURCE_DIR}/API/*.h ${PROJECT_SOURCE_DIR}/../logic/API
	COMMAND cp -f ${PROJECT_SOURCE_DIR}/public/*.h ${PROJECT_SOURCE_DIR}/../logic/public
)
ENDIF(UNIX)
