#ifndef __IHARBOR_H__
#define __IHARBOR_H__

#include "IKernel.h"
#include "IModule.h"
#include "OArgs.h"

class INodeListener {
public:
    virtual ~INodeListener() {}

    virtual void OnOpen(IKernel * kernel, s32 nodeType, s32 nodeId, bool hide, const char * ip, s32 port) = 0;
    virtual void OnClose(IKernel * kernel, s32 nodeType, s32 nodeId) = 0;
};

typedef void (* node_cb)(IKernel * kernel, s32 nodeType, s32 nodeId, const void * context, const s32 size);
typedef void (* node_args_cb)(IKernel * kernel, s32 nodeType, s32 nodeId, const OArgs & args);

class IHarbor : public IModule {
public:
    virtual ~IHarbor() {}

    virtual void Connect(const char * ip, const s32 port) = 0;
    virtual void AddNodeListener(INodeListener * listener, const char * debug) = 0;

	virtual void Send(s32 nodeType, s32 nodeId, const s32 messageId, const OArgs& args) = 0;
	virtual void Brocast(s32 nodeType, const s32 messageId, const OArgs& args) = 0;
	virtual void Brocast(const s32 messageId, const OArgs& args) = 0;

    virtual bool PrepareSend(s32 nodeType, s32 nodeId, const s32 messageId, const s32 size) = 0;
    virtual bool Send(s32 nodeType, s32 nodeId, const void * context, const s32 size) = 0;

    virtual void PrepareBrocast(s32 nodeType, const s32 messageId, const s32 size) = 0;
    virtual void Brocast(s32 nodeType, const void * context, const s32 size) = 0;

	virtual void PrepareBrocast(const s32 messageId, const s32 size) = 0;
	virtual void Brocast(const void * context, const s32 size) = 0;

    virtual void RegProtocolHandler(s32 messageId, const node_cb& handler, const char * debug) = 0;
	virtual void RegProtocolHandler(s32 messageId, const node_args_cb& handler, const char * debug) = 0;

    virtual s32 GetNodeType() const = 0;
    virtual s32 GetNodeId() const = 0;
};

#define REGPROTOCOL(messageId, handler) s_harbor->RegProtocolHandler(messageId, handler, #handler)

#endif //__IHARBOR_H__
