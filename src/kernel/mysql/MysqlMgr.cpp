#include "MysqlMgr.h"
#include "XmlReader.h"
#include "MysqlBase.h"
#include "kernel.h"
#include "ConfigMgr.h"

bool MysqlMgr::Ready() {
    return true;
}

bool MysqlMgr::Initialize() {
	s32 threadCount = ConfigMgr::Instance()->GetHttpThreadCount();
	if (threadCount > 0) {
		if (!Start(threadCount, this)) {
			OASSERT(false, "wtf");
			return false;
		}
	}

    return true;
}

bool MysqlMgr::Destroy() {
	s32 threadCount = ConfigMgr::Instance()->GetHttpThreadCount();
	if (threadCount > 0)
		Terminate();
    DEL this;
    return true;
}

void MysqlMgr::Loop() {
	Process(0);
}

void MysqlMgr::Execute(const s64 threadId, const char * mysql, IHttpHandler * handler) {

}

void MysqlMgr::Complete(MysqlBase * command) {
	command->Complete(Kernel::Instance());
	command->Release();
}

olib::IRunner<MysqlBase> * MysqlMgr::Create() {
	
}
