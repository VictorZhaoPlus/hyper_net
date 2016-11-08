#include "IdMgr.h"
#include "IHarbor.h"
#include "OArgs.h"
#include "XmlReader.h"

#define SEQUENCE_BITS 5
#define SEQUENCE_MASK 0x001F
#define AREA_BITS 10
#define AREA_MASK 0x03FF
#define TIME_BITS 49
#define TIME_MASK 0x0001FFFFFFFFFFFF
#define SINGLE_PATCH 1000
#define SINGLE_PATCH_SIZE 64000
#define TIMER_INTERVAL 500

bool IdMgr::Initialize(IKernel * kernel) {
    _kernel = kernel;

	olib::XmlReader reader;
	std::string coreConfigPath = std::string(tools::GetAppPath()) + "/config/server_conf.xml";
	if (!reader.LoadXml(coreConfigPath.c_str())) {
		OASSERT(false, "can't find core file : %s", coreConfigPath.c_str());
		return false;
	}

	const olib::IXmlObject& id = reader.Root()["id"][0];
	_multiProcess = id.GetAttributeBoolean("multi");
	if (_multiProcess) {
		_nodeType = id.GetAttributeInt32("server");
		_AskProtocolId = id.GetAttributeInt32("ask_proto");
		_GiveProtocolId = id.GetAttributeInt32("give_proto");
		_areaId = reader.Root()["app"][0].GetAttributeInt32("area");
		_poolSize = id.GetAttributeInt32("pool_size");
	}

    return true;
}

bool IdMgr::Launched(IKernel * kernel) {
	if (_multiProcess) {
		FIND_MODULE(_harbor, Harbor);
		RGS_HABOR_ARGS_HANDLER(_AskProtocolId, IdMgr::AskId);
		RGS_HABOR_ARGS_HANDLER(_GiveProtocolId, IdMgr::GiveId);

		START_TIMER(this, 0, TIMER_BEAT_FOREVER, TIMER_INTERVAL);
	}
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
		OASSERT(!_ids.empty(), "wtf");
		s64 ret = *_ids.rbegin();
		_ids.pop_back();
		return ret;
	}
	return 0;
}

void IdMgr::OnTimer(IKernel * kernel, s64 tick) {
	if ((s32)_ids.size() < _poolSize) {
		IArgs<1, 32> args;
		args << 0;
		_harbor->Send(_nodeType, 1, _AskProtocolId, args.Out());
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
	IArgs<SINGLE_PATCH, SINGLE_PATCH_SIZE> ret;
	for (s32 i = 0; i < SINGLE_PATCH; ++i)
		ret << GenerateId();
	_harbor->Send(nodeType, nodeId, _GiveProtocolId, ret.Out());
}

void IdMgr::GiveId(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args) {
	for (s32 i = 0; i < args.Count(); ++i)
		_ids.push_back(args.GetDataInt64(i));
}
