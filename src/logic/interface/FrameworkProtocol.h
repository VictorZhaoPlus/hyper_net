#ifndef __FRAMEWORK_PROTOCOL_H__
#define __FRAMEWORK_PROTOCOL_H__
#include "CoreProtocol.h"

namespace framework_proto {
	enum {
		START = core_proto::END,

		RPCNR,
		RPC,
		RESPONE,

		END,
	};
}

#endif //__USER_NODE_PROTOCOL_H__
