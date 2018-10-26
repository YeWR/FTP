#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <sys/un.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <arpa/inet.h>

// TODO:need to clear all the printf functions.

// cmd type
enum CMDTYPE
{
    USER=0, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, PASV, MKD, CWD, PWD, LIST, RMD, MV, ERROR
};

struct thread_para
{
	int newfd;
	int sockfd;
};

const int MAXCLIENTNUM = 100;

void *cmdSocket();
void *fileSocket();

int main()
{
	int sockfd,newfds[MAXCLIENTNUM];
	int linkPort = 6789;
    int ret;
	int cur_thread_id = 0;

    pthread_t read_tids[MAXCLIENTNUM],write_tids[MAXCLIENTNUM];
    struct sockaddr_in server_addr;
	
	memset(newfds, -1, sizeof(newfds));

    server_addr.sin_family=AF_INET;			    // IPV4
    server_addr.sin_addr.s_addr=INADDR_ANY;		// Bind INADDR_ANY address
    server_addr.sin_port=htons(linkPort);			// notice the htons
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if (sockfd<0)
    {
        exit(1);								// error in socket preparation
    }
    ret=bind(sockfd,(struct sockaddr *)(&server_addr),sizeof(server_addr));
    perror("server");
    if (ret<0)
    {
        exit(2);								// bind the socket with the addr error
    }
    ret=listen(sockfd,10);
    if (ret<0)
    {
        exit(3);								// listen error
    }
	// set random seed
	srand((unsigned)time(NULL));
	// can use the thread pool to optimize the multi-thread program, (if I have some spare time...)
	while(1)
	{
        // waiting for connect
		printf("waiting for client to connect...\n");
		fflush(stdout);
		newfds[cur_thread_id]=accept(sockfd,NULL,NULL);			
		if (newfds[cur_thread_id]<0)
		{
			exit(4);								// fail to connect
		}
        // connect success
		int bufLen = 50;
		char msgOk[] = "220 Anonymous FTP server ready.\r\n";
		send(newfds[cur_thread_id], msgOk, bufLen, 0);
		sync();
		
		// new a thread to send / recv data from the client...
		struct thread_para para;
		para.newfd = newfds[cur_thread_id];
		pthread_create(&read_tids[cur_thread_id],NULL, cmdSocket,&para);
		
		// prepare the next socket for another client...
		cur_thread_id ++;// send over successfully
		
		// after the thread pool over, it could be ok.
		if (cur_thread_id >= MAXCLIENTNUM){
			printf("Too many clients (over 100)... Server has to shut...");
			fflush(stdout);
			break;
		}
	}
}
//================================================================================================================================================
// extra functions
//================================================================================================================================================
// print a array
void printArr(char **arr, int len)
{
	int i = 0;
	printf("************************************\n");
	fflush(stdout);
	for(i = 0; i < len; ++i)
	{
		printf("%s\n", arr[i]);
		fflush(stdout);
	}
	printf("************************************\n");
	fflush(stdout);
	fflush(stdout);
}

// create a socket listening on the given port
int createServerSocket(int port)
{
	int sockfd;
	int ret;
	struct sockaddr_in server_addr;
	server_addr.sin_family=AF_INET;			    // IPV4
    server_addr.sin_addr.s_addr=INADDR_ANY;		// Bind INADDR_ANY address
    server_addr.sin_port=htons(port);			// notice the htons
    sockfd=socket(AF_INET,SOCK_STREAM,0);
	
	if (sockfd<0)
    {
        exit(1);								// error in socket preparation
    }
    ret=bind(sockfd,(struct sockaddr *)(&server_addr),sizeof(server_addr));
    perror("server");
    if (ret<0)
    {
        exit(2);								// bind the socket with the addr error
    }
    ret=listen(sockfd,10);
    if (ret<0)
    {
        exit(3);								// listen error
    }
	return sockfd;
}

