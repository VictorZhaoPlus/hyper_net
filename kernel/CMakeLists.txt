#cmake file for project kernel
#author:ooeyusea
#created:2015-07-08 09:38:00

cmake_minimum_required(VERSION 2.8)
PROJECT(kernel)
IF(UNIX)
    SET(CMAKE_CXX_COMPILER "/usr/bin/g++")
    SET(CMAKE_CXX_FLAGS "-Wall -std=c++0x -lpthread -lcurl -rdynamic -ldl -Dlinux -Wl,-rpath,./")
    SET(CMAKE_CXX_FLAGS_DEBUG "-g -D_DEBUG -O0 -pg")
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O2")
	SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
    MESSAGE(STATUS "Linux")
ENDIF(UNIX)

IF(WIN32)
    SET(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /D_WINSOCK_DEPRECATED_NO_WARNINGS /Zm200 /MTd /Zi /Ob0 /Od /RTC1") 
    SET(CMAKE_CXX_FLAGS_MINSIZEREL "/D_WINSOCK_DEPRECATED_NO_WARNINGS /MT /Zm200 /O1 /Ob1 /D NDEBUG") 
    SET(CMAKE_CXX_FLAGS_RELEASE "/D_WINSOCK_DEPRECATED_NO_WARNINGS /MT /Zm200 /O2 /Ob2 /D NDEBUG") 
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/D_WINSOCK_DEPRECATED_NO_WARNINGS /MT /Zm200 /Zi /O2 /Ob1 /D NDEBUG")
    MESSAGE(STATUS "WINDOWS")
    IF(CMAKE_CL_64)
        MESSAGE(STATUS "x64")
    ELSE()
        MESSAGE(STATUS "x86")
    ENDIF(CMAKE_CL_64)
	SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS On)
	ADD_DEFINITIONS(-DKERNEL_EXPORT)
ENDIF(WIN32)

SET(API
    "${PROJECT_SOURCE_DIR}/API/IKernel.h"
    "${PROJECT_SOURCE_DIR}/API/IModule.h"
    )

SET(PUBLIC
    "${PROJECT_SOURCE_DIR}/public/util.h"
    "${PROJECT_SOURCE_DIR}/public/util.cpp"
    "${PROJECT_SOURCE_DIR}/public/tools.h"
	"${PROJECT_SOURCE_DIR}/public/tools.cpp"
    "${PROJECT_SOURCE_DIR}/public/singleton.h"
	"${PROJECT_SOURCE_DIR}/public/XmlReader.h"
	"${PROJECT_SOURCE_DIR}/public/XmlReader.cpp"
	"${PROJECT_SOURCE_DIR}/public/CycleQueue.h"
	"${PROJECT_SOURCE_DIR}/public/OMap.h"
	"${PROJECT_SOURCE_DIR}/public/Fsm.h"
	"${PROJECT_SOURCE_DIR}/public/OPool.h"
	"${PROJECT_SOURCE_DIR}/public/OArgs.h"
	"${PROJECT_SOURCE_DIR}/public/OBuffer.h"
	"${PROJECT_SOURCE_DIR}/public/ORingBuffer.h"
	"${PROJECT_SOURCE_DIR}/public/ORingBuffer.c"
	"${PROJECT_SOURCE_DIR}/public/base64.h"
	"${PROJECT_SOURCE_DIR}/public/base64.c"
	"${PROJECT_SOURCE_DIR}/public/spin_mutex.h"
	"${PROJECT_SOURCE_DIR}/public/Md5.h"
	"${PROJECT_SOURCE_DIR}/public/Md5.cpp"
    )

SET(PUBLIC_XML
    "${PROJECT_SOURCE_DIR}/public/tinyxml/tinystr.cpp"
    "${PROJECT_SOURCE_DIR}/public/tinyxml/tinystr.h"
    "${PROJECT_SOURCE_DIR}/public/tinyxml/tinyxml.cpp"
    "${PROJECT_SOURCE_DIR}/public/tinyxml/tinyxmlerror.cpp"
    "${PROJECT_SOURCE_DIR}/public/tinyxml/tinyxml.h"
    "${PROJECT_SOURCE_DIR}/public/tinyxml/tinyxmlparser.cpp"
    )

SET(JIT OFF)
	
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src)