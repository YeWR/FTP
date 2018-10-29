#ifndef CMD_H
#define CMD_H

#include "utils.h"

//================================================================================================================================================
// process cmd, especially whether the cmd is valid or not.
//================================================================================================================================================

// get the cmd type: if not valide, then set the ERROR type.
extern enum CMDTYPE getCmdType(const char *cmd);

// log in success, cannot only check prefix
extern int userSucc(const char *cmd);

// pwd success
extern int passSucc(const char *cmd);

// RETR success
extern int retrSucc(const char *cmd);

// STOR success
extern int storSucc(const char *cmd);

// QUIT success
extern int quitSucc(const char *cmd);

// SYST success
extern int systSucc(const char *cmd);

// TYPE success
extern int typeSucc(const char *cmd);

// PORT success: ip and port
extern int portSucc(const char *cmd);

// PASV success
extern int pasvSucc(const char *cmd);

// MKD success
extern int mkdSucc(const char *cmd);

// CWD success
extern int cwdSucc(const char *cmd);

// PWD success
extern int pwdSucc(const char *cmd);

// LIST success: LIST or LIST para
extern int listSucc(const char *cmd);

// RMD success
extern int rmdSucc(const char *cmd);

// RNFR success
extern int rnfrSucc(const char *cmd);

// RNTO success. We need more details then.
extern int rntoSucc(const char *cmd);

#endif