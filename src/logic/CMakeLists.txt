SET(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")
SET(LOGIC_INTERFACE
    "${PROJECT_SOURCE_DIR}/src/logic/interface/IHarbor.h"
    "${PROJECT_SOURCE_DIR}/src/logic/interface/Define.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/NodeType.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/UserNodeType.h"
    "${PROJECT_SOURCE_DIR}/src/logic/interface/CoreProtocol.h"
    "${PROJECT_SOURCE_DIR}/src/logic/interface/FrameworkProtocol.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/LogicProtocol.h"
   

    "${PROJECT_SOURCE_DIR}/src/logic/interface/ICluster.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/IScriptEngine.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/ITest.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/IAgent.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/IRedis.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/IMonitor.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/IAnalysis.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/IHttpMgr.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/IIdMgr.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/IObjectMgr.h"
)

ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/harbor)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/cluster)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/scriptengine)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/redis)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/luaserver)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/agent)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/test)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/startnodestrategy)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/monitor)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/analysis)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/httpmgr)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/idmgr)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/objectmgr)
