#ifndef UTILS_H
#define UTILS_H

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

//================================================================================================================================================
// extra functions
//================================================================================================================================================

extern char **split(const char *cmd, const char *s, int *numAddr);
extern void printArr(char **arr, const int len);
extern void strip(char *cmd);
extern void deleteCharArr2(char **source, const int num);

extern int createServerSocket(const int port);
extern int setServerPort();
extern int prefixCorrect(const char *source, const char *prefix);
extern int isNumber(const char *source);
extern int isDirectory(const char *path);

extern void getServerIp(char *ip);
extern void getIpPort(const char *cmd, char *ip, int *port);
extern void getDir(char *dir);
extern void getStatInfo(const struct stat buf, const char *fileName, char *msg);
extern void getFileName(const char *cmd, char *fileName);
#endif