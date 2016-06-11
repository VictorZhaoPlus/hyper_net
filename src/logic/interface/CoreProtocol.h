#ifndef __CORE_PROTOCOL_H__
#define __CORE_PROTOCOL_H__
#include "util.h"
#include "Define.h"

namespace core_proto {
    enum {
        NEW_NODE = 1,
		START_NODE,
		OVER_LOAD,
		TEST_DELAY,
		TEST_DELAY_RESPONE,

		END,
    };

    struct NewNode {
        s32 nodeType;
        s32 nodeId;
        char ip[MAX_IP_SIZE];
        s32 port;
    };
}

#endif //__CORE_PROTOCOL_H__
