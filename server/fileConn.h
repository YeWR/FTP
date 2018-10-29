#ifndef FILECONN_H
#define FILECONN_H

#include "utils.h"
#include "cmd.h"
#include "msg.h"
#include "router.h"

//================================================================================================================================================
// the socket of cmd: choose how to process the cmd.
//================================================================================================================================================

// main loop of file recv
extern void *fileSocket(void *arg);

// the waiting state in PORT format and return a socket fd
extern int PORTWaiting(const char *ip, const int port);

// the waiting state in PASV format and return a socket fd
extern int PASVWaiting(const int port);

// process LIST cmd
extern void processLIST(const int newfd, const int fileFd, const enum CMDTYPE cmdType, const char *dirName);

// process STOR cmd
extern void processSTOR(const int newfd, const int filefd, const enum CMDTYPE cmdType, const char *fileName);

// process RETR cmd
extern void processRETR(const int newfd, const int filefd, const enum CMDTYPE cmdType, const char *fileName);

#endif
