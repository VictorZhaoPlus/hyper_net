#include "kernel.h"
#include <stdio.h>

int main(int argc, char ** argv) {
    Kernel * kernel = Kernel::Instance();
    OASSERT(kernel != nullptr, "create kernel failed.");
	if (!kernel)
		return -1;
#ifndef _DEBUG
    OASSERT(kernel->Initialize(argc, argv), "start kernel failed.");
#else
    if (!kernel->Initialize(argc, argv)) {
        printf("start kernel failed");
        return -1;
    }
#endif
    kernel->Loop();
    kernel->Destroy();
    return 0;
}
