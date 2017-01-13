#include "SceneClient.h"
#include "IHarbor.h"
#include "IEventEngine.h"
#include "UserNodeType.h"

bool SceneClient::Initialize(IKernel * kernel) {
    _kernel = kernel;

    return true;
}

bool SceneClient::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);
	if (_harbor->GetNodeType() == user_node_type::LOGIC) {
		FIND_MODULE(_event, EventEngine);


	}
    return true;
}

bool SceneClient::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

s64 SceneClient::RegisterArea(s8 type, const char * scene, s16 x, s16 y, s16 z, s16 range, const AreaCallBack& f) {

}

void SceneClient::SwitchTo(IObject * object, const char * scene, const Position& pos, const s64 copyId = 0) {

}

void SceneClient::SwitchTo(IObject * object, IObject * scene, const Position& pos) {

}

std::vector<Position> SceneClient::FindPath(IObject * scene, const Position& start, const Position& end, float radius = 0) {

}

Position SceneClient::RandomInRange(IObject * scene, const Position& start, const Position& end, float radius) {

}

Position SceneClient::RayCast(IObject * scene, const Position& start, const Position& end, float radius) {

}

s32 SceneClient::GetAreaType(IObject * object) {

}


