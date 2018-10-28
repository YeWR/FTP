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
    return 0;
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
    return 0;
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
    if (temLen == STDLEN && prefixCorrect(temStr[0], "CWD") && prefixCorrect(temStr[1], ROOTDIR))
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
    return 0;
}

// RNTO success
int rntoSucc(const char *cmd)
{
    return 0;
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

// send LIST msg
//  ->d_name: get the name of the file relative to the current directory, thereore, I need to reset the dir and set it back after thisngs finished.(If para)
void sendLISTMsg(const int newfd, const char *cmd, const char *dir)
{
    int temLen = 0;
    char **temStr = split(cmd, ", ", &temLen); // ',' and ' ' split
    char *path[100];
    memset(path, 0, sizeof(path));
    // LIST
    if (temLen == 1)
    {
        strcpy(path, dir);
    }
    // LIST para
    else
    {
        // reset dir
        chdir(ROOTDIR);
        strcpy(path, temStr[1]);
    }

    int isDir = isDirectory(path);
    // is directory
    if (isDir == 1)
    {
        DIR *dir = opendir(path);
        struct dirent *link;
        struct stat buf;

        char msg[4096] = {0};
        int index = 0;

        while ((link = readdir(dir)) != NULL)
        {
            if (lstat(link->d_name, &buf) == -1)
            {
                perror("lstat");
                continue;
            }
            if (strcmp(link->d_name, ".") == 0 || strcmp(link->d_name, "..") == 0)
            {
                continue;
            }
            getStatInfo(buf, link->d_name, msg + index);
            index = strlen(msg);
        }
        // no file in the dir
        if (index == 0)
        {
            strcpy(msg, "No file in the directory.\r\n");
        }
        sendMsg(newfd, msg);
    }
    // is file
    else if (isDir == 0)
    {
        int fileNameLen = 0;
        char **filePaths = split(path, "/\\", &fileNameLen);
        char *fileName = filePaths[fileNameLen - 1];

        int file, size;
        char file_type[11] = {0};
        struct stat buf;
        char msg[4096] = {0};

        file = open(path, O_RDONLY);
        fstat(file, &buf);

        getStatInfo(buf, fileName, msg);

        close(file);
        sendMsg(newfd, msg);

        deleteCharArr2(filePaths, fileNameLen);
    }

    // set dir back
    chdir(dir);
    deleteCharArr2(temStr, temLen);
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
        // do this in some where else, because I need some relative data.
        break;
    case RMD:
        sendMsg(newfd, "250 The directory was successfully removed!\r\n");
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

    // server socket fd, -1 -> closed
    int sockfd = -1;
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
                    msgRouter(newfd, USER, USER);
                }
                else
                {
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
                    msgRouter(newfd, PASS, PASS);
                }
                else
                {
                    msgRouter(newfd, ERROR, PASS);
                }
            }
            // login and pwd correct
            else if (clientState == LOGIN)
            {
                enum CMDTYPE cmdType = getCmdType(cmd);
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
                            // sleep(1);
                        }
                        serverPort = setServerPort();
                        // send PASV msg
                        msgRouter(newfd, cmdType, cmdType); // actually do nothing here.
                        sendPASVMsg(newfd, serverIp, serverPort);
                        // set the sock fd
                        sockfd = createServerSocket(serverPort);
                        // new a thread for waiting for client to connect
                        struct thread_para para;
                        para.sockfd = sockfd;
                        pthread_create(&file_tid, NULL, fileSocket, &para);
                    }
                    // is PWD
                    else if (cmdType == PWD)
                    {
                        // set the client dir
                        setDir(clientDir);
                        // send PWD msg
                        msgRouter(newfd, cmdType, cmdType); // actually do nothing here.
                        sendPWDMsg(newfd, clientDir);
                    }
                    else if (cmdType == CWD)
                    {
                        // set the client dir
                        setDir(clientDir);
                        msgRouter(newfd, cmdType, cmdType);
                    }
                    else if (cmdType == LIST)
                    {
                        // set the client dir
                        setDir(clientDir);
                        // send LIST msg
                        msgRouter(newfd, cmdType, cmdType); // actually do nothing here.
                        sendLISTMsg(newfd, cmd, clientDir);
                    }
                    else if (cmdType == SYST || cmdType == TYPE || cmdType == RMD)
                    {
                        msgRouter(newfd, cmdType, cmdType);
                    }
                }
                // cmd not success
                else
                {
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
    return ((void *)0);
}

// file
void *fileSocket(void *arg)
{
    //------------------------------------------------------------------------
    // the parameters need in the function
    struct thread_para *para = (struct thread_para *)arg;
    int sockfd = para->sockfd;
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    // some buffer for recv and send
    int recv_num;
    const int bufLen = 50;
    char buffer[bufLen];
    //------------------------------------------------------------------------

    // accept, waiting
    int newfd = accept(sockfd, NULL, NULL);

    if (newfd < 0)
    {
        return ((void *)0);
    }
    else
    {
        // send file or receive file
    }

    // trans data...
    while (1)
    {
        memset(buffer, 0, sizeof(buffer)); // clear the buffer.
        recv_num = recv(newfd, buffer, bufLen, 0);

        if (recv_num < 0)
        {
            printf("file socket %d exit...", newfd);
            fflush(stdout);
            break;
        }
        else if (recv_num > 0)
        {
            printf("file socket receive %s\n", buffer);
            fflush(stdout);
        }
        else
        {
            printf("error in the server...");
            fflush(stdout);
            break;
        }
        sleep(1);
    }

    return ((void *)0);
}