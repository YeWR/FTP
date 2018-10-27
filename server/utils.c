#include "utils.h"

// print a array
void printArr(char **arr, const int len)
{
    int i = 0;
    printf("************************************\n");
    fflush(stdout);
    for (i = 0; i < len; ++i)
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
    server_addr.sin_family = AF_INET;         // IPV4
    server_addr.sin_addr.s_addr = INADDR_ANY; // Bind INADDR_ANY address
    server_addr.sin_port = htons(port);       // notice the htons
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

    for (i = 0; hent->h_addr_list[i]; i++)
    {
        if (strcmp(inet_ntoa(*(struct in_addr *)(hent->h_addr_list[i])), "127.0.0.1") != 0)
        {
            strcpy(ip, inet_ntoa(*(struct in_addr *)(hent->h_addr_list[i])));
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

// is a directory or a file: 1 -> directory, 0 -> file
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
        if (strlen(ip) > 15)
        {
            int i = 1;
            int ip_index = 0;
            for (i = 1; i <= 4; ++i)
            {
                strcpy(ip + ip_index, temStr[i]);
                ip_index += strlen(temStr[i]);
                if (i != 4)
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

// get the info of a file or a directory through its stat
// drwxrwxr-x    2 1039     1039         4096 Oct 26 18:39 videos
// -rw-rw-r--    1 1039     1039            0 Oct 26 18:39 README.txt
void getStatInfo(const struct stat buf, const char *fileName, char *msg)
{
    struct tm *p;
    char file_type[11] = {0};
    char t_buffer[128] = {0};

    strcpy(file_type, "----------");
    switch (buf.st_mode & S_IFMT)
    {
    case S_IFSOCK:
        file_type[0] = 's';
        break;
    case S_IFLNK:
        file_type[0] = 'l';
        break;
    case S_IFBLK:
        file_type[0] = 'b';
        break;
    case S_IFDIR:
        file_type[0] = 'd';
        break;
    case S_IFCHR:
        file_type[0] = 'c';
        break;
    case S_IFIFO:
        file_type[0] = 'f';
        break;
    default:
        break;
    }
    if (buf.st_mode & S_IRUSR)
    {
        file_type[1] = 'r';
    }
    if (buf.st_mode & S_IWUSR)
    {
        file_type[2] = 'w';
    }
    if (buf.st_mode & S_IXUSR)
    {
        file_type[3] = 'x';
    }
    if (buf.st_mode & S_IRGRP)
    {
        file_type[4] = 'r';
    }
    if (buf.st_mode & S_IWGRP)
    {
        file_type[5] = 'w';
    }
    if (buf.st_mode & S_IXGRP)
    {
        file_type[6] = 'x';
    }
    if (buf.st_mode & S_IROTH)
    {
        file_type[7] = 'r';
    }
    if (buf.st_mode & S_IWOTH)
    {
        file_type[8] = 'w';
    }
    if (buf.st_mode & S_IXOTH)
    {
        file_type[9] = 'x';
    }
    p = gmtime((time_t *)&(buf.st_mtim));
    strftime(t_buffer, 128, "%b %e %H:%M", p);
    sprintf(msg, "%s  %3d %-8d %-8d %8lu %s %s\r\n", file_type,
            (int)buf.st_nlink, buf.st_uid, buf.st_gid, (long)buf.st_size,
            t_buffer, fileName);
}