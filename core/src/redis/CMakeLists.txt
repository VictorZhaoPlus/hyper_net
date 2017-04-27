IF(UNIX)
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/interface"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis"
    )
	

SET(HIREDIS
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis/dict.c"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis/dict.h"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis/fmacros.h"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis/hiredis.c"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis/hiredis.h"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis/net.c"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis/net.h"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis/sds.c"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis/sds.h"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis/sdsalloc.h"
	)
ENDIF(UNIX)

IF(WIN32)
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/interface"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis_win32"
    )
	
SET(HIREDIS
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis_win32/dict.c"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis_win32/dict.h"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis_win32/fmacros.h"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis_win32/hiredis.c"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis_win32/hiredis.h"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis_win32/net.c"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis_win32/net.h"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis_win32/sds.c"
	"${PROJECT_SOURCE_DIR}/src/redis/hiredis_win32/sds.h"
	)
ENDIF(WIN32)


SET(REDIS
    "${PROJECT_SOURCE_DIR}/src/redis/Redis.h"
    "${PROJECT_SOURCE_DIR}/src/redis/Redis.cpp"
    "${PROJECT_SOURCE_DIR}/src/redis/RedisCallResult.h"
    "${PROJECT_SOURCE_DIR}/src/redis/RedisCallResult.cpp"
    "${PROJECT_SOURCE_DIR}/src/redis/main.cpp"
    )

SOURCE_GROUP(interface FILES ${LOGIC_INTERFACE})
SOURCE_GROUP(API FILES ${API})
SOURCE_GROUP(public\\tinyxml FILES ${PUBLIC_XML})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(hiredis FILES ${HIREDIS})
SOURCE_GROUP(\\ FILES ${REDIS})

ADD_LIBRARY(Redis SHARED
        ${REDIS}
		${HIREDIS}
        ${API}
        ${LOGIC_INTERFACE}
        ${PUBLIC}
        ${PUBLIC_XML}
        )

IF(WIN32)
TARGET_LINK_LIBRARIES(Redis
	"${PROJECT_SOURCE_DIR}/thirdlibary/bin/$<CONFIGURATION>/serverd.lib"
)
ENDIF(WIN32)

SET_TARGET_PROPERTIES(Redis PROPERTIES FOLDER "core")