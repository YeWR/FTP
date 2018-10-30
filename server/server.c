#include "cmdConn.h"
#include "fileConn.h"
// TODO:need to clear all the printf functions.

const int MAXCLIENTNUM = 100;
char ROOTDIR[100] = {0};
char SERVERIP[20] = {0};

int main(int argc, char **argv) {
    char root[20];
    memset(root, 0, sizeof(root));
    strcpy(root, "/tmp");
    int sockfd, newfds[MAXCLIENTNUM];
    int linkPort = 21;
    int ret;
    int cur_thread_id = 0;

    memset(ROOTDIR, 0, sizeof(ROOTDIR));
    memset(SERVERIP, 0, sizeof(SERVERIP));

    // get the port and root
    int notSudo = getArgv(argc, (const char **) argv, &linkPort, root);
    // -root -port
    if (notSudo == 1) {
        // set the root directory
        char curDir[70];
        getDir(curDir);
        if (isRelativePath(root)) {
            sprintf(ROOTDIR, "%s/%s", curDir, root);
        } else {
            strcpy(ROOTDIR, root);
        }
    }
        // /tmp
    else if (notSudo == 0) {
        strcpy(ROOTDIR, "/tmp");
    }
    chdir(ROOTDIR);

    pthread_t read_tids[MAXCLIENTNUM];
    struct sockaddr_in server_addr;

    memset(newfds, -1, sizeof(newfds));

    server_addr.sin_family = AF_INET;          // IPV4
    server_addr.sin_addr.s_addr = INADDR_ANY; // Bind INADDR_ANY address
    server_addr.sin_port = htons((uint16_t) linkPort);   // notice the htons
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        exit(1); // error in socket preparation
    }
    int sock_reuse = 1;
    ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &sock_reuse, sizeof(sock_reuse));
    if (ret == -1) {
        // printf("Couldn't setsockopt(TCP_NODELAY)\n");
    }
    ret = bind(sockfd, (struct sockaddr *) (&server_addr), sizeof(server_addr));
    // perror("server");
    if (ret < 0) {
        exit(2); // bind the socket with the addr error
    }
    ret = listen(sockfd, 10);
    if (ret < 0) {
        exit(3); // listen error
    }
    // set random seed
    srand((unsigned) time(NULL));

    // can use the thread pool to optimize the multi-thread program, (if I have some spare time...)
    while (1) {
        // waiting for connect
        // printf("waiting for client to connect...\n");
        fflush(stdout);
        newfds[cur_thread_id] = accept(sockfd, NULL, NULL);
        if (newfds[cur_thread_id] < 0) {
            exit(4); // fail to connect
        }
        struct sockaddr_in getsockaddr;
        int len = sizeof(struct sockaddr_in);
        getsockname(newfds[cur_thread_id], (struct sockaddr *) &getsockaddr, (socklen_t *) &len);
        strcpy(SERVERIP, inet_ntoa(getsockaddr.sin_addr));

        // connect success
        char msgOk[] = "220 Anonymous FTP server ready.\r\n";
        sendMsg(newfds[cur_thread_id], msgOk);

        // new a thread to send / recv data from the client...
        struct cmdThreadParameters para;
        para.newfd = newfds[cur_thread_id];
        pthread_create(&read_tids[cur_thread_id], NULL, cmdSocket, &para);

        // prepare the next socket for another client...
        cur_thread_id++; // send over successfully

        // after the thread pool over, it could be ok.
        if (cur_thread_id >= MAXCLIENTNUM) {
            // printf("Too many clients (over 100)... Server has to shut...");
            fflush(stdout);
            break;
        }
    }
    return 0;
}