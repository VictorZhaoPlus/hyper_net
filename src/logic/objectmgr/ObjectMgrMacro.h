/* 
 * File:   ObjectMgrMacro.h
 * Author: alax
 *
 * Created on March 3, 2015, 10:44 AM
 */

#ifndef __ObjectMgrMacro_h__
#define __ObjectMgrMacro_h__

#include "IKernel.h"
#include "IObjectMgr.h"

extern tcore::IKernel * g_pKernel;
#define OM_TRACE(format, ...) { \
    char log[1024] = {0}; \
    SafeSprintf(log, sizeof(log), "[OM_LOG_TRACE]%s:%d:%s"#format, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
    g_pKernel->Log(log); \
}

#endif //defined __ObjectMgrMacro_h__
