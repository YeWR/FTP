#include "fileConn.h"

// the waiting state in PORT format and return a socket fd
int PORTWaiting(const char *ip, const int port)
{
    return connectToSocket(ip, port);
}

// the waiting state in PASV format and return a socket fd
int PASVWaiting(const int port)
{
    return acceptSocket(port);
}

// process LIST cmd
void processLIST(const int newfd, const int filefd, const enum CMDTYPE cmdType, const char *dirName)
{
    const int bufLen = 1024;
    char buffer[bufLen];

    char ls[100];
    memset(ls, 0, sizeof(ls));
    sprintf(ls, "ls -al %s 2>/dev/null", dirName);

    FILE *fstream = NULL;

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
            if (send(filefd, buffer, strlen(buffer), 0) < 0)
            {
                listSucc = 0;
                break;
            }

            bzero(buffer, sizeof(buffer));
        }
        send(filefd, "\r\n", 3, 0);
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

// process STOR cmd
void processSTOR(const int newfd, const int filefd, const enum CMDTYPE cmdType, const char *fileName)
{
    const int bufLen = 1024;
    char buffer[bufLen];

    int pathLen = 0;
    char **fnPath = split(fileName, "/", &pathLen);

    char *fn = fnPath[pathLen - 1];
    FILE *fp = fopen(fn, "w");
    int fileSucc = 1;

    if (fp == NULL)
    {
        sendMsg(newfd, "451 The server had trouble saving the file to disk.\r\n");
    }
    else
    {
        bzero(buffer, sizeof(buffer));
        int length = 0;
        while ((length = recv(filefd, buffer, bufLen, 0)) != 0 )
        {
            if (length < 0)
            {
                fileSucc = 0;
                break;
            }

            int write_length = fwrite(buffer, sizeof(char), length, fp);
            if (write_length < length)
            {
                fileSucc = 0;
                break;
            }
            bzero(buffer, bufLen);
        }

        if (fileSucc)
        {
            msgRouter(newfd, cmdType, cmdType);
        }
        else
        {
            sendMsg(newfd, "426 The TCP connection was established but then broken by the client or by network failure Or file write failed.\r\n");
        }

        // 传输完毕，关闭socket
        fclose(fp);
    }

    deleteCharArr2(fnPath, pathLen);
}

// process RETR cmd
void processRETR(const int newfd, const int filefd, const enum CMDTYPE cmdType, const char *fileName)
{
    const int bufLen = 1024;
    char buffer[bufLen];

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
            if (send(filefd, buffer, file_block_length, 0) < 0)
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

// main loop of file recv
void *fileSocket(void *arg)
{
    //=======================================================================================================
    // consider this function as a class, prepared for the variables.
    //=======================================================================================================
    //------------------------------------------------------------------------
    // the parameters need in the function
    struct fileThreadParameters *para = (struct fileThreadParameters *)arg;
    // data connection thread fd -> for send msgs
    const int newfd = para->newfd;
    int *sockfdAddr = para->sockfdAddr;
    int *lock = para->lock;
    enum CMDTYPE sockType = para->sockType; //
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    // file fd
    int filefd = -1;
    //------------------------------------------------------------------------
    //=======================================================================================================

    // connection, waiting, PORT
    if (sockType == PORT)
    {
        filefd = PORTWaiting(para->ip, para->port);
    }
    // accept, waiting, PASV
    else if (sockType == PASV)
    {
        filefd = PASVWaiting(para->port);
    }

    // set the file sock fd
    *sockfdAddr = filefd;
    if (filefd < 0)
    {
        sendMsg(newfd, "425 No TCP connection was established.\r\n");
    }
    else
    {
        enum CMDTYPE cmdType = *(para->cmdTypeAddr);
        if (cmdType == LIST)
        {
            processLIST(newfd, filefd, cmdType, para->dirName);
        }
        else if (cmdType == STOR)
        {
            processSTOR(newfd, filefd, cmdType, para->fileName);
        }
        else if (cmdType == RETR)
        {
            processRETR(newfd, filefd, cmdType, para->fileName);
        }

        close(filefd);
        filefd = -1;
        *sockfdAddr = filefd;
    }

    // unlock the cmd socket
    *lock = 0;

    return ((void *)0);
}