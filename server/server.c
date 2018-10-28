#include "ftp.h"
// TODO:need to clear all the printf functions.

const int MAXCLIENTNUM = 100;
char ROOTDIR[100] = {0};

int main()
{
	int sockfd, newfds[MAXCLIENTNUM];
	int linkPort = 6789;
	int ret;
	int cur_thread_id = 0;

	pthread_t read_tids[MAXCLIENTNUM], write_tids[MAXCLIENTNUM];
	struct sockaddr_in server_addr;

	memset(newfds, -1, sizeof(newfds));

	server_addr.sin_family = AF_INET;		  // IPV4
	server_addr.sin_addr.s_addr = INADDR_ANY; // Bind INADDR_ANY address
	server_addr.sin_port = htons(linkPort);   // notice the htons
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		exit(1); // error in socket preparation
	}
	ret = bind(sockfd, (struct sockaddr *)(&server_addr), sizeof(server_addr));
	perror("server");
	if (ret < 0)
	{
		exit(2); // bind the socket with the addr error
	}
	ret = listen(sockfd, 10);
	if (ret < 0)
	{
		exit(3); // listen error
	}
	// set random seed
	srand((unsigned)time(NULL));
	// set the root directory
	char curDir[100];
	getDir(curDir);
	memset(ROOTDIR, 0, sizeof(ROOTDIR));
	strcpy(ROOTDIR, curDir);
	strcpy(ROOTDIR + strlen(curDir), "/tmp"); // ROOTDIR = "...../tmp"
	chdir(ROOTDIR);
	// can use the thread pool to optimize the multi-thread program, (if I have some spare time...)
	while (1)
	{
		// waiting for connect
		printf("waiting for client to connect...\n");
		fflush(stdout);
		newfds[cur_thread_id] = accept(sockfd, NULL, NULL);
		if (newfds[cur_thread_id] < 0)
		{
			exit(4); // fail to connect
		}
		// connect success
		int bufLen = 50;
		char msgOk[] = "220 Anonymous FTP server ready.\r\n";
		send(newfds[cur_thread_id], msgOk, bufLen, 0);
		sync();

		// new a thread to send / recv data from the client...
		struct thread_para para;
		para.newfd = newfds[cur_thread_id];
		pthread_create(&read_tids[cur_thread_id], NULL, cmdSocket, &para);

		// prepare the next socket for another client...
		cur_thread_id++; // send over successfully

		// after the thread pool over, it could be ok.
		if (cur_thread_id >= MAXCLIENTNUM)
		{
			printf("Too many clients (over 100)... Server has to shut...");
			fflush(stdout);
			break;
		}
	}
}