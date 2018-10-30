#include "router.h"

//================================================================================================================================================
// routers: decide how to process data or msg with the cmd type.
//================================================================================================================================================

// success router: return whether the cmd is correct or can be executed successfully judged by the cmd type.
int successRouter(const char *cmd, enum CMDTYPE cmdType)
{
    switch (cmdType)
    {
    case USER:
        return userSucc(cmd);
    case PASS:
        return passSucc(cmd);
    case RETR:
        return retrSucc(cmd);
    case STOR:
        return storSucc(cmd);
    case QUIT:
        return quitSucc(cmd);
    case SYST:
        return systSucc(cmd);
    case TYPE:
        return typeSucc(cmd);
    case PORT:
        return portSucc(cmd);
    case PASV:
        return pasvSucc(cmd);
    case MKD:
        return mkdSucc(cmd);
    case CWD:
        return cwdSucc(cmd);
    case PWD:
        return pwdSucc(cmd);
    case LIST:
        return listSucc(cmd);
    case RMD:
        return rmdSucc(cmd);
    case RNFR:
        return rnfrSucc(cmd);
    case RNTO:
        return rntoSucc(cmd);
    default:
        return 0;
    }
}

// msg router
// if cmdType != ERROR -> send success msg
// if cmdType == ERROR -> send ERROR msg which depends on errorType
// some msg should be processed out of this router for they need extra data.
void msgRouter(const int newfd, const enum CMDTYPE cmdType, const enum CMDTYPE errorType)
{
    switch (cmdType)
    {
    case USER:
        sendMsg(newfd, "331 Login success! Please send me your password (email).\r\n");
        break;
    case PASS:
        sendMsg(newfd, "230 pwd correct! You've got permission.\r\n");
        break;
    case PORT:
        sendMsg(newfd, "200 Port over!\r\n");
        break;
    case PASV:
        // do this in some where else, because I need some relative data.
        break;
    case SYST:
        sendMsg(newfd, "215 UNIX Type: L8\r\n");
        break;
    case TYPE:
        sendMsg(newfd, "200 Type set to I.\r\n");
        break;
    case PWD:
        // do this in some where else, because I need some relative data.
        break;
    case CWD:
        sendMsg(newfd, "250 Okay.\r\n");
        break;
    case LIST:
        sendMsg(newfd, "226 The entire directory was successfully transmitted.\r\n");
        break;
    case RMD:
        sendMsg(newfd, "250 The directory was successfully removed!\r\n");
        break;
    case MKD:
        // do this in some where else, because I need some relative data.
        break;
    case RNFR:
        sendMsg(newfd, "350 The file exists and is ready to be renamed.\r\n");
        break;
    case RNTO:
        sendMsg(newfd, "250 The file has been renamed successfully.\r\n");
        break;
    case RETR:
        sendMsg(newfd, "226 Transfer complete.\r\n");
        break;
    case STOR:
        sendMsg(newfd, "226 The entire file was successfully received and stored.\r\n");
        break;
    case QUIT:
        sendMsg(newfd, "221 Bye.\r\n");
        break;
    case ERROR:
    {
        switch (errorType)
        {
        case USER:
            sendMsg(newfd, "530 The username is unacceptable.\r\n");
            break;
        case PASS:
            sendMsg(newfd, "503 Username and password are jointly unacceptable.\r\n");
            break;
        case PORT:
            sendMsg(newfd, "500 PORT format error.\r\n");
            break;
        case PASV:
            sendMsg(newfd, "500 PASV format error.\r\n");
            break;
        case SYST:
            sendMsg(newfd, "500 SYST format error.\r\n");
            break;
        case TYPE:
            sendMsg(newfd, "500 TYPE format error.\r\n");
            break;
        case PWD:
            sendMsg(newfd, "550 Your pwd request are rejected.\r\n");
            break;
        case CWD:
            sendMsg(newfd, "550 No such file or directory, or permission denied.\r\n");
            break;
        case LIST:
            sendMsg(newfd, "550 No such file or directory, or permission denied.\r\n");
            break;
        case RMD:
            sendMsg(newfd, "550 No such directory, or permission denied.\r\n");
            break;
        case MKD:
            sendMsg(newfd, "550 Directory creation failed.\r\n");
            break;
        case RNFR:
            sendMsg(newfd, "550 No such file, or permission denied.\r\n");
            break;
        case RNTO: // not RNFR or RNFR invalid.
            sendMsg(newfd, "503 Last command was not RNFR or RNFR was invalid.\r\n");
            break;
        case RETR: // had trouble reading the file from disk.
            sendMsg(newfd, "550 No such file, or permission denied.\r\n");
            break;
        case STOR: // had trouble reading the file from disk.
            sendMsg(newfd, "550 The file exists or permission denied.\r\n");
            break;
        case QUIT:
            sendMsg(newfd, "500 QUIT format error.\r\n");
            break;
        default:
            sendMsg(newfd, "500 No such command.\r\n");
            break;
        }
    }
    break;
    }
}