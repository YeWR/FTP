#include "ftp.h"

// judge the cmd's type
enum CMDTYPE getCmdType(const char *cmd)
{
    enum CMDTYPE ans = ERROR;

    // temLen may be change
    int temLen = 0;
    char **temStr = split(cmd, " ", &temLen);

    if (temLen != 0)
    {
        if (prefixCorrect(temStr[0], "USER"))
            ans = USER;
        else if (prefixCorrect(temStr[0], "PASS"))
            ans = PASS;
        else if (prefixCorrect(temStr[0], "RETR"))
            ans = RETR;
        else if (prefixCorrect(temStr[0], "STOR"))
            ans = STOR;
        else if (prefixCorrect(temStr[0], "QUIT"))
            ans = QUIT;
        else if (prefixCorrect(temStr[0], "SYST"))
            ans = SYST;
        else if (prefixCorrect(temStr[0], "TYPE"))
            ans = TYPE;
        else if (prefixCorrect(temStr[0], "PORT"))
            ans = PORT;
        else if (prefixCorrect(temStr[0], "PASV"))
            ans = PASV;
        else if (prefixCorrect(temStr[0], "MKD"))
            ans = MKD;
        else if (prefixCorrect(temStr[0], "CWD"))
            ans = CWD;
        else if (prefixCorrect(temStr[0], "PWD"))
            ans = PWD;
        else if (prefixCorrect(temStr[0], "LIST"))
            ans = LIST;
        else if (prefixCorrect(temStr[0], "RMD"))
            ans = RMD;
        else if (prefixCorrect(temStr[0], "RNFR"))
            ans = RNFR;
        else if (prefixCorrect(temStr[0], "RNTO"))
            ans = RNTO;
        else
            ans = ERROR;
    }
    deleteCharArr2(temStr, temLen);
    return ans;
}

// log in success, cannot only check prefix
int userSucc(const char *cmd)
{
    char scale[] = "USER anonymous";

    if (strcmp(cmd, scale) == 0)
    {
        return 1;
    }
    return 0;
}

// pwd success
int passSucc(const char *cmd)
{
    const int STDLEN = 2;
    // temLen may be change
    int temLen = 0;
    char **temStr = split(cmd, " ", &temLen);

    int ans = 0;
    // pass cmd
    if (temLen == STDLEN)
    {
        if (prefixCorrect(temStr[0], "PASS"))
        {
            // TODO: re process pwd and verification
            ans = 1;
        }
    }

    deleteCharArr2(temStr, temLen);
    return ans;
}

// RETR success
int retrSucc(const char *cmd)
{
    const int STDLEN = 2;
    // temLen may be change
    int temLen = 0;
    char **temStr = split(cmd, ", ", &temLen); // ',' and ' ' split

    int ans = 1;
    // need to check if out of ROOTDIR -> ROOTDIR is the prefix of the cwd para
    // need to check if that para is file
    if (temLen == STDLEN && prefixCorrect(temStr[0], "RETR") && prefixCorrect(temStr[1], ROOTDIR) && isDirectory(temStr[1]) == 0)
    {
        ans = 1;
    }
    else
    {
        ans = 0;
    }

    deleteCharArr2(temStr, temLen);
    return ans;
}

// STOR success
int storSucc(const char *cmd)
{
    return 0;
}

// QUIT success
int quitSucc(const char *cmd)
{
    return 0;
}

// SYST success
int systSucc(const char *cmd)
{
    char scale[] = "SYST";

    if (strcmp(cmd, scale) == 0)
    {
        return 1;
    }
    return 0;
}

// TYPE success
int typeSucc(const char *cmd)
{
    char scale[] = "TYPE I";

    if (strcmp(cmd, scale) == 0)
    {
        return 1;
    }
    return 0;
}

// PORT success: ip and port
int portSucc(const char *cmd)
{
    const int STDLEN = 7;
    // temLen may be change
    int temLen = 0;
    char **temStr = split(cmd, ", ", &temLen); // ',' and ' ' split

    int ans = 1;
    // pass PORT and 6 numbers
    if (temLen == STDLEN && prefixCorrect(temStr[0], "PORT"))
    {
        int i = 1;
        // judge every para in PORT is number
        for (i = 1; i < STDLEN; ++i)
        {
            if (!isNumber(temStr[i]))
            {
                ans = 0;
                break;
            }
        }
    }
    else
    {
        ans = 0;
    }

    deleteCharArr2(temStr, temLen);
    return ans;
}

