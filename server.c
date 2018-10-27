#include <stdio.h>
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
// #include <sys/ioctl.h>

// #include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>


// TODO:need to clear all the printf functions.


// cmd type
enum CMDTYPE
{
    USER=0, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, PASV, MKD, CWD, PWD, LIST, RMD, RNFR, RNTO, ERROR
};

// NOTLOGIN -> not USER;
// NOTPWD -> not PASS
// LOGIN -> USER and PASS success
enum CLIENTSTATE
{
	NOTLOGIN=0,NOTPWD,LOGIN
};

struct thread_para
{
	int newfd;
	int sockfd;
};

void setDir(char *dir);

const int MAXCLIENTNUM = 100;
char ROOTDIR[100];

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
	// set the root directory
	char curDir[100];
	setDir(curDir);
	memset(ROOTDIR, 0, sizeof(ROOTDIR));
	strcpy(ROOTDIR, curDir);
	strcpy(ROOTDIR + strlen(curDir), "/tmp");// ROOTDIR = "...../tmp"
	chdir(ROOTDIR);
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
void printArr(char **arr, const int len)
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
int createServerSocket(const int port)
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
	char hname[128];
    struct hostent *hent;
    int i;

    gethostname(hname, sizeof(hname));

    hent = gethostbyname(hname);

    for(i = 0; hent->h_addr_list[i]; i++) {
        if(strcmp(inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i])), "127.0.0.1") != 0)
		{
			strcpy(ip, inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i])));
		}
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
char **split(const char *cmd, const char *s, int *numAddr)
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

// strip, clear the \r\n
void strip(char *cmd)
{
	int len = strlen(cmd);
	if(len < 2)
	{
		return;
	}
	if(cmd[len-2] == '\r' && cmd[len-1] == '\n')
	{
		cmd[len-2] = '\0';
		cmd[len-1] = '\0';
	}
}


// delete char ** ;len of char ** is num
void deleteCharArr2(char **source, const int num)
{
	int i = 0;
	for (i = 0; i < num; ++i)
	{
		free((char *)source[i]);
	}
	free((char *)source);
}

// prefix is the prefix of source
int prefixCorrect(const char *source, const char *prefix)
{	
	int len = strlen(prefix) + 1;
	if(strlen(source) < len - 1)
	{
		return 0;
	}
	char *cnt = (char *)malloc(sizeof(char) * len);
	strncpy(cnt, source, len - 1);
	cnt[len - 1] = '\0';
	
	int ans = 0;
	if(strcmp(cnt, prefix) == 0)
	{
		ans = 1;
	}
	
	free((char *)cnt);
	return ans;
}

// a string is a number
int isNumber(const char *source)
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
enum CMDTYPE getCmdType(const char *cmd)
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
		else if(prefixCorrect(temStr[0], "RNFR")) ans = RNFR;
		else if(prefixCorrect(temStr[0], "RNTO")) ans = RNTO;
		else ans = ERROR;
	}	
	deleteCharArr2(temStr, temLen);
	return ans;
}

// get the ip and port in PORT cmd if the cmd is verified.
void getIpPort(const char *cmd, char *ip, int *port)
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

// set the current directory in dir
void setDir(char *dir)
{
	char temDir[100];
	memset(temDir, 0, sizeof(temDir));
	getcwd(temDir, sizeof(temDir));
	memset(dir, 0, sizeof(dir));
	strcpy(dir, temDir);
}
//================================================================================================================================================

//================================================================================================================================================
// success verification
//================================================================================================================================================
// log in success, cannot only check prefix
int userSucc(const char* cmd)
{
	char scale[] = "USER anonymous";

	if(strcmp(cmd, scale) == 0)
	{
		return 1;
	}
	return 0;
}

// pwd success
int passSucc(const char* cmd)
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
int portSucc(const char *cmd)
{
	const int STDLEN = 7;
	// temLen may be change
	int temLen = 0;
	char **temStr = split(cmd, ", ", &temLen);// ',' and ' ' split
	
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
	
	deleteCharArr2(temStr, temLen);
	return ans;
}

// PASV success
int pasvSucc(const char *cmd)
{
	char scale[] = "PASV";

	if(strcmp(cmd, scale) == 0)
	{
		return 1;
	}
	return 0;
}

// SYST success
int systSucc(const char *cmd)
{
	char scale[] = "SYST";

	if(strcmp(cmd, scale) == 0)
	{
		return 1;
	}
	return 0;
}

