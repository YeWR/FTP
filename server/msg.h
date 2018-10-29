#ifndef MSG_H
#define MSG_H

#include "utils.h"

//================================================================================================================================================
// send msgs: only those that need extra data in msg should be defined outside $sednMsg
//================================================================================================================================================

// send msg through newfd
extern void sendMsg(const int newfd, const char *msg);

// send PASV msg: 227 Entering Passive Mode(166,111,80,233,128,2)
extern void sendPASVMsg(const int newfd, const char *ip, const int port);

// send PWD msg
extern void sendPWDMsg(const int newfd, const char *dir);

// send LIST msg begin
extern void sendLISTMsg(const int newfd, const char *cmd, char *dirName);

// send MKD msg
extern void sendMKDMsg(const int newfd, const char *cmd);

// send RETR msg with code 150
extern void sendRETRMsg(const int newfd, const char *cmd, char *fileName);

// send STOR msg with code 150
extern void sendSTORMsg(const int newfd, const char *cmd, char *fileName);

#endif