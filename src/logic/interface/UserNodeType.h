#ifndef __USER_NODE_TYPE_H__
#define __USER_NODE_TYPE_H__
#include "NodeType.h"

namespace user_node_type {
    enum {
		START = node_type::USER,

		GATE,
		ACCOUNT,
		LOGIC,
		SCENEMGR,
		CELL,
    };
}

#endif //__NODE_TYPE_H__
