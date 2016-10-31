
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/API"
    "${PROJECT_SOURCE_DIR}/src/public"
    "${PROJECT_SOURCE_DIR}/src/public/tinyxml"
    "${PROJECT_SOURCE_DIR}/src/logic/interface"
	"${PROJECT_SOURCE_DIR}/src/logic/redis/hiredis"
    )
	
SET(HIREDIS
	"${PROJECT_SOURCE_DIR}/src/logic/redis/hiredis/dict.c"
	"${PROJECT_SOURCE_DIR}/src/logic/redis/hiredis/dict.h"
	"${PROJECT_SOURCE_DIR}/src/logic/redis/hiredis/fmacros.h"
	"${PROJECT_SOURCE_DIR}/src/logic/redis/hiredis/hiredis.c"
	"${PROJECT_SOURCE_DIR}/src/logic/redis/hiredis/hiredis.h"
	"${PROJECT_SOURCE_DIR}/src/logic/redis/hiredis/net.c"
	"${PROJECT_SOURCE_DIR}/src/logic/redis/hiredis/net.h"
	"${PROJECT_SOURCE_DIR}/src/logic/redis/hiredis/read.c"
	"${PROJECT_SOURCE_DIR}/src/logic/redis/hiredis/read.h"
	"${PROJECT_SOURCE_DIR}/src/logic/redis/hiredis/sds.c"
	"${PROJECT_SOURCE_DIR}/src/logic/redis/hiredis/sds.h"
	"${PROJECT_SOURCE_DIR}/src/logic/redis/hiredis/sdsalloc.h"
	)

SET(REDIS
    "${PROJECT_SOURCE_DIR}/src/logic/redis/Redis.h"
    "${PROJECT_SOURCE_DIR}/src/logic/redis/Redis.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/redis/RedisCallResult.h"
    "${PROJECT_SOURCE_DIR}/src/logic/redis/RedisCallResult.cpp"
    "${PROJECT_SOURCE_DIR}/src/logic/redis/main.cpp"
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

SET_TARGET_PROPERTIES(Redis PROPERTIES FOLDER "core") 