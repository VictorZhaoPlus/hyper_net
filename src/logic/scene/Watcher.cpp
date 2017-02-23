#include "Watcher.h"
#include "IHarbor.h"
#include "IEventEngine.h"
#include "IProtocolMgr.h"
#include "ILogin.h"
#include "UserNodeType.h"
#include "IOCommand.h"
#include "IObjectMgr.h"
#include "IShadowMgr.h"
#include "OArgs.h"
#include "OBuffer.h"

enum InterestTable {
	IT_COL_ID = 0,
	IT_COL_WATCH,
};

enum WatcherTable {
	WT_COL_ID = 0,
	WT_COL_GATE,
	WT_COL_LOGIC,
};

bool Watcher::Initialize(IKernel * kernel) {
    _kernel = kernel;

	SetSelector(this);
    return true;
}

bool Watcher::Launched(IKernel * kernel) {
	FIND_MODULE(_harbor, Harbor);
	if (_harbor->GetNodeType() == user_node_type::LOGIC) {
		FIND_MODULE(_objectMgr, ObjectMgr);
		_gate = _objectMgr->CalcProp("gate");
		_logic = _objectMgr->CalcProp("logic");
		_type = _objectMgr->CalcProp("type");

		_tableInterest = _objectMgr->CalcTableName("interest");
		_tableWatcher = _objectMgr->CalcTableName("watcher");

		_settingShare = _objectMgr->CalcPropSetting("share");

		FIND_MODULE(_protocolMgr, ProtocolMgr);


		_eventSceneObjectDestroy = _protocolMgr->GetId("event", "scene_object_destroy");

		_appearId = _protocolMgr->GetId("proto", "role_appear");
		_disappearId = _protocolMgr->GetId("proto", "role_disappear");

		FIND_MODULE(_eventEngine, EventEngine);
		RGS_EVENT_HANDLER(_eventEngine, _eventSceneObjectDestroy, Watcher::DisapperWhenDestroy);
		RGS_EVENT_HANDLER(_eventEngine, _eventSceneObjectDestroy, Watcher::RemoveAllInterestWhenDestroy);

		FIND_MODULE(_packetSender, PacketSender);

		FIND_MODULE(_command, OCommand);
		RGS_COMMAND_CB(_commandAddInterest, Watcher::AddInterest);
		RGS_COMMAND_CB(_commandRemoveInterest, Watcher::RemoveInterest);
		RGS_COMMAND_CB(_commandAddWatcher, Watcher::AddWatcher);
		RGS_COMMAND_CB(_commandRemoveWatcher, Watcher::RemoveWatcher);

		FIND_MODULE(_shadowMgr, ShadowMgr);
	}

    return true;
}

bool Watcher::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void Watcher::Brocast(IObject * object, const s32 msgId, const OBuffer& buf, bool self) {
	ITableControl * watcher = object->FindTable(_tableWatcher);
	OASSERT(watcher, "wtf");

	std::unordered_map<s32, std::vector<s64>> actors;
	if (self) {
		s32 gate = object->GetPropInt32(_gate);
		OASSERT(gate != 0, "wtf");
		if (gate != 0)
			actors[gate].push_back(object->GetID());
	}

	for (s32 i = 0; i < watcher->RowCount(); ++i) {
		IRow * row = watcher->GetRow(i);
		OASSERT(row, "wtf");
		
		s64 id = row->GetDataInt64(WatcherTable::WT_COL_ID);
		s32 gate = row->GetDataInt32(WatcherTable::WT_COL_GATE);
		if (gate != 0)
			actors[gate].push_back(id);
	}

	_packetSender->Brocast(actors, msgId, buf);
}

void Watcher::QueryNeighbor(IObject * object, const std::function<void(IKernel*, IObject * object)>& f) {
	ITableControl * interest = object->FindTable(_tableInterest);
	OASSERT(interest, "wtf");

	for (s32 i = 0; i < interest->RowCount(); ++i) {
		IRow * row = interest->GetRow(i);
		OASSERT(row, "wtf");

		if (row->GetDataInt8(InterestTable::IT_COL_WATCH)) {
			s64 id = row->GetDataInt64(InterestTable::IT_COL_ID);
			IObject * neighbor = _objectMgr->FindObject(id);
			if (neighbor)
				f(_kernel, neighbor);
		}
	}
}

bool Watcher::IsNeighbor(IObject * object, s64 id) {
	ITableControl * interest = object->FindTable(_tableInterest);
	OASSERT(interest, "wtf");

	IRow * row = interest->FindRow(id);
	if (!row)
		return false;

	return row->GetDataInt8(InterestTable::IT_COL_WATCH) != 0;
}

s32 Watcher::Check(IObject * object, s64 id, s32 type, s64& eliminateId) {
	return WSCR_ADD;
}

s64 Watcher::Pop(IObject * object, s64 id, s32 type) {
	return 0;
}

void Watcher::AddInterest(IKernel * kernel, const s64 sender, IObject * reciever, const OArgs& args) {
	ITableControl * interest = reciever->FindTable(_tableInterest);
	OASSERT(interest, "wtf");

	s64 id = args.GetDataInt64(0);
	s32 type = args.GetDataInt32(1);

	IRow * row = interest->FindRow(id);
	if (row)
		return;

	row = interest->AddRowKeyInt64(id);
	OASSERT(row, "wtf");

	s64 eliminateId = 0;
	s32 ret = _selector->Check(reciever, id, type, eliminateId);
	switch (ret) {
	case WSCR_REPLACE: {
			IArgs<1, 64> cmd;
			cmd << reciever->GetPropInt32(_gate);
			cmd.Fix();
			_command->Command(_commandRemoveWatcher, reciever, eliminateId, cmd.Out());

			IRow * replace = interest->FindRow(eliminateId);
			OASSERT(replace, "wtf");
			replace->SetDataInt8(InterestTable::IT_COL_WATCH, 0);
		} // purpos no break
	case WSCR_ADD: {
			IArgs<2, 64> cmd;
			cmd << reciever->GetPropInt32(_gate) << reciever->GetPropInt32(_logic);
			cmd.Fix();
			_command->Command(_commandAddWatcher, reciever, id, cmd.Out());
			row->SetDataInt8(InterestTable::IT_COL_WATCH, 1);
		}
		break;
	default: break;
	}
}