// TYPE success
int typeSucc(const char *cmd)
{
	char scale[] = "TYPE I";
	
	if(strcmp(cmd, scale) == 0)
	{
		return 1;
	}
	return 0;
}

// PWD success
int pwdSucc(const char *cmd)
{
	char scale[] = "PWD";
	
	if(strcmp(cmd, scale) == 0)
	{
		return 1;
	}
	return 0;
}

// CWD success
int cwdSucc(const char *cmd)
{
	const int STDLEN = 2;
	// temLen may be change
	int temLen = 0;
	char **temStr = split(cmd, ", ", &temLen);// ',' and ' ' split
	
	int ans = 1;
	// need to check if out of ROOTDIR -> ROOTDIR is the prefix of the cwd para
	if(temLen == STDLEN && prefixCorrect(temStr[0], "CWD") && prefixCorrect(temStr[1], ROOTDIR))
	{		
		int cdSucc = chdir(temStr[1]);
		if(cdSucc != 0)
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

// success router
int successRouter(const char *cmd, enum CMDTYPE cmdType)
{
	switch(cmdType)
	{
		case USER:
			return userSucc(cmd);
		case PASS:
			return passSucc(cmd);
		case PORT:
			return portSucc(cmd);
		case PASV:
			return pasvSucc(cmd);
		case SYST:
			return systSucc(cmd);
		case TYPE:
			return typeSucc(cmd);
		case PWD:
			return pwdSucc(cmd);
		case CWD:
			return cwdSucc(cmd);
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
	const int bufLen = 100;
	send(newfd, msg, bufLen, 0);
	sync();
}

// send PASV msg: 227 Entering Passive Mode(166,111,80,233,128,2)
void sendPASVMsg(const int newfd, const char *ip, const int port)
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
	if(strlen(dir) == 0)
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

// msg router
// if cmdType != ERROR -> send success msg
// if cmdType == ERROR -> send ERROR msg which depends on errorType
void msgRouter(const int newfd, const enum CMDTYPE cmdType, const enum CMDTYPE errorType)
{
	switch(cmdType)
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
		case ERROR:
			{
				switch(errorType)
				{
					case USER:
						sendMsg(newfd, "530 the username is unacceptable.\r\n");
						break;
					case PASS:
						sendMsg(newfd, "503 username and password are jointly unacceptable.\r\n");
						break;
					case PWD:
						sendMsg(newfd, "550 your pwd request are rejected.\r\n");
						break;
					case CWD:
						sendMsg(newfd, "550 No such file or directory, or permission denied.\r\n");
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
        memset(cmd,0,sizeof(cmd));		// clear the buffer.
        recv_num = recv(newfd, cmd, bufLen, 0);
        if (recv_num<0)
		{
            printf("client %d exit...\n", newfd);
			fflush(stdout);
			break;
		}
        else if(recv_num>0)
        {
			strip(cmd);
        	// need USER
            if(clientState == NOTLOGIN)
            {
            	// log in success
				if(successRouter(cmd, USER))
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
            else if(clientState == NOTPWD)
            {
				// pwd in success	
				if(successRouter(cmd, PASS))
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
				if(successRouter(cmd, cmdType))
				{
					// is PORT
					if(cmdType == PORT)
					{
						memset(clientIp, 0, sizeof(clientIp));
						
						getIpPort(cmd, clientIp, &clientPort);
						msgRouter(newfd, cmdType, cmdType);
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
						msgRouter(newfd, cmdType, cmdType);// actually do nothing here.
						sendPASVMsg(newfd, serverIp, serverPort);
						// set the sock fd
						sockfd = createServerSocket(serverPort);
						// new a thread for waiting for client to connect
						struct thread_para para;
						para.sockfd = sockfd;
						pthread_create(&file_tid,NULL, fileSocket,&para);
						
					}
					// is PWD
					else if(cmdType == PWD)
					{
						// set the cline dir
						setDir(clientDir);
						// send PWD msg
						msgRouter(newfd, cmdType, cmdType);// actually do nothing here.
						sendPWDMsg(newfd, clientDir);
					}
					else if(cmdType == SYST || cmdType == TYPE)
					{
						msgRouter(newfd, cmdType, cmdType);
					}
					else if(cmdType == CWD)
					{
						// set the cline dir
						setDir(clientDir);
						msgRouter(newfd, cmdType, cmdType);
						printf("dir: %s\n", clientDir);				
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
	int newfd = accept(sockfd,NULL,NULL);
	
	if(newfd < 0)
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