// get the ip address of server
void getServerIp(char *ip)
{
	memset(ip, 0, sizeof(ip));
	
	struct ifaddrs *ifAddrStruct = NULL;
    void *tmpAddrPtr = NULL;
 
    getifaddrs(&ifAddrStruct);
	while (ifAddrStruct!=NULL)
	{
		if ((ifAddrStruct->ifa_addr)->sa_family==AF_INET)
		{
			tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tmpAddrPtr, ip, 20);
			break;
		}	
		ifAddrStruct = ifAddrStruct->ifa_next;
	}
}

// set server port randomly
int setServerPort()
{
	int delta = 65536 - 20000;
	int port = 2000 + (rand() % delta); // 2000 + [0, 45536) -> [2000. 65535]
	return port;
}

// split a string by any char in s, the len of a is num
char **split(char *cmd, char *s, int *numAddr)
{
	char temCmd[100];
	
	memset(temCmd, 0, sizeof(temCmd));
	strcpy(temCmd, cmd);
	
	int num = 0;
	char *pch = strtok(temCmd, s);
	// get the num
	while (pch != NULL)
	{
		pch = strtok(NULL, s);
		num++;
	}
	// set the num
	*(numAddr) = num;
	char **ans = (char **)malloc(sizeof(char *) * num);
	
	memset(temCmd, 0, sizeof(temCmd));
	strcpy(temCmd, cmd);
	
	pch = strtok(temCmd, s);
	int i = 0;
	while (pch != NULL)
	{
		int len = strlen(pch);
		ans[i] = (char *)malloc(sizeof(char) * len);
		strcpy(ans[i], pch);
		pch = strtok(NULL, s);
		i++;
	}
	return ans;
}

// delete char ** ;len of char ** is num
void deleteCharArr2(char **source, int num)
{
	int i = 0;
	for (i = 0; i < num; ++i)
	{
		free((char *)source[i]);
	}
	free((char *)source);
}

// prefix is the prefix of source
int prefixCorrect(char *source, const char *prefix)
{
	int len = strlen(prefix);
	if(strlen(source) < len)
	{
		return 0;
	}
	char *cnt = (char *)malloc(sizeof(char) * len);
	strncpy(cnt, source, len);
	
	int ans = 0;
	if(strcmp(cnt, prefix) == 0)
	{
		ans = 1;
	}
	
	free((char *)cnt);
	return ans;
}

// a string is a number
int isNumber(char *source)
{
	int len = strlen(source);
	if(len == 0)
	{
		return 0;
	}
	int i = 0;
	for(i = 0; i < len; ++i)
	{
		if(!isdigit(source[i]))
		{
			return 0;
		}
	}
	return 1;
}

// judge the cmd's type
enum CMDTYPE getCmdType(char *cmd)
{
	enum CMDTYPE ans = ERROR;
	
	// temLen may be change
	int temLen = 0;
	char **temStr = split(cmd, " ", &temLen);
	
	if(temLen != 0)
	{
		if(prefixCorrect(temStr[0], "USER")) ans = USER;
		else if(prefixCorrect(temStr[0], "PASS")) ans = PASS;
		else if(prefixCorrect(temStr[0], "RETR")) ans = RETR;
		else if(prefixCorrect(temStr[0], "STOR")) ans = STOR;
		else if(prefixCorrect(temStr[0], "QUIT")) ans = QUIT;
		else if(prefixCorrect(temStr[0], "SYST")) ans = SYST;
		else if(prefixCorrect(temStr[0], "TYPE")) ans = TYPE;
		else if(prefixCorrect(temStr[0], "PORT")) ans = PORT;
		else if(prefixCorrect(temStr[0], "PASV")) ans = PASV;
		else if(prefixCorrect(temStr[0], "MKD")) ans = MKD;
		else if(prefixCorrect(temStr[0], "CWD")) ans = CWD;
		else if(prefixCorrect(temStr[0], "PWD")) ans = PWD;
		else if(prefixCorrect(temStr[0], "LIST")) ans = LIST;
		else if(prefixCorrect(temStr[0], "RMD")) ans = RMD;
		else if(prefixCorrect(temStr[0], "MV")) ans = MV;
		else ans = ERROR;
	}	
	deleteCharArr2(temStr, temLen);
	return ans;
}