// PASV success
int pasvSucc(const char *cmd)
{
    char scale[] = "PASV";

    if (strcmp(cmd, scale) == 0)
    {
        return 1;
    }
    return 0;
}

// MKD success
int mkdSucc(const char *cmd)
{
    const int STDLEN = 2;
    // temLen may be change
    int temLen = 0;
    char **temStr = split(cmd, ", ", &temLen); // ',' and ' ' split

    int ans = 1;
    // need to check if out of ROOTDIR -> ROOTDIR is the prefix of the cwd para
    if (temLen == STDLEN && prefixCorrect(temStr[0], "MKD") && prefixCorrect(temStr[1], ROOTDIR))
    {
        // mkdir that dir
        int mkSucc = mkdir(temStr[1], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (mkSucc != 0)
        {
            ans = 0;
        }
    }
    else
    {
        ans = 0;
    }

    deleteCharArr2(temStr, temLen);
    return ans;
}

// CWD success
int cwdSucc(const char *cmd)
{
    const int STDLEN = 2;
    // temLen may be change
    int temLen = 0;
    char **temStr = split(cmd, ", ", &temLen); // ',' and ' ' split

    int ans = 1;
    // need to check if out of ROOTDIR -> ROOTDIR is the prefix of the cwd para
    // need to check if that para is directory
    if (temLen == STDLEN && prefixCorrect(temStr[0], "CWD") && prefixCorrect(temStr[1], ROOTDIR) && isDirectory(temStr[1]) == 1)
    {
        // cd that path
        int cdSucc = chdir(temStr[1]);
        if (cdSucc != 0)
        {
            ans = 0;
        }
    }
    else
    {
        ans = 0;
    }

    deleteCharArr2(temStr, temLen);
    return ans;
}

// PWD success
int pwdSucc(const char *cmd)
{
    char scale[] = "PWD";

    if (strcmp(cmd, scale) == 0)
    {
        return 1;
    }
    return 0;
}

// LIST success: LIST or LIST para
int listSucc(const char *cmd)
{
    char scale[] = "LIST";

    // LIST
    if (strcmp(cmd, scale) == 0)
    {
        return 1;
    }
    // LIST para
    else
    {
        const int STDLEN = 2;
        // temLen may be change
        int temLen = 0;
        char **temStr = split(cmd, ", ", &temLen); // ',' and ' ' split

        int ans = 1;
        // need to check if out of ROOTDIR -> ROOTDIR is the prefix of the cwd para
        if (temLen == STDLEN && prefixCorrect(temStr[0], "LIST") && prefixCorrect(temStr[1], ROOTDIR))
        {
            // if exist return isExist -> 0
            int isExist = access(temStr[1], F_OK);
            if (isExist != 0)
            {
                ans = 0;
            }
        }
        else
        {
            ans = 0;
        }

        deleteCharArr2(temStr, temLen);
        return ans;
    }
}

// RMD success
int rmdSucc(const char *cmd)
{
    const int STDLEN = 2;
    // temLen may be change
    int temLen = 0;
    char **temStr = split(cmd, ", ", &temLen); // ',' and ' ' split

    int ans = 1;
    // need to check if out of ROOTDIR -> ROOTDIR is the prefix of the cwd para
    // need to check if para is directory, not file!
    // need to check if para is the root.
    if (temLen == STDLEN && prefixCorrect(temStr[0], "RMD") && prefixCorrect(temStr[1], ROOTDIR) && strcmp(temStr[1], ROOTDIR) != 0 && isDirectory(temStr[1]) == 1)
    {
        int rmSucc = remove(temStr[1]);
        // remove the directory
        if (rmSucc != 0)
        {
            ans = 0;
        }
    }
    else
    {
        ans = 0;
    }

    deleteCharArr2(temStr, temLen);
    return ans;
}

// RNFR success
int rnfrSucc(const char *cmd)
{
    const int STDLEN = 2;
    // temLen may be change
    int temLen = 0;
    char **temStr = split(cmd, ", ", &temLen); // ',' and ' ' split

    int ans = 1;
    // need to check if out of ROOTDIR -> ROOTDIR is the prefix of the cwd para
    // need to check if that para is file
    if (temLen == STDLEN && prefixCorrect(temStr[0], "RNFR") && prefixCorrect(temStr[1], ROOTDIR) && isDirectory(temStr[1]) == 0)
    {
        ans = 1;
    }
    else
    {
        ans = 0;
    }

    deleteCharArr2(temStr, temLen);
    return ans;
}

// RNTO success. We need more details then.
int rntoSucc(const char *cmd)
{
    return 1;
}

// success router
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
//================================================================================================================================================

//================================================================================================================================================
// send msgs
//================================================================================================================================================
// send msg
void sendMsg(const int newfd, const char *msg)
{
    const int bufLen = 1000;
    send(newfd, msg, bufLen, 0);
    sync();
}

// send PASV msg: 227 Entering Passive Mode(166,111,80,233,128,2)
void sendPASVMsg(const int newfd, const char *ip, const int port)
{
    int temLen = 0;
    char **temStr = split(ip, ".", &temLen);
    if (temLen == 4)
    {
        char msg[100];
        memset(msg, 0, sizeof(msg));

        char scale[] = "227 Entering Passive Mode(";
        int index = 0;
        strcpy(msg, scale);
        index += strlen(scale);

        int i = 0;
        for (i = 0; i < temLen; ++i)
        {
            strcpy(msg + index, temStr[i]);
            index += strlen(temStr[i]);
            strcpy(msg + index, ",");
            index++;
        }

        int p1 = port / 256;
        int p2 = port % 256;

        const int SLEN = 10;
        char sp1[SLEN];
        memset(sp1, 0, sizeof(sp1));
        char sp2[SLEN];
        memset(sp2, 0, sizeof(sp2));
        sprintf(sp1, "%d", p1);
        sprintf(sp2, "%d", p2);

        // ...,p1,
        strcpy(msg + index, sp1);
        index += strlen(sp1);

        strcpy(msg + index, ",");
        index++;
        // ...,p1,p2
        strcpy(msg + index, sp2);
        index += strlen(sp2);

        // ...,p1,p2)
        strcpy(msg + index, ")\r\n");
        index += 3;

        // send msg
        sendMsg(newfd, msg);
    }

    deleteCharArr2(temStr, temLen);
}

// send PWD msg
void sendPWDMsg(const int newfd, const char *dir)
{
    if (strlen(dir) == 0)
    {
        return;
    }
    // :257 "dir"\r\n
    const char signHead[] = "257 \"";
    const char signTail[] = "\"\r\n\0";
    char msg[100];
    memset(msg, 0, sizeof(msg));
    int index = 0;

    strcpy(msg + index, signHead);
    index += strlen(signHead);

    strcpy(msg + index, dir);
    index += strlen(dir);

    strcpy(msg + index, signTail);
    index += strlen(signTail);

    sendMsg(newfd, msg);
}

// send LIST msg begin
void sendLISTMsg(const int newfd, const char *cmd, char *dirName)
{
    char msg[200];
    memset(msg, 0, sizeof(msg));

    // set the file name
    getFileName(cmd, dirName);

    // send 150 Opening BINARY mode data connection for robots.txt (26 bytes).
    sprintf(msg, "150 Opening BINARY mode data connection for %s.\r\n", dirName);
    sendMsg(newfd, msg);
}

// send MKD msg
void sendMKDMsg(const int newfd, const char *cmd)
{
    int temLen = 0;
    char **temStr = split(cmd, " ", &temLen);

    // send PWD msg with the mkd path: temStr[1]
    sendPWDMsg(newfd, temStr[1]);

    deleteCharArr2(temStr, temLen);
}

// send RETR msg with code 150
void sendRETRMsg(const int newfd, const char *cmd, char *fileName)
{
    char msg[200];
    memset(msg, 0, sizeof(msg));

    // set the file name
    getFileName(cmd, fileName);
    int fileSize = getFileSize(fileName);

    // send 150 Opening BINARY mode data connection for robots.txt (26 bytes).
    sprintf(msg, "150 Opening BINARY mode data connection for %s (%d bytes).\r\n", fileName, fileSize);
    sendMsg(newfd, msg);
}

// msg router
// if cmdType != ERROR -> send success msg
// if cmdType == ERROR -> send ERROR msg which depends on errorType
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
        case ERROR:
            sendMsg(newfd, "500 No such command.\r\n");
            break;
        }
    }
    break;
    }
}
//================================================================================================================================================

