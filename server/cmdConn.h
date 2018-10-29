#ifndef FTP_H
#define FTP_H

#include "utils.h"
#include "fileConn.h"
#include "cmd.h"
#include "msg.h"
#include "router.h"

//================================================================================================================================================
// the socket of cmd: choose how to process the cmd.
//================================================================================================================================================

// main loop of cmd recv
extern void *cmdSocket(void *arg);

// process RNTO cmd 
extern void processRNTO(const int newfd, const char *cmd, const enum CMDTYPE cmdType, const enum CMDTYPE previousCmdType, char *oldFileName);

// some cmd need too many paras, therefore implement them in the loop.

#endif