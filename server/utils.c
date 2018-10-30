#include "utils.h"

//================================================================================================================================================
// extra functions
//================================================================================================================================================

// get the argv to server
int getArgv(const int argc, const char **argv, int *port, char *root)
{
    if(argc == 5)
    {
        if(strcmp(argv[1], "-root") ==0 )
        {
            strcpy(root, argv[2]);
        }
        else if(strcmp(argv[1], "-port") == 0)
        {
            *port = atoi(argv[2]);
        }

        if(strcmp(argv[3], "-root") ==0 )
        {
            strcpy(root, argv[4]);
        }
        else if(strcmp(argv[3], "-port") == 0)
        {
            *port = atoi(argv[4]);
        }
        return 1;
    }
    return 0;
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

// print a array
void printArr(char **arr, const int len)
{
    int i = 0;
    // printf("************************************\n");
    fflush(stdout);
    for (i = 0; i < len; ++i)
    {
        // printf("%s\n", arr[i]);
        fflush(stdout);
    }
    // printf("************************************\n");
    fflush(stdout);
    fflush(stdout);
}

// strip, clear the \r\n
void strip(char *cmd)
{
    int len = strlen(cmd);
    if (len < 2)
    {
        return;
    }
    if (cmd[len - 2] == '\r' && cmd[len - 1] == '\n')
    {
        cmd[len - 2] = '\0';
        cmd[len - 1] = '\0';
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

// create a socket listening on the given port, waiting...
int acceptSocket(const int port)
{
    int newfd;
    int sockfd;
    int ret;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;         // IPV4
    server_addr.sin_addr.s_addr = INADDR_ANY; // Bind INADDR_ANY address
    server_addr.sin_port = htons(port);       // notice the htons
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        return -1;
        //exit(1); // error in socket preparation
    }
    ret = bind(sockfd, (struct sockaddr *)(&server_addr), sizeof(server_addr));
    // perror("server");

    if (ret < 0)
    {
        return -1;
        //exit(2); // bind the socket with the addr error
    }

    ret = listen(sockfd, 10);
    if (ret < 0)
    {
        return -1;
        //exit(3); // listen error
    }

    newfd = accept(sockfd, NULL, NULL);
    sleep(1);
    if (newfd < 0)
    {
        return -1;
        //exit(4);
    }

    return newfd;
}

// create a socket to connect to the given ip and port, waiting...
int connectToSocket(const char *ip, const int port)
{
    int sockfd;
    int ret;
    struct sockaddr_in addr;

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        return -1;
        // exit(1); // error in socket preparation
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    ret = inet_pton(AF_INET, ip, &addr.sin_addr);
    if (ret <= 0)
    {
        return -1;
        //exit(2);
    }

    ret = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        return -1;
        //exit(3);
    }
    return sockfd;
}

// set server port randomly and return the random port
int setServerPort()
{
    int delta = 65536 - 20000;
    int port = 2000 + (rand() % delta); // 2000 + [0, 45536) -> [2000. 65535]
    return port;
}

// prefix is the prefix of source
int prefixCorrect(const char *source, const char *prefix)
{
    int len = strlen(prefix) + 1;
    if (strlen(source) < len - 1)
    {
        return 0;
    }
    char *cnt = (char *)malloc(sizeof(char) * len);
    strncpy(cnt, source, len - 1);
    cnt[len - 1] = '\0';

    int ans = 0;
    if (strcmp(cnt, prefix) == 0)
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
    if (len == 0)
    {
        return 0;
    }
    int i = 0;
    for (i = 0; i < len; ++i)
    {
        if (!isdigit(source[i]))
        {
            return 0;
        }
    }
    return 1;
}

// is a directory or a file: 1 -> directory, 0 -> file, -1 -> not exist.
int isDirectory(const char *path)
{
    struct stat buf;
    if (lstat(path, &buf) < 0)
    {
        return -1;
    }
    if (S_ISDIR(buf.st_mode))
        return 1;
    else if (S_ISREG(buf.st_mode))
        return 0;
    return -1;
}

// file exists under the root: 1 -> exists, 0 -> not
int isFileExists(const char *path)
{
    int len = strlen(path);
    int ans = 0;
    if (len > 0)
    {
        int access_res = access(path, F_OK);

        // exists
        if (access_res == 0)
        {
            char abs_path[100];
            memset(abs_path, 0, sizeof(abs_path));
            realpath(path, abs_path);

            // under the root
            if(prefixCorrect(abs_path, ROOTDIR))
            {
                ans = 1;
            }
        }
    }
    return ans;
}

// judge whether the path is relative or not: 1 -> is relative 0 -> is absolute.
int isRelativePath(const char *path)
{
    int len = strlen(path);
    if (len > 0)
    {
        if (path[0] == '/')
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return -1;
    }
}

// judge the path is accessible under the root: 1 -> is accessible, 0 -> not(the path is a dir) 
int isAccessiblePath(const char *path)
{
    int len = strlen(path);
    int access = 0;
    if (len > 0)
    {
        char curPath[100];
        memset(curPath, 0, sizeof(curPath));
        getDir(curPath);
        int cdSucc = chdir(path);

        // can cd to that path
        if (cdSucc == 0)
        {
            char newPath[100];
            memset(newPath, 0, sizeof(newPath));
            getDir(newPath);
            // ROOTDIR is new path is the prefix
            if (prefixCorrect(newPath, ROOTDIR))
            {
                access = 1;
            }
        }

        // do not forget to chdir back to the current path.
        chdir(curPath);
    }
    return access;
}

// get the ip and port in PORT cmd if the cmd is verified.
void getIpPort(const char *cmd, char *ip, int *port)
{
    // [PORT xxx.xxx.xxx.xxx xxx,xxx] len => 7
    const int STDLEN = 7;
    // temLen may be change
    int temLen = 0;
    char **temStr = split(cmd, ", ", &temLen); // ',' and ' ' split

    if (temLen == STDLEN)
    {
        sprintf(ip, "%s.%s.%s.%s", temStr[1], temStr[2], temStr[3], temStr[4]);

        int temPort = 256 * atoi(temStr[5]);
        temPort += atoi(temStr[6]);

        *port = temPort;
    }

    deleteCharArr2(temStr, temLen);
}

// set the current directory in dir
void getDir(char *dir)
{
    char temDir[100];
    memset(temDir, 0, sizeof(temDir));
    getcwd(temDir, sizeof(temDir));
    strcpy(dir, temDir);
}

// get filename from cmd like: XXX filename
void getFileName(const char *cmd, char *fileName)
{
    const int STDLEN = 2;
    // temLen may be change
    int temLen = 0;
    char **temStr = split(cmd, ", ", &temLen); // ',' and ' ' split

    if (temLen == STDLEN)
    {
        strcpy(fileName, temStr[1]);
    }

    deleteCharArr2(temStr, temLen);
}

// get the size of a file
int getFileSize(const char *fileName)
{
    struct stat statbuf;
    stat(fileName, &statbuf);
    int size = statbuf.st_size;

    return size;
}

// mkdir the path: 0 -> cannot, 1 -> mkdir success
int mkdirPath(const char *dir)
{
    int mkSucc = mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    int ans = 0;

    // success
    if(mkSucc == 0)
    {
        ans = 1;   
    }

    return ans;
}