// read from socket
void *cmdSocket(void *arg)
{
    //------------------------------------------------------------------------
    // the parameters need in the function
    struct thread_para *para = (struct thread_para *)arg;
    int newfd = para->newfd;
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    // some flags
    const int bufLen = 100;

    // client state
    enum CLIENTSTATE clientState = NOTLOGIN;

    // current cmd type
    enum CMDTYPE cmdType = ERROR;
    // the previous cmd
    // for convenience, if RNTO then it's RNTO, instead of ERROR when failed.
    enum CMDTYPE previousCmdType = ERROR;

    char clientDir[bufLen];
    memset(clientDir, 0, sizeof(clientDir));

    // ip and port of the client after PORT
    char clientIp[20];
    memset(clientIp, 0, sizeof(clientIp));
    int clientPort;

    // ip and port of the server after PASV
    char serverIp[20];
    memset(serverIp, 0, sizeof(serverIp));

    // get the serverIp
    getServerIp(serverIp);
    int serverPort;

    // file socket fd, -1 -> closed
    int sockfd = -1;

    // file socket lock -> after data conection build and trans file, this thread will be locked.
    int dataConnectionLock = 0;

    // the file's name, which is to be renamed
    char oldFileName[bufLen];
    memset(oldFileName, 0, sizeof(oldFileName));

    // the trans file name
    char transFileName[bufLen];
    memset(transFileName, 0, sizeof(transFileName));
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    // some buffer for recv and send
    pthread_t file_tid;
    int recv_num;
    char cmd[bufLen];
    //------------------------------------------------------------------------

    // trans data...
    while (1)
    {
        memset(cmd, 0, sizeof(cmd)); // clear the buffer.
        recv_num = recv(newfd, cmd, bufLen, 0);

        // lock when file trans
        if (dataConnectionLock)
        {
            printf("waiting...%d\n", dataConnectionLock);
            sleep(1);
            continue;
        }

        if (recv_num < 0)
        {
            printf("client %d exit...\n", newfd);
            fflush(stdout);
            break;
        }
        else if (recv_num > 0)
        {
            strip(cmd);
            // need USER
            if (clientState == NOTLOGIN)
            {
                // log in success
                if (successRouter(cmd, USER))
                {
                    clientState = NOTPWD;
                    previousCmdType = USER;
                    msgRouter(newfd, USER, USER);
                }
                else
                {
                    previousCmdType = ERROR;
                    msgRouter(newfd, ERROR, USER);
                }
            }
            // login pwd, need PASS
            else if (clientState == NOTPWD)
            {
                // pwd in success
                if (successRouter(cmd, PASS))
                {
                    clientState = LOGIN;
                    previousCmdType = PASS;
                    msgRouter(newfd, PASS, PASS);
                }
                else
                {
                    previousCmdType = ERROR;
                    msgRouter(newfd, ERROR, PASS);
                }
            }
            // login and pwd correct
            else if (clientState == LOGIN)
            {
                cmdType = getCmdType(cmd);
                // if the format of cmd is correct
                if (successRouter(cmd, cmdType))
                {
                    // is PORT
                    if (cmdType == PORT)
                    {
                        memset(clientIp, 0, sizeof(clientIp));

                        getIpPort(cmd, clientIp, &clientPort);
                        msgRouter(newfd, cmdType, cmdType);
                    }
                    // is PASV
                    else if (cmdType == PASV)
                    {
                        // close the previous socket
                        if (sockfd >= 0)
                        {
                            close(sockfd);
                            sockfd = -1;
                        }
                        serverPort = setServerPort();
                        // send PASV msg
                        msgRouter(newfd, cmdType, cmdType); // actually do nothing here.
                        sendPASVMsg(newfd, serverIp, serverPort);
                        // new a thread for waiting for client to connect
                        struct thread_para para;
                        para.newfd = newfd;
                        para.sockType = PASV;
                        para.cmdTypeAddr = &cmdType;
                        para.port = serverPort;
                        para.fileName = transFileName;
                        para.dirName = clientDir;
                        para.lock = &dataConnectionLock;
                        para.sockfdAddr = &sockfd;
                        pthread_create(&file_tid, NULL, fileSocket, &para);
                    }
                    // is PWD
                    else if (cmdType == PWD)
                    {
                        // set the client dir
                        getDir(clientDir);
                        // send PWD msg
                        msgRouter(newfd, cmdType, cmdType); // actually do nothing here.
                        sendPWDMsg(newfd, clientDir);
                    }
                    // is CWD
                    else if (cmdType == CWD)
                    {
                        // set the client dir
                        getDir(clientDir);
                        msgRouter(newfd, cmdType, cmdType);
                    }
                    // is LIST
                    else if (cmdType == LIST)
                    {
                        if (previousCmdType == PORT)
                        {
                            // close the previous socket
                            if (sockfd >= 0)
                            {
                                close(sockfd);
                                sockfd = -1;
                            }

                            // set the client dir
                            getDir(clientDir);

                            // send 150 LIST msg and get the dir name
                            sendLISTMsg(newfd, cmd, clientDir);

                            // new a thread for waiting for client to connect
                            struct thread_para para;
                            para.newfd = newfd;
                            para.sockType = PORT;
                            para.cmdTypeAddr = &cmdType;
                            para.port = clientPort;
                            memset(para.ip, 0, sizeof(para.ip));
                            strcpy(para.ip, clientIp);
                            para.fileName = clientDir;
                            para.lock = &dataConnectionLock;
                            para.sockfdAddr = &sockfd;
                            pthread_create(&file_tid, NULL, fileSocket, &para);

                            // lock this thread
                            dataConnectionLock = 1;
                        }
                        else if (previousCmdType == PASV)
                        {
                            // send 150 LIST msg and get the dir name
                            sendLISTMsg(newfd, cmd, clientDir);

                            // lock this thread
                            dataConnectionLock = 1;
                        }
                        else
                        {
                            // no TCP connection
                            sendMsg(newfd, "500 Last request is not PORT neither PASV.\r\n");
                        }
                    }
                    // is MKD
                    else if (cmdType == MKD)
                    {
                        // send MKD msg
                        msgRouter(newfd, cmdType, cmdType); // actually do nothing here.
                        sendMKDMsg(newfd, cmd);
                    }
                    // is RNFR
                    else if (cmdType == RNFR)
                    {
                        // get the file name to be renamed
                        getFileName(cmd, oldFileName);
                        // send RNFR msg
                        msgRouter(newfd, cmdType, cmdType);
                    }
                    // is RNTO, success judge independently.
                    else if (cmdType == RNTO)
                    {
                        // previous cmd type is RNFR and success
                        if (previousCmdType == RNFR)
                        {
                            char newFileName[bufLen];
                            memset(newFileName, 0, sizeof(newFileName));
                            getFileName(cmd, newFileName);
                            // rename success
                            // new file name must in root
                            if (prefixCorrect(newFileName, ROOTDIR) && rename(oldFileName, newFileName) == 0)
                            {
                                // send RNTO msg
                                msgRouter(newfd, cmdType, cmdType);
                            }
                            else
                            {
                                // send RNTO msg failed.
                                sendMsg(newfd, "553 File name not allowed.\r\n");
                            }
                        }
                        // previous cmd type is not RNFR(not success)
                        else
                        {
                            msgRouter(newfd, ERROR, cmdType);
                        }
                    }
                    // is RETR
                    else if (cmdType == RETR)
                    {
                        // if previous cmd is PORT, here try to connect
                        if (previousCmdType == PORT)
                        {
                            // close the previous socket
                            if (sockfd >= 0)
                            {
                                close(sockfd);
                                sockfd = -1;
                            }

                            // send 150 RETR msg and get the file name
                            sendRETRMsg(newfd, cmd, transFileName);

                            // new a thread for waiting for client to connect
                            struct thread_para para;
                            para.newfd = newfd;
                            para.sockType = PORT;
                            para.cmdTypeAddr = &cmdType;
                            para.port = clientPort;
                            memset(para.ip, 0, sizeof(para.ip));
                            strcpy(para.ip, clientIp);
                            para.fileName = transFileName;
                            para.lock = &dataConnectionLock;
                            para.sockfdAddr = &sockfd;
                            pthread_create(&file_tid, NULL, fileSocket, &para);

                            // lock this thread
                            dataConnectionLock = 1;
                        }
                        // if previous cmd is PASV
                        else if (previousCmdType == PASV)
                        {
                            // send 150 RETR msg and get the file name
                            sendRETRMsg(newfd, cmd, transFileName);

                            // lock this thread
                            dataConnectionLock = 1;
                        }
                        // if no preceeding PORT or PASV
                        else
                        {
                            // no TCP connection
                            sendMsg(newfd, "500 Last request is not PORT neither PASV.\r\n");
                        }
                    }
                    else if (cmdType == SYST || cmdType == TYPE || cmdType == RMD)
                    {
                        msgRouter(newfd, cmdType, cmdType);
                    }
                    else if (cmdType == USER || cmdType == PASS)
                    {
                        sendMsg(newfd, "530 You've been log in.\r\n");
                    }

                    // set the previous cmd type
                    previousCmdType = cmdType;
                }
                // cmd not success
                else
                {
                    // set the previous cmd type
                    previousCmdType = ERROR;
                    msgRouter(newfd, ERROR, cmdType);
                }
            }
        }
        else
        {
            printf("error in the server...");
            fflush(stdout);
            break;
        }
        // sleep(1);
    }

    if (newfd > 0)
    {
        close(newfd);
    }

    return ((void *)0);
}