// get the ip and port in PORT cmd if the cmd is verified.
void getIpPort(char *cmd, char *ip, int *port)
{
	// [PORT xxx.xxx.xxx.xxx xxx,xxx] len => 7
	const int STDLEN = 7;
	// temLen may be change
	int temLen = 0;
	char **temStr = split(cmd, ", ", &temLen);// ',' and ' ' split
	
	if(temLen == STDLEN)
	{
		if(strlen(ip) > 15)
		{
			int i = 1;
			int ip_index = 0;
			for(i = 1; i <= 4; ++i)
			{
				strcpy(ip + ip_index, temStr[i]);
				ip_index += strlen(temStr[i]);
				if(i != 4)
				{
					ip[ip_index] = '.';					
				}
				else
				{
					ip[ip_index] = '\0';					
				}
				ip_index++;
			}
		}
		int temPort = 256 * atoi(temStr[5]);
		temPort += atoi(temStr[6]);
		
		*port = temPort;
	}	
	
	deleteCharArr2(temStr, temLen);
}
//================================================================================================================================================

//================================================================================================================================================
// success verification
//================================================================================================================================================
// log in success, cannot only check prefix
int loginSucc(char* cmd)
{
	char scale[] = "USER anonymous";

	if(strcmp(cmd, scale) == 0)
	{
		return 1;
	}
	return 0;
}

// pwd success
int pwdSucc(char* cmd)
{
	const int STDLEN = 2;
	// temLen may be change
	int temLen = 0;
	char **temStr = split(cmd, " ", &temLen);
	
	int ans = 0;
	// pass cmd
	if(temLen == STDLEN)
	{
		if(prefixCorrect(temStr[0], "PASS"))
		{
			// TODO: re process pwd and verification
			ans = 1;
		}
	}
	
	deleteCharArr2(temStr, temLen);
	return ans;
}

