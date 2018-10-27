#ifndef FTP_H
#define FTP_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils.h"

//================================================================================================================================================
// ftp
//================================================================================================================================================

//=========================================================================
// Variables
//=========================================================================
// NOTLOGIN -> not USER;
// NOTPWD -> not PASS
// LOGIN -> USER and PASS success
enum CLIENTSTATE
{
	NOTLOGIN = 0,
	NOTPWD,
	LOGIN
};

struct thread_para
{
	int newfd;
	int sockfd;
};

// cmd type
enum CMDTYPE
{
	USER = 0,
	PASS,
	RETR,
	STOR,
	QUIT,
	SYST,
	TYPE,
	PORT,
	PASV,
	MKD,
	CWD,
	PWD,
	LIST,
	RMD,
	RNFR,
	RNTO,
	ERROR
};

extern const int MAXCLIENTNUM;

extern char ROOTDIR[100];
//=========================================================================

//=========================================================================
// functions
//=========================================================================

extern enum CMDTYPE getCmdType(const char *cmd);
extern int userSucc(const char *cmd);
extern int passSucc(const char *cmd);
extern int retrSucc(const char *cmd);
extern int storSucc(const char *cmd);
extern int quitSucc(const char *cmd);
extern int systSucc(const char *cmd);
extern int typeSucc(const char *cmd);
extern int portSucc(const char *cmd);
extern int pasvSucc(const char *cmd);
extern int mkdSucc(const char *cmd);
extern int cwdSucc(const char *cmd);
extern int pwdSucc(const char *cmd);
extern int listSucc(const char *cmd);
extern int rmdSucc(const char *cmd);
extern int rnfrSucc(const char *cmd);
extern int rntoSucc(const char *cmd);
extern int successRouter(const char *cmd, enum CMDTYPE cmdType);
extern void sendMsg(const int newfd, const char *msg);
extern void sendPASVMsg(const int newfd, const char *ip, const int port);
extern void sendPWDMsg(const int newfd, const char *dir);
extern void sendLISTMsg(const int newfd, const char *cmd);
extern void msgRouter(const int newfd, const enum CMDTYPE cmdType, const enum CMDTYPE errorType);
extern void *cmdSocket(void *arg);
extern void *fileSocket(void *arg);

//=========================================================================

#endif