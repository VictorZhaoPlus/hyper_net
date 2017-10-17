#include "Watcher.h"
#include "IHarbor.h"
#include "IEventEngine.h"
#include "ILogin.h"
#include "IOCommand.h"
#include "IObjectMgr.h"
#include "OArgs.h"
#include "OBuffer.h"

#define MAX_ROLE_NOTIFY_PACKET_SIZE 8192
#define MAX_QUERY_PACKET 8192
#define SINGLE_PATCH 10

void Watcher::RuningQuery::Awake(IKernel * kernel, s64 target, s8 batch) {
	if (batch != 0)
		_ret.insert({ batch, target });
	_targets.erase(target);

	if (_targets.empty()) {
		IObject * object = OMODULE(ObjectMgr)->FindObject(_objectId);
		if (object)
			_cb(kernel, object, _ret);

		kernel->KillTimer(this);
	}
}

void Watcher::RuningQuery::OnEnd(IKernel * kernel, bool nonviolent, s64 tick) {
	if (nonviolent) {
		IObject * object = OMODULE(ObjectMgr)->FindObject(_objectId);
		if (object)
			_cb(kernel, object, _ret);
	}

	Watcher::Instance()->FinishQuery(_queryId);
	DEL this;
}

bool Watcher::Initialize(IKernel * kernel) {
    _kernel = kernel;

	_nextQueryId = 0;
    return true;
}

bool Watcher::Launched(IKernel * kernel) {
	if (OMODULE(Harbor)->GetNodeType() == OID("node_type", "logic")) {
		RGS_HABOR_HANDLER(OID("scene", "deal_interest"), Watcher::DealInterest);
		RGS_HABOR_HANDLER(OID("scene", "deal_watcher"), Watcher::DealWatcher);
		RGS_HABOR_HANDLER(OID("scene", "query"), Watcher::Query);
		RGS_HABOR_HANDLER(OID("scene", "query_ack"), Watcher::QueryAck);


		RGS_EVENT_HANDLER(OID("evt_scene", "scene_object_destroy"), Watcher::DisapperWhenDestroy);
	}

    return true;
}

bool Watcher::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void Watcher::Brocast(IObject * object, const s32 msgId, const OBuffer& buf, bool self) {
	ITableControl * watcher = object->FindTable(OTABLE("watcher"));
	OASSERT(watcher, "wtf");

	std::unordered_map<s32, std::vector<s64>> actors;
	if (self) {
		s32 gate = object->GetPropInt32(OPROP("gate"));
		OASSERT(gate != 0, "wtf");
		if (gate != 0)
			actors[gate].push_back(object->GetID());
	}

	for (s32 i = 0; i < watcher->RowCount(); ++i) {
		IRow * row = watcher->GetRow(i);
		OASSERT(row, "wtf");
		
		s64 id = row->GetDataInt64(OCOLUMN("watcher", "id"));
		s32 gate = row->GetDataInt32(OCOLUMN("watcher", "gate"));
		if (gate != 0)
			actors[gate].push_back(id);
	}

	OMODULE(PacketSender)->Brocast(actors, msgId, buf);
}

s64 Watcher::QueryInVision(IObject * object, const s32 type, const void * context, const s32 size, s32 wait, const QueryCallback& cb) {
	ITableControl * interest = object->FindTable(OTABLE("interest"));
	OASSERT(interest, "wtf");

	RuningQuery * query = NEW RuningQuery(_nextQueryId++, object->GetID(), cb);
	std::map<s32, std::vector<s64>> ids;
	for (s32 i = 0; i < interest->RowCount(); ++i) {
		IRow * row = interest->GetRow(i);
		OASSERT(row, "wtf");

		ids[row->GetDataInt32(OCOLUMN("interest", "logic"))].push_back(row->GetDataInt64(OCOLUMN("interest", "id")));
		query->Wait(row->GetDataInt64(OCOLUMN("interest", "id")));
	}

	olib::Buffer<MAX_QUERY_PACKET> buf;
	buf << query->GetQueryId() << type;
	buf.WriteBuffer(context, size);
	auto mark = buf.Mark();
	
	for (auto itr = ids.begin(); itr != ids.end(); ++itr) {
		buf.Back(mark);

		s32 count = 0;
		for (auto id : itr->second) {
			buf << id;
			++count;
			if (count >= SINGLE_PATCH) {
				count = 0;
				OMODULE(Harbor)->Send(OID("node_type", "logic"), itr->first, OID("scene", "query"), buf.Out());

				buf.Back(mark);
			}
		}
		
		if (count > 0)
			OMODULE(Harbor)->Send(OID("node_type", "logic"), itr->first, OID("scene", "query"), buf.Out());
	}

	START_TIMER(query, 0, 1, wait);
	return query->GetQueryId();
}

