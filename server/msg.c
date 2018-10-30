#include "msg.h"

//================================================================================================================================================
// send msgs
//================================================================================================================================================

// send msg through newfd
void sendMsg(const int newfd, const char *msg)
{
    send(newfd, msg, strlen(msg), 0);
    // sync();
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

// send STOR msg with code 150
void sendSTORMsg(const int newfd, const char *cmd, char *fileName)
{
    char msg[200];
    memset(msg, 0, sizeof(msg));

    // set the file name
    getFileName(cmd, fileName);

    // send 150 Ready to store files.
    sprintf(msg, "150 Ready to store file %s.\r\n", fileName);
    sendMsg(newfd, msg);
}
