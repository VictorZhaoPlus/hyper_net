#ifndef __NODE_PROTOCOL_H__
#define __NODE_PROTOCOL_H__
#include "util.h"
#include "Define.h"

namespace node_proto {
    enum {
        NEW_NODE = 1,
		START_NODE,
		OVER_LOAD,
		TEST_DELAY,
		TEST_DELAY_RESPONE,

        RPCNR = 10001,
        RPC,
        RESPONE,

		USER,
    };

    struct NewNode {
        s32 nodeType;
        s32 nodeId;
        char ip[MAX_IP_SIZE];
        s32 port;
    };
}

#endif //__NODE_PROTOCOL_H__
