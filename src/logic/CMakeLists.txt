SET(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")
SET(LOGIC_INTERFACE
    "${PROJECT_SOURCE_DIR}/src/logic/interface/IHarbor.h"
    "${PROJECT_SOURCE_DIR}/src/logic/interface/Define.h"
    "${PROJECT_SOURCE_DIR}/src/logic/interface/NodeProtocol.h"
    "${PROJECT_SOURCE_DIR}/src/logic/interface/NodeType.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/UserNodeProtocol.h"
    "${PROJECT_SOURCE_DIR}/src/logic/interface/UserNodeType.h"

    "${PROJECT_SOURCE_DIR}/src/logic/interface/IMaster.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/IScriptEngine.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/ITest.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/IStarter.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/ISlave.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/IStartNodeStrategy.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/ICapacityPublisher.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/ICapacitySubscriber.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/IAnalysis.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/IHttpMgr.h"
	"${PROJECT_SOURCE_DIR}/src/logic/interface/IObjectMgr.h"
)

ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/harbor)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/cluster)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/master)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/starter)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/scriptengine)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/redis)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/rpc)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/agent)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/test)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/slave)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/startnodestrategy)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/capacitypublisher)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/capacitysubscriber)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/analysis)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/logic/httpmgr)