void Watcher::RemoveInterest(IKernel * kernel, const s64 sender, IObject * reciever, const OArgs& args) {
	ITableControl * interest = reciever->FindTable(_tableInterest);
	OASSERT(interest, "wtf");

	s64 id = args.GetDataInt64(0);
	s32 type = args.GetDataInt32(1);

	IRow * row = interest->FindRow(id);
	if (!row)
		return;

	if (row->GetDataInt8(InterestTable::IT_COL_WATCH)) {
		IArgs<1, 64> cmd;
		cmd << reciever->GetPropInt32(_gate);
		cmd.Fix();
		_command->Command(_commandRemoveWatcher, reciever, id, cmd.Out());
	}
	DEL_TABLE_ROW(interest, row);

	s64 addId = _selector->Pop(reciever, id, type);
	if (addId != 0) {
		IArgs<2, 64> cmd;
		cmd << reciever->GetPropInt32(_gate) << reciever->GetPropInt32(_logic);
		cmd.Fix();
		_command->Command(_commandAddWatcher, reciever, addId, cmd.Out());

		IRow * add = interest->FindRow(addId);
		OASSERT(add, "wtf");
		add->SetDataInt8(InterestTable::IT_COL_WATCH, 1);
	}
}

void Watcher::AddWatcher(IKernel * kernel, const s64 sender, IObject * reciever, const OArgs& args) {
	s32 gate = args.GetDataInt32(0);
	s32 logic = args.GetDataInt32(1);

	ITableControl * watcher = reciever->FindTable(_tableWatcher);
	OASSERT(watcher, "wtf");

	IRow * row = watcher->FindRow(sender);
	OASSERT(!row, "wtf");
	if (row)
		return;

	row = watcher->AddRowKeyInt64(sender);
	row->SetDataInt32(WatcherTable::WT_COL_GATE, gate);
	row->SetDataInt32(WatcherTable::WT_COL_LOGIC, logic);

	_shadowMgr->Project(reciever, logic);

	olib::Buffer<8196> buf;
	buf << reciever->GetID() << reciever->GetPropInt32(_type);
	s16 * count = buf.Reserve<s16>();
	for (auto * prop : reciever->GetPropsInfo()) {
		if (prop->GetSetting(reciever) & _settingShare) {
			buf << prop->GetName();
			switch (prop->GetType(reciever)) {
			case DTYPE_INT8: buf << reciever->GetPropInt8(prop); break;
			case DTYPE_INT16: buf << reciever->GetPropInt16(prop); break;
			case DTYPE_INT32: buf << reciever->GetPropInt32(prop); break;
			case DTYPE_INT64: buf << reciever->GetPropInt64(prop); break;
			case DTYPE_FLOAT: buf << reciever->GetPropFloat(prop); break;
			case DTYPE_STRING: buf << reciever->GetPropString(prop); break;
			case DTYPE_BLOB: {
				s32 size = 0;
				const void * p = reciever->GetPropBlob(prop, size);
				buf.WriteBuffer(p, size);
			}
							 break;
			default: OASSERT(false, "wtf"); break;
			}
			++(*count);
		}
	}

	_packetSender->Send(gate, sender, _appearId, buf.Out());
}

void Watcher::RemoveWatcher(IKernel * kernel, const s64 sender, IObject * reciever, const OArgs& args) {
	ITableControl * watcher = reciever->FindTable(_tableWatcher);
	OASSERT(watcher, "wtf");

	IRow * row = watcher->FindRow(sender);
	OASSERT(row, "wtf");
	if (!row)
		return;

	s32 gate = row->GetDataInt32(WatcherTable::WT_COL_GATE);
	s32 logic = row->GetDataInt32(WatcherTable::WT_COL_LOGIC);

	DEL_TABLE_ROW(watcher, row);

	_shadowMgr->Unproject(reciever, logic);

	olib::Buffer<64> buf;
	buf << reciever->GetID();

	_packetSender->Send(gate, sender, _disappearId, buf.Out());
}

void Watcher::DisapperWhenDestroy(IKernel * kernel, const void * context, const s32 size) {
	OASSERT(size == sizeof(IObject*), "wtf");
	IObject * object = *(IObject**)context;

	olib::Buffer<64> buf;
	buf << object->GetID();

	Brocast(object, _disappearId, buf.Out());
}

void Watcher::RemoveAllInterestWhenDestroy(IKernel * kernel, const void * context, const s32 size) {
	OASSERT(size == sizeof(IObject*), "wtf");
	IObject * object = *(IObject**)context;

	ITableControl * interest = object->FindTable(_tableInterest);
	OASSERT(interest, "wtf");

	for (s32 i = 0; i < interest->RowCount(); ++i) {
		IRow * row = interest->GetRow(i);
		OASSERT(row, "wtf");

		if (row->GetDataInt8(InterestTable::IT_COL_WATCH)) {
			s64 id = row->GetDataInt64(InterestTable::IT_COL_ID);

			IArgs<1, 64> cmd;
			cmd << object->GetPropInt32(_gate);
			cmd.Fix();
			_command->Command(_commandRemoveWatcher, object, id, cmd.Out());
		}
	}
}
