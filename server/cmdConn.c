#include "cmdConn.h"

//================================================================================================================================================
// the socket of cmd: choose how to process the cmd.
//================================================================================================================================================

// process RNTO cmd 
void processRNTO(const int newfd, const char *cmd, const enum CMDTYPE cmdType, const enum CMDTYPE previousCmdType, char *oldFileName)
{
	// previous cmd type is RNFR and success
	if (previousCmdType == RNFR)
	{
		int bufLen = 100;
		char newFileName[bufLen];
		memset(newFileName, 0, sizeof(newFileName));
		getFileName(cmd, newFileName);

        int rntoSucc = 0;
        int isOk = rename(oldFileName, newFileName);

        // rename success
        if(isOk == 0)
        {
            // under root -> success
            if(isFileExists(newFileName))
            {
                rntoSucc = 1;
            }
            // not under root -> rename back
            else
            {
                rename(newFileName, oldFileName);
                rntoSucc = 0;
            }
        }

        // rnto success
		if (rntoSucc)
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

// main loop of cmd recv
void *cmdSocket(void *arg)
{
    //=======================================================================================================
    // consider this function as a class, prepared for the variables.
    //=======================================================================================================
    //------------------------------------------------------------------------
    // the parameters need in the function
    struct cmdThreadParameters *para = (struct cmdThreadParameters *)arg;
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

    char listDir[bufLen];
    memset(listDir, 0, sizeof(listDir));

    // ip and port of the client after PORT
    char clientIp[20];
    memset(clientIp, 0, sizeof(clientIp));
    int clientPort;

    // ip and port of the server after PASV
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
    pthread_t fileTid;
    int recv_num;
    char cmd[bufLen];
    //------------------------------------------------------------------------
    //=======================================================================================================

    //=======================================================================================================
    // the loop: cmd recv
    //=======================================================================================================
    // trans data...
    while (1)
    {

        memset(cmd, 0, sizeof(cmd)); // clear the buffer.
        recv_num = (int) recv(newfd, cmd, bufLen, 0);
        // printf("%s\n", cmd);
        // lock when file trans
        if (dataConnectionLock)
        {
            // printf("waiting...%d\n", dataConnectionLock);
            sleep(1);
            continue;
        }

        if (recv_num < 0)
        {
            // printf("client %d exit...\n", newfd);
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

						// set server port
						serverPort = setServerPort();

						// send PASV msg
                        sendPASVMsg(newfd, SERVERIP, serverPort);

						// set the parameters
						struct fileThreadParameters para;
                        para.newfd = newfd;
                        para.sockType = PASV;
                        para.cmdTypeAddr = &cmdType;
                        para.port = serverPort;
                        para.fileName = transFileName;
                        para.dirName = listDir;
                        para.lock = &dataConnectionLock;
                        para.sockfdAddr = &sockfd;

						// new a thread
                        pthread_create(&fileTid, NULL, fileSocket, &para);
                    }
                    // is PWD
                    else if (cmdType == PWD)
                    {
                        // set the client dir
                        getCurrentDir(clientDir);

                        // send PWD msg
                        sendPWDMsg(newfd, clientDir);
                    }
                    // is CWD
                    else if (cmdType == CWD)
                    {
                        // set the client dir
                        getCurrentDir(clientDir);
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
                            getCurrentDir(clientDir);

                            // send 150 LIST msg and get the dir name
                            sendLISTMsg(newfd, cmd, listDir);

                            // new a thread for waiting for client to connect
                            struct fileThreadParameters para;
                            para.newfd = newfd;
                            para.sockType = PORT;
                            para.cmdTypeAddr = &cmdType;
                            para.port = clientPort;
                            memset(para.ip, 0, sizeof(para.ip));
                            strcpy(para.ip, clientIp);
                            para.fileName = listDir;
                            para.lock = &dataConnectionLock;
                            para.sockfdAddr = &sockfd;

							// new a
                            pthread_create(&fileTid, NULL, fileSocket, &para);

                            // lock this thread
                            dataConnectionLock = 1;
                        }
                        else if (previousCmdType == PASV)
                        {
                            // set the client dir
                            getCurrentDir(clientDir);

                            // send 150 LIST msg and get the dir name
                            sendLISTMsg(newfd, cmd, listDir);

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
                        processRNTO(newfd, cmd, cmdType, previousCmdType, oldFileName);
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

                            // set the parameters
                            struct fileThreadParameters para;
                            para.newfd = newfd;
                            para.sockType = PORT;
                            para.cmdTypeAddr = &cmdType;
                            para.port = clientPort;
                            memset(para.ip, 0, sizeof(para.ip));
                            strcpy(para.ip, clientIp);
                            para.fileName = transFileName;
                            para.lock = &dataConnectionLock;
                            para.sockfdAddr = &sockfd;

							// new a thread
                            pthread_create(&fileTid, NULL, fileSocket, &para);

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
                    else if (cmdType == STOR)
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

                            // send 150 STOR msg and get the file name
                            sendSTORMsg(newfd, cmd, transFileName);

                            // set the parameters
                            struct fileThreadParameters para;
                            para.newfd = newfd;
                            para.sockType = PORT;
                            para.cmdTypeAddr = &cmdType;
                            para.port = clientPort;
                            memset(para.ip, 0, sizeof(para.ip));
                            strcpy(para.ip, clientIp);
                            para.fileName = transFileName;
                            para.lock = &dataConnectionLock;
                            para.sockfdAddr = &sockfd;

							// new a thread
                            pthread_create(&fileTid, NULL, fileSocket, &para);

                            // lock this thread
                            dataConnectionLock = 1;
                        }
                        // if previous cmd is PASV
                        else if (previousCmdType == PASV)
                        {
                            // send 150 RETR msg and get the file name
                            sendSTORMsg(newfd, cmd, transFileName);

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
                    else if (cmdType == QUIT)
                    {
                        if (sockfd >= 0)
                        {
                            close(sockfd);
                            sockfd = -1;
                        }
                        msgRouter(newfd, cmdType, cmdType);
                        break;
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
            // printf("error in the server...");
            fflush(stdout);
            break;
        }
    }

    if (newfd >= 0)
    {
        close(newfd);
    }

    return ((void *)0);
}