// file
void *fileSocket(void *arg)
{
    //------------------------------------------------------------------------
    // the parameters need in the function
    struct thread_para *para = (struct thread_para *)arg;
    // data connection thread fd -> for send msgs
    const int newfd = para->newfd;
    int *sockfdAddr = para->sockfdAddr;
    int *lock = para->lock;
    enum CLIENTSTATE sockType = para->sockType; //
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    // some buffer for recv and send
    const int bufLen = 1024;
    char buffer[bufLen];
    memset(buffer, 0, sizeof(buffer));
    char fileName[bufLen];
    memset(fileName, 0, sizeof(fileName));
    char dirName[bufLen];
    memset(dirName, 0, sizeof(dirName));

    int recv_num;
    int fileFd = -1;
    //------------------------------------------------------------------------

    // connection, waiting, PORT
    if (sockType == PORT)
    {
        char ip[20];
        memset(ip, 0, sizeof(ip));
        strcpy(ip, para->ip);
        int port = para->port;
        fileFd = connectToSocket(ip, port);
    }
    // accept, waiting, PASV
    else if (sockType == PASV)
    {
        int port = para->port;
        fileFd = acceptSocket(port);
    }

    *sockfdAddr = fileFd;
    if (fileFd < 0)
    {
        sendMsg(newfd, "425 No TCP connection was established.\r\n");
    }
    else
    {
        enum CLIENTSTATE cmdType = *(para->cmdTypeAddr);
        strcpy(fileName, para->fileName);
        strcpy(dirName, para->dirName);
        if (cmdType == LIST)
        {
            FILE *fstream = NULL;
            char ls[100];
            memset(ls, 0, sizeof(ls));
            sprintf(ls, "ls -al %s 2>/dev/null", dirName);

            if (NULL == (fstream = popen(ls, "r")))
            {
                sendMsg(newfd, "451 The server had trouble reading the file or directory from disk.\r\n");
            }
            else
            {
                int listSucc = 1;
                bzero(buffer, bufLen);
                while (NULL != fgets(buffer, sizeof(buffer), fstream))
                {
                    if (send(fileFd, buffer, strlen(buffer), 0) < 0)
                    {
                        listSucc = 0;
                        break;
                    }
                    
                    bzero(buffer, sizeof(buffer));
                }
                send(fileFd, "\r\n", 3, 0);
                pclose(fstream);
                // if success
                if (listSucc)
                {
                    // 226
                    msgRouter(newfd, cmdType, cmdType);
                }
                // if failed
                else
                {
                    sendMsg(newfd, "426 The TCP connection was established but then broken by the client or by network failure.\r\n");
                }
            }
        }
        else if (cmdType == STOR)
        {
        }
        else if (cmdType == RETR)
        {
            // trans data...
            FILE *fp = fopen(fileName, "r");
            int fileSucc = 1;
            if (fp == NULL)
            {
                sendMsg(newfd, "451 The server had trouble reading the file from disk.\r\n");
            }
            else
            {
                bzero(buffer, bufLen);
                int file_block_length = 0;
                while ((file_block_length = fread(buffer, sizeof(char), bufLen, fp)) > 0)
                {
                    if (send(fileFd, buffer, file_block_length, 0) < 0)
                    {
                        fileSucc = 0;
                        break;
                    }

                    bzero(buffer, sizeof(buffer));
                }
                fclose(fp);
                // if success
                if (fileSucc)
                {
                    // 226
                    msgRouter(newfd, cmdType, cmdType);
                }
                // if failed
                else
                {
                    sendMsg(newfd, "426 The TCP connection was established but then broken by the client or by network failure.\r\n");
                }
            }
        }

        close(fileFd);
        fileFd = -1;
        *sockfdAddr = fileFd;
    }
    *lock = 0;
    return ((void *)0);
}