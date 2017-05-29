#ifndef __IHARBOR_H__
#define __IHARBOR_H__

#include "IKernel.h"
#include "IModule.h"

class OArgs;
class OBuffer;

class INodeListener {
public:
    virtual ~INodeListener() {}

    virtual void OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port) = 0;
    virtual void OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) = 0;
};

typedef std::function<void(IKernel * kernel, s32 nodeType, s32 nodeId, const OBuffer& stream)> NodeCB;
typedef std::function<void(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args)> NodeArgsCB;

class IHarbor : public IModule {
public:
    virtual ~IHarbor() {}

    virtual void Connect(const char * ip, const s32 port) = 0;
    virtual void AddNodeListener(INodeListener * listener, const char * debug) = 0;

	virtual void Send(s32 nodeType, s32 nodeId, const s32 messageId, const OArgs& args) = 0;
	virtual void Send(s32 nodeType, s32 nodeId, const s32 messageId, const OBuffer& args) = 0;
	virtual void Brocast(s32 nodeType, const s32 messageId, const OArgs& args) = 0;
	virtual void Brocast(const s32 messageId, const OArgs& args) = 0;

    virtual bool PrepareSend(s32 nodeType, s32 nodeId, const s32 messageId, const s32 size) = 0;
    virtual bool Send(s32 nodeType, s32 nodeId, const void * context, const s32 size) = 0;

    virtual void PrepareBrocast(s32 nodeType, const s32 messageId, const s32 size) = 0;
    virtual void Brocast(s32 nodeType, const void * context, const s32 size) = 0;

	virtual void PrepareBrocast(const s32 messageId, const s32 size) = 0;
	virtual void Brocast(const void * context, const s32 size) = 0;

    virtual void RegProtocolHandler(s32 messageId, const NodeCB& handler, const char * debug) = 0;
	virtual void RegProtocolArgsHandler(s32 messageId, const NodeArgsCB& handler, const char * debug) = 0;

    virtual s32 GetNodeType() const = 0;
    virtual s32 GetNodeId() const = 0;
};

#define RGS_HABOR_HANDLER(messageId, handler) OMODULE(Harbor)->RegProtocolHandler(messageId, std::bind(&handler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), #handler)
#define RGS_HABOR_ARGS_HANDLER(messageId, handler) OMODULE(Harbor)->RegProtocolArgsHandler(messageId, std::bind(&handler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), #handler)

#endif //__IHARBOR_H__