// PORT success: ip and port
int portSucc(char *cmd)
{
	const int STDLEN = 7;
	// temLen may be change
	int temLen = 0;
	char **temStr = split(cmd, ", ", &temLen);// ',' and ' ' split
	
	printArr(temStr, temLen);
	
	int ans = 1;
	// pass PORT and 6 numbers
	if(temLen == STDLEN && prefixCorrect(temStr[0], "PORT"))
	{
		int i = 1;
		// judge every para in PORT is number
		for(i = 1; i < STDLEN; ++i)
		{
			if(!isNumber(temStr[i]))
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
	
	printf("port: %d\n", ans);
	
	deleteCharArr2(temStr, temLen);
	return ans;
}

// PASV success
int pasvSucc(char *cmd)
{
	char scale[] = "PASV";

	if(strcmp(cmd, scale) == 0)
	{
		return 1;
	}
	return 0;
}

// success router
int successRouter(char *cmd, enum CMDTYPE TYPE)
{
	printf("%d, %s\n", TYPE, cmd);
	switch(TYPE)
	{
		case USER:
			return loginSucc(cmd);
		case PASS:
			return pwdSucc(cmd);
		case PORT:
			return portSucc(cmd);
		case PASV:
			return pasvSucc(cmd);
		default:
			return 0;
	}
}
//================================================================================================================================================

//================================================================================================================================================
// send msgs
//================================================================================================================================================
// send msg 
void sendMsg(int newfd, const char *msg)
{
	const int bufLen = 100;
	send(newfd, msg, bufLen, 0);
	sync();
}

// send PASV msg: 227 Entering Passive Mode(166,111,80,233,128,2)
void sendPASVMsg(int newfd, char *ip, int port)
{
	int temLen = 0;
	char **temStr = split(ip, ".", &temLen);
	if(temLen == 4)
	{
		char msg[100];
		memset(msg, 0, sizeof(msg));
		
		char scale[] = "227 Entering Passive Mode(";
		int index = 0;
		strcpy(msg, scale);
		index += strlen(scale);
		
		int i = 0;
		for(i = 0; i < temLen; ++i)
		{
			strcpy(msg + index, temStr[i]);
			index += strlen(temStr[i]);
			strcpy(msg + index, ",");
			index ++;
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
		index ++;
		// ...,p1,p2
		strcpy(msg + index, sp2);
		index += strlen(sp2);
		
		// ...,p1,p2)
		strcpy(msg + index, ")");
		index++;	

		// send msg
		sendMsg(newfd, msg);
	}
	
	deleteCharArr2(temStr, temLen);
}

// msg router
void msgRouter(int newfd, enum CMDTYPE TYPE)
{
	switch(TYPE)
	{
		case USER: 
			sendMsg(newfd, "Login success! Please send me your password (email).\r\n");
			break;
		case PASS:
			sendMsg(newfd, "pwd correct! You've got permission.\r\n");
			break;
		case PORT:
			sendMsg(newfd, "200 Port over!\r\n");
			break;
		case PASV:
			// do this in some where else, because I need some relative data.
			break;
		case ERROR:
			sendMsg(newfd, "Invalid command!\r\n");
			break;
		default:
			sendMsg(newfd, "Invalid command!\r\n");
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
	// client need to login.
    // -1 -> need USER
    // 0 -> need PASS
    // 1-> ok
    int isLogin = -1;   
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
	const int bufLen = 50;
    char cmd[bufLen];
	//------------------------------------------------------------------------
	
	// trans data...
    while (1)
    {
        memset(cmd,0,sizeof(cmd));		// clear the buffer.
        recv_num = recv(newfd, cmd, bufLen, 0);
		
        if (recv_num<0)
		{
            printf("client %d exit...", newfd);
			fflush(stdout);
			break;
		}
        else if(recv_num>0)
        {
			printf("%s\n", cmd);
			fflush(stdout);
        	// need USER
            if(isLogin == -1)
            {
            	// log in success
				if(successRouter(cmd, USER))
				{
					isLogin = 0;
					msgRouter(newfd, USER);
				}
				else
				{
					msgRouter(newfd, ERROR);
				}
            }
            // login pwd, need PASS
            else if(isLogin == 0)
            {
				// pwd in success	
				if(successRouter(cmd, PASS))
				{
					isLogin = 1;
					msgRouter(newfd, PASS);
				}
				else
				{
					msgRouter(newfd, ERROR);
				}
            }
			// login and pwd correct
			else
			{
				enum CMDTYPE cmdType = getCmdType(cmd);
				// if the format of cmd is correct
				if(successRouter(cmd, cmdType))
				{
					// is PORT
					if(cmdType == PORT)
					{
						printf("gggggggggg\n");
						fflush(stdout);
						memset(clientIp, 0, sizeof(clientIp));
						
						getIpPort(cmd, clientIp, &clientPort);
						msgRouter(newfd, PORT);
					}
					// is PASV
					else if(cmdType == PASV)
					{
						// close the previous socket
						if(sockfd >= 0)
						{
							close(sockfd);
							// sleep(1);
						}
						serverPort = setServerPort();
						// send PASV msg
						msgRouter(newfd, PASV);// actually do nothing here.
						sendPASVMsg(newfd, serverIp, serverPort);
						// set the sock fd
						sockfd = createServerSocket(serverPort);
						// new a thread for waiting for client to connect
						struct thread_para para;
						para.sockfd = sockfd;
						pthread_create(&file_tid,NULL, fileSocket,&para);
						
					}
				}
				else
				{
					msgRouter(newfd, ERROR);
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
	int newfd = accept(sockfd,NULL,NULL);
	
	if(newfd < 0)
	{
		return ((void *)0);
	}
	
	// trans data...
    while (1)
    {
        memset(buffer,0,sizeof(buffer));		// clear the buffer.
        recv_num = recv(newfd, buffer, bufLen, 0);
		
        if (recv_num<0)
		{
            printf("file socket %d exit...", newfd);
			fflush(stdout);
			break;
		}
        else if(recv_num>0)
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

