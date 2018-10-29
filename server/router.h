#ifndef ROUTER_H
#define ROUTER_H

#include "utils.h"
#include "msg.h"
#include "cmd.h"

//================================================================================================================================================
// routers: decide how to process data or msg with the cmd type.
//================================================================================================================================================

// success router: return whether the cmd is correct or can be executed successfully judged by the cmd type.
extern int successRouter(const char *cmd, enum CMDTYPE cmdType);

// msg router
// if cmdType != ERROR -> send success msg
// if cmdType == ERROR -> send ERROR msg which depends on errorType
// some msg should be processed out of this router for they need extra data.
extern void msgRouter(const int newfd, const enum CMDTYPE cmdType, const enum CMDTYPE errorType);

#endif