void Watcher::StopQuery(const s64 queryId) {
	auto itr = _queries.find(queryId);
	if (itr != _queries.end()) {
		OASSERT(itr->second, "wtf");

		_kernel->KillTimer(itr->second);
	}
}

void Watcher::Query(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args) {
	s64 queryId;
	s32 type;
	const void * context;
	s32 size;

	args.ReadMulti(queryId, type);
	args.Read(context, size);

	auto itr = _queriors.find(type);
	if (itr == _queriors.end())
		return;

	olib::Buffer<MAX_QUERY_PACKET> buf;
	buf << queryId;

	s32 count = args.Left().GetSize() / sizeof(s64);
	for (s32 i = 0; i < count; ++i) {
		s64 id;
		args.Read(id);

		s8 batch = 0;
		IObject * object = OMODULE(ObjectMgr)->FindObject(id);
		if (object)
			batch = itr->second(kernel, object, context, size);

		buf << id << batch;
	}
	
	OMODULE(Harbor)->Send(nodeType, nodeId, OID("scene", "query_ack"), buf.Out());
}

void Watcher::QueryAck(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args) {
	s64 queryId;
	args.Read(queryId);

	auto itr = _queries.find(queryId);
	if (itr == _queries.end())
		return;

	s32 count = args.Left().GetSize() / (sizeof(s64) + sizeof(s8));
	for (s32 i = 0; i < count; ++i) {
		s64 id;
		s8 batch;
		args.ReadMulti(id, batch);

		itr->second->Awake(kernel, id, batch);
	}
}

void Watcher::DealInterest(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& args) {
	s64 id = 0;
	args.Read(id);

	IObject * reciever = OMODULE(ObjectMgr)->FindObject(id);
	if (reciever) {
		ITableControl * interest = reciever->FindTable(OTABLE("interest"));
		OASSERT(interest, "wtf");
		
		s16 count = 0;
		args.Read(count);
		for (s32 i = 0; i < count; ++i) {
			s64 interestId;
			s32 logic;
			args.ReadMulti(interestId, logic);
			IRow * row = interest->AddRowKeyInt64(id);
			row->SetDataInt32(OCOLUMN("interest", "logic"), logic);
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

	IObject * reciever = OMODULE(ObjectMgr)->FindObject(id);
	if (reciever) {
		ITableControl * watcher = reciever->FindTable(OTABLE("watcher"));
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
				row->SetDataInt32(OCOLUMN("watcher", "gate"), gate);
				row->SetDataInt32(OCOLUMN("watcher", "logic"), logic);

				if (gate > 0)
					actors[gate].push_back(watcherId);
			}
			if (!actors.empty()) {
				olib::Buffer<MAX_ROLE_NOTIFY_PACKET_SIZE> buf;
				buf << reciever->GetID() << reciever->GetPropInt32(OPROP("type"));
				s16 * propCount = buf.Reserve<s16>();
				for (auto * prop : reciever->GetPropsInfo()) {
					if (prop->GetSetting(reciever) & OSETTING("share")) {
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

				OMODULE(PacketSender)->Brocast(actors, OID("cli_scene", "role_appear"), buf.Out());
			}
		}

		args.Read(count);
		if (count > 0) {
			std::unordered_map<s32, std::vector<s64>> actors;
			for (s32 i = 0; i < count; ++i) {
				s64 watcherId;
				args.Read(watcherId);

				IRow * row = watcher->FindRow(id);
				s32 gate = row->GetDataInt32(OCOLUMN("watcher", "gate"));
				if (gate > 0)
					actors[gate].push_back(watcherId);

				DEL_TABLE_ROW(watcher, row);
			}

			olib::Buffer<64> buf;
			buf << reciever->GetID();

			OMODULE(PacketSender)->Brocast(actors, OID("cli_scene", "role_disappear"), buf.Out());
		}
	}
}

void Watcher::DisapperWhenDestroy(IKernel * kernel, const void * context, const s32 size) {
	OASSERT(size == sizeof(IObject*), "wtf");
	IObject * object = *(IObject**)context;

	olib::Buffer<64> buf;
	buf << object->GetID();

	Brocast(object, OID("cli_scene", "role_disappear"), buf.Out());
}
