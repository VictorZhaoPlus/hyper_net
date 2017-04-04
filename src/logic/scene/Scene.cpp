#include "Cell.h"
#include "IHarbor.h"
#include "UserNodeType.h"
#include "FrameworkProtocol.h"
#include "CellInterface.h"
#include "OArgs.h"
#include "OBuffer.h"
#include "IObjectMgr.h"

bool Cell::Initialize(IKernel * kernel) {
    _kernel = kernel;
	_cellVisibleChecker = this;

    return true;
}

bool Cell::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);
	if (_harbor->GetNodeType() == user_node_type::CELL) {
		FIND_MODULE(_objectMgr, ObjectMgr);
		FIND_MODULE(_capacityPublisher, CapacityPublisher);

		RGS_HABOR_ARGS_HANDLER(framework_proto::CREATE_CELL, Cell::CreateCell);
		RGS_HABOR_ARGS_HANDLER(framework_proto::RECOVER_CELL, Cell::RecoverCell);
		RGS_HABOR_HANDLER(framework_proto::ENTER_CELL, Cell::EnterCell);
		RGS_HABOR_ARGS_HANDLER(framework_proto::LEAVE_CELL, Cell::LeaveCell);
		RGS_HABOR_HANDLER(framework_proto::UPDATE_CELL, Cell::UpdateCell);

		_propId = _objectMgr->CalcProp("objectId");
	}
    return true;
}

bool Cell::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void Cell::CreateCell(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args) {
	s64 id = args.GetDataInt64(0);

	OASSERT(_cells.find(id) == _cells.end(), "wtf");

	_cells[id] = NEW CellInterface(id);
	START_TIMER(_cells[id], 0, TIMER_BEAT_FOREVER, _updateInterval);
}

void Cell::RecoverCell(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args) {
	s64 id = args.GetDataInt64(0);

	auto itr = _cells.find(id);
	OASSERT(itr != _cells.end(), "wtf");
	if (itr != _cells.end()) {
		kernel->KillTimer(itr->second);
		DEL itr->second;
		_cells.erase(itr);
	}
}

void Cell::EnterCell(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args) {
	s64 cellId = 0;
	if (!args.Read(cellId)) {
		OASSERT(false, "wtf");
		return;
	}

	bool update = false;
	if (!args.Read(update)) {
		OASSERT(false, "wtf");
		return;
	}

	auto itr = _cells.find(cellId);
	OASSERT(itr != _cells.end(), "wtf");
	if (itr != _cells.end()) {
		s64 id = 0;
		if (!args.Read(id)) {
			OASSERT(false, "wtf");
			return;
		}

		IObject * object = CREATE_OBJECT(_objectMgr, "CellObject");
		OASSERT(object, "wtf");
		if (object) {
			object->SetPropInt64(_propId, id);
			ReadProps(kernel, object, args);

			itr->second->Add(id, object, update);
		}
	}
}

void Cell::LeaveCell(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs& args) {
	s64 cellId = args.GetDataInt64(0);
	bool update = args.GetDataBool(1);
	s64 id = args.GetDataInt64(2);

	auto itr = _cells.find(cellId);
	OASSERT(itr != _cells.end(), "wtf");
	if (itr != _cells.end())
		itr->second->Remove(id, update);
}

void Cell::UpdateCell(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args) {
	s64 cellId = 0;
	if (!args.Read(cellId)) {
		OASSERT(false, "wtf");
		return;
	}

	auto itr = _cells.find(cellId);
	OASSERT(itr != _cells.end(), "wtf");
	if (itr != _cells.end()) {
		s64 id = 0;
		if (!args.Read(id)) {
			OASSERT(false, "wtf");
			return;
		}

		IObject * object = itr->second->Find(id);
		OASSERT(object, "wtf");
		if (object) {
			ReadProps(kernel, object, args);

			itr->second->Update(id, object);
		}
	}
}

void Cell::ReadProps(IKernel * kernel, IObject * object, const OBuffer& buf) {

}

