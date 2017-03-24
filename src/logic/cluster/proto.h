#ifndef __PROTO_H__
#define __PROTO_H__
#include "util.h"

#define MAX_IP_SIZE 64

namespace core_proto {
	struct NewNode {
		s32 nodeType;
		s32 nodeId;
		char ip[MAX_IP_SIZE];
		s32 port;
	};
}

#endif //__PROTO_H__