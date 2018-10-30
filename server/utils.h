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
#include <ctype.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//================================================================================================================================================
// variables
//================================================================================================================================================

// maximum of clients permitted to connect
extern const int MAXCLIENTNUM;

// the root directory of the ftp
extern char ROOTDIR[100];

// NOTLOGIN -> not USER;
// NOTPWD -> not PASS
// LOGIN -> USER and PASS success
enum CLIENTSTATE
{
    NOTLOGIN = 0,
    NOTPWD,
    LOGIN
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

// the parameters a cmd thread need.
struct cmdThreadParameters
{
    int newfd; // the fd for cmd socket
};

// the parameters a file thread need.(data connections)
// some para need to be pointers as they may be changed after the thread running.
struct fileThreadParameters
{
    int newfd;       // the fd for cmd socket
    int *sockfdAddr; // the address of the fd for the file socket
    int *lock;       // the address of lock

    int port;       // the port needed
    char ip[20];    // the ip needed
    char *fileName; // the address of file  needed
    char *dirName;  // the address of dir needed

    enum CMDTYPE sockType;     // the type of sock -> PASV or PORT
    enum CMDTYPE *cmdTypeAddr; // the type of cmd -> LIST or RETR or STORE, the address of the cmd type
};

//================================================================================================================================================
// extra functions
//================================================================================================================================================

// get the argv to server
extern int getArgv(const int argc, const char **argv, int *port, char *root);

// split a string by any char in s, the len of a is num
extern char **split(const char *cmd, const char *s, int *numAddr);

// printf an array in some format in debug form. should be abandoned then.
extern void printArr(char **arr, const int len);

// strip, clear the \r\n
extern void strip(char *cmd);

// delete char ** ;len of char ** is num
extern void deleteCharArr2(char **source, const int num);

// create a socket listening on the given port, waiting...
extern int acceptSocket(const int port);

// create a socket to connect to the given ip and port, waiting...
extern int connectToSocket(const char *ip, const int port);

// set server port randomly and return the random port
extern int setServerPort();

// prefix is the prefix of source
extern int prefixCorrect(const char *source, const char *prefix);

// a string is a number
extern int isNumber(const char *source);

// is a directory or a file: 1 -> directory, 0 -> file, -1 -> not exist.
extern int isDirectory(const char *path);

// file exists under the root: 1 -> exists, 0 -> not
extern int isFileExists(const char *path);

// judge whether the path is relative or not: 1 -> is relative 0 -> is absolute.
extern int isRelativePath(const char *path);

// judge the path is accessible under the root: 1 -> is accessible, 0 -> not(the path is a dir)
extern int isAccessiblePath(const char *path);

// get the ip address of server
extern void getServerIp(char *ip);

// get the ip and port in PORT cmd if the cmd is verified.
extern void getIpPort(const char *cmd, char *ip, int *port);

// set the current directory in dir
extern void getDir(char *dir);

// get filename from cmd like: XXX filename -> get the <filename>
extern void getFileName(const char *cmd, char *fileName);

// get the size of a file
extern int getFileSize(const char *fileName);

// mkdir the path: 0 -> cannot, 1 -> mkdir success
extern int mkdirPath(const char *dir);

#endif