#include "cmd.h"

//================================================================================================================================================
// process cmd, especially whether the cmd is valid or not.
//================================================================================================================================================

// get the cmd type: if not valide, then set the ERROR type.
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
    const int STDLEN = 2;
    // temLen may be change
    int temLen = 0;
    char **temStr = split(cmd, ", ", &temLen); // ',' and ' ' split

    int ans = 1;
    // need to check if out of ROOTDIR -> ROOTDIR is the prefix of the cwd para
    // need to check if that para is file
    if (temLen == STDLEN && prefixCorrect(temStr[0], "RETR") && prefixCorrect(temStr[1], ROOTDIR) && isDirectory(temStr[1]) == 0)
    {
        ans = 1;
    }
    else
    {
        ans = 0;
    }

    deleteCharArr2(temStr, temLen);
    return ans;
}

// STOR success
int storSucc(const char *cmd)
{
    const int STDLEN = 2;
    // temLen may be change
    int temLen = 0;
    char **temStr = split(cmd, ", ", &temLen); // ',' and ' ' split

    int ans = 1;
    // need to check if out of ROOTDIR -> ROOTDIR is the prefix of the cwd para
    // need to check if that file does not exist
    if (temLen == STDLEN && prefixCorrect(temStr[0], "STOR"))
    {
        ans = 1;
    }
    else
    {
        ans = 0;
    }

    deleteCharArr2(temStr, temLen);
    return ans;
}

// QUIT success
int quitSucc(const char *cmd)
{
    char scale[] = "QUIT";

    if (strcmp(cmd, scale) == 0)
    {
        return 1;
    }
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
    const int STDLEN = 2;
    // temLen may be change
    int temLen = 0;
    char **temStr = split(cmd, ", ", &temLen); // ',' and ' ' split

    int ans = 1;
    // need to check if out of ROOTDIR -> ROOTDIR is the prefix of the cwd para
    if (temLen == STDLEN && prefixCorrect(temStr[0], "MKD") && prefixCorrect(temStr[1], ROOTDIR))
    {
        // mkdir that dir
        int mkSucc = mkdir(temStr[1], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (mkSucc != 0)
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

// CWD success
int cwdSucc(const char *cmd)
{
    const int STDLEN = 2;
    // temLen may be change
    int temLen = 0;
    char **temStr = split(cmd, ", ", &temLen); // ',' and ' ' split

    int ans = 1;
    // need to check if out of ROOTDIR -> ROOTDIR is the prefix of the cwd para
    // need to check if that para is directory
    if (temLen == STDLEN && prefixCorrect(temStr[0], "CWD") && prefixCorrect(temStr[1], ROOTDIR) && isDirectory(temStr[1]) == 1)
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
    const int STDLEN = 2;
    // temLen may be change
    int temLen = 0;
    char **temStr = split(cmd, ", ", &temLen); // ',' and ' ' split

    int ans = 1;
    // need to check if out of ROOTDIR -> ROOTDIR is the prefix of the cwd para
    // need to check if that para is file
    if (temLen == STDLEN && prefixCorrect(temStr[0], "RNFR") && prefixCorrect(temStr[1], ROOTDIR) && isDirectory(temStr[1]) == 0)
    {
        ans = 1;
    }
    else
    {
        ans = 0;
    }

    deleteCharArr2(temStr, temLen);
    return ans;
}

// RNTO success. We need more details then.
int rntoSucc(const char *cmd)
{
    return 1;
}