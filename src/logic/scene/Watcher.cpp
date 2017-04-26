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
#define MAX_ROLE_NOTIFY_PACKET_SIZE 8192

enum InterestTable {
	IT_COL_ID = 0,
};

enum WatcherTable {
	WT_COL_ID = 0,
	WT_COL_GATE,
	WT_COL_LOGIC,
};

bool Watcher::Initialize(IKernel * kernel) {
    _kernel = kernel;
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

		FIND_MODULE(_packetSender, PacketSender);
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

void Watcher::QueryNeighbor(IObject * object, const s32 cmd, const OArgs& args, const std::function<void(IKernel*, IObject * object, const ITargetSet * targets)>& cb) {

}


void Watcher::DealInterest(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args) {
	s64 id = 0;
	args.Read(id);

	IObject * reciever = _objectMgr->FindObject(id);
	if (reciever) {
		ITableControl * interest = reciever->FindTable(_tableInterest);
		OASSERT(interest, "wtf");
		
		s16 count = 0;
		args.Read(count);
		for (s32 i = 0; i < count; ++i) {
			s64 interestId;
			args.Read(interestId);
			interest->AddRowKeyInt64(id);
		}

		args.Read(count);
		for (s32 i = 0; i < count; ++i) {
			s64 interestId;
			args.Read(interestId);
			IRow * row = interest->FindRow(id);
			DEL_TABLE_ROW(interest, row);
		}
	}
}

void Watcher::DealWatcher(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args) {
	s64 id = 0;
	args.Read(id);

	IObject * reciever = _objectMgr->FindObject(id);
	if (reciever) {
		ITableControl * watcher = reciever->FindTable(_tableWatcher);
		OASSERT(watcher, "wtf");

		s16 count = 0;
		args.Read(count);
		if (count > 0) {
			std::unordered_map<s32, std::vector<s64>> actors;
			for (s32 i = 0; i < count; ++i) {
				s64 watcherId;
				s32 gate;
				s32 logic;
				args.ReadMulti(watcherId, gate, logic);

				IRow * row = watcher->AddRowKeyInt64(id);
				row->SetDataInt32(WatcherTable::WT_COL_GATE, gate);
				row->SetDataInt32(WatcherTable::WT_COL_LOGIC, logic);

				if (gate > 0)
					actors[gate].push_back(watcherId);
			}
			if (!actors.empty()) {
				olib::Buffer<MAX_ROLE_NOTIFY_PACKET_SIZE> buf;
				buf << reciever->GetID() << reciever->GetPropInt32(_type);
				s16 * propCount = buf.Reserve<s16>();
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
						++(*propCount);
					}
				}

				_packetSender->Brocast(actors, _appearId, buf.Out());
			}
		}

		args.Read(count);
		if (count > 0) {
			std::unordered_map<s32, std::vector<s64>> actors;
			for (s32 i = 0; i < count; ++i) {
				s64 watcherId;
				args.Read(watcherId);

				IRow * row = watcher->FindRow(id);
				s32 gate = row->GetDataInt32(WatcherTable::WT_COL_GATE);
				if (gate > 0)
					actors[gate].push_back(watcherId);

				DEL_TABLE_ROW(watcher, row);
			}

			olib::Buffer<64> buf;
			buf << reciever->GetID();

			_packetSender->Brocast(actors, _disappearId, buf.Out());
		}
	}
}

void Watcher::DisapperWhenDestroy(IKernel * kernel, const void * context, const s32 size) {
	OASSERT(size == sizeof(IObject*), "wtf");
	IObject * object = *(IObject**)context;

	olib::Buffer<64> buf;
	buf << object->GetID();

	Brocast(object, _disappearId, buf.Out());
}
