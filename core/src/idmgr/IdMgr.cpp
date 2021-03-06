#include "IdMgr.h"
#include "IHarbor.h"
#include "OArgs.h"
#include "XmlReader.h"
#include "IEventEngine.h"
#include "OBuffer.h"

#define SEQUENCE_BITS 5
#define SEQUENCE_MASK 0x001F
#define AREA_BITS 10
#define AREA_MASK 0x03FF
#define TIME_BITS 49
#define TIME_MASK 0x0001FFFFFFFFFFFF
#define SINGLE_PATCH 100
#define SINGLE_PATCH_SIZE 64000
#define TIMER_INTERVAL 500

bool IdMgr::Initialize(IKernel * kernel) {
    _kernel = kernel;

	olib::XmlReader reader;
	std::string coreConfigPath = std::string(tools::GetWorkPath()) + "/config/server_conf.xml";
	if (!reader.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
		return false;
	}

	const olib::IXmlObject& id = reader.Root()["id"][0];
	_areaId = reader.Root()["app"][0].GetAttributeInt32("area");

	_multiProcess = id.GetAttributeBoolean("multi");
	if (_multiProcess) {
		_nodeType = id.GetAttributeInt32("server");
		_poolSize = id.GetAttributeInt32("pool_size");
	}

    return true;
}

bool IdMgr::Launched(IKernel * kernel) {
	if (_multiProcess) {
		_loadFirst = false;

		if (OMODULE(Harbor)->GetNodeType() == _nodeType) {
			RGS_HABOR_ARGS_HANDLER(OID("id", "ask"), IdMgr::AskId);

			START_TIMER(this, 0, 1, TIMER_INTERVAL);
		}
		else {
			RGS_HABOR_HANDLER(OID("id", "give"), IdMgr::GiveId);

			START_TIMER(this, 0, TIMER_BEAT_FOREVER, TIMER_INTERVAL);
		}
	}
	else
		_loadFirst = true;
    return true;
}

bool IdMgr::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

s64 IdMgr::AllocId() {
	if (!_multiProcess)
		return GenerateId();
	else {
		if (OMODULE(Harbor)->GetNodeType() == _nodeType)
			return GenerateId();
		else {
			OASSERT(!_ids.empty(), "wtf");
			s64 ret = *_ids.rbegin();
			_ids.pop_back();
			return ret;
		}
	}
	return 0;
}

void IdMgr::OnTimer(IKernel * kernel, s32 beatCount, s64 tick) {
	
	if ((s32)_ids.size() < _poolSize) {
		if (OMODULE(Harbor)->GetNodeType() == _nodeType) {
			for (s32 i = 0; i < SINGLE_PATCH; ++i)
				_ids.push_back(GenerateId());

			if (_loadFirst) {
				OMODULE(EventEngine)->Exec(OID("evt_id", "id_loaded"), nullptr, 0);
				_loadFirst = false;
			}
		}
		else {
			IArgs<1, 32> args;
			args << 0;
			args.Fix();
			OMODULE(Harbor)->Send(_nodeType, 1, OID("id", "ask"), args.Out());
		}
	}
}

s64 IdMgr::GenerateId() {
	static s64 lastTimestamp = tools::GetTimeMillisecond();
	static s16 sequence = 0;

	s64 timestamp = tools::GetTimeMillisecond();
	if (lastTimestamp == timestamp) {
		sequence = (sequence + 1) & SEQUENCE_MASK;
		if (sequence == 0) {
			while (timestamp == lastTimestamp)
				timestamp = tools::GetTimeMillisecond();
		}
	}
	else
		sequence = 0;
	lastTimestamp = timestamp;

	return ((timestamp & TIME_MASK) << (AREA_BITS + SEQUENCE_BITS)) | ((_areaId & AREA_MASK) << SEQUENCE_BITS) | (sequence & SEQUENCE_MASK);
}

void IdMgr::AskId(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	olib::Buffer<SINGLE_PATCH * sizeof(s64)> ret;
	for (s32 i = 0; i < SINGLE_PATCH; ++i)
		ret << GenerateId();
	OMODULE(Harbor)->Send(nodeType, nodeId, OID("id", "give"), ret.Out());
}

void IdMgr::GiveId(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer & args) {
	s32 count = args.GetSize() / sizeof(s64);
	for (s32 i = 0; i < count; ++i) {
		s64 id = 0;
		if (!args.Read(id))
			return;

		_ids.push_back(id);
	}

	if (_loadFirst) {
		OMODULE(EventEngine)->Exec(OID("evt_id", "id_loaded"), nullptr, 0);
		printf("id loaded\n");
		_loadFirst = false;
	}
}
