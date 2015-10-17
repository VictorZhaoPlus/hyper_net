#include "Test.h"
#include <string>

Test * Test::s_self = nullptr;
IKernel * Test::s_kernel = nullptr;

#pragma pack(push, 1)
struct TestHeader {
	s32 message;
	s32 len;
};
#pragma pack(pop)

bool Test::Initialize(IKernel * kernel) {
    s_self = this;
    s_kernel = kernel;

    return true;
}

bool Test::Launched(IKernel * kernel) {
    return true;
}

bool Test::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

void Test::OnConnected(IKernel * kernel) {
}

s32 Test::OnRecv(IKernel * kernel, const void * context, const s32 size) {
	return 0;
}

void Test::OnError(IKernel * kernel, const s32 error) {

}

void Test::OnDisconnected(IKernel * kernel) {

}

void Test::OnConnectFailed(IKernel * kernel) {

}

