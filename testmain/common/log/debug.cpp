/*******************************************************************
*
*    DESCRIPTION:Copyright(c) 2014-2024 KaoPu Network Technology Co,.Ltd
*
*    AUTHOR: chenxianfeng
*
*    HISTORY:
*
*    DATE:2014-08-15
*
*******************************************************************/


#include "debug.h"

#include "utils/regutil.h"

#include "model.h"

static int getTtInstallPath(wchar_t* szTtInstallPath)
{
    if (regRead32(HKEY_LOCAL_MACHINE,_T("SOFTWARE\\TianTian\\Setup"),_T("InstallPath"),szTtInstallPath)==0)
    {
        return -1;
    }

    return 0;
}

FILE* debug_getOutPutFile(void)
{
    static bool  bInit = false;
    static FILE* outfp = NULL;

#if TTEBUG_LOG_TOFILE
    if (!bInit)
    {
        bInit = true;

        wchar_t szInstallPath[MAX_PATH] = { 0 };
        wchar_t szLogFile[MAX_PATH]  = { 0 };

        if (getTtInstallPath(szInstallPath)<0)
            return NULL;

        wsprintf(szLogFile,_T("%s\\log\\TianTian.log"),szInstallPath);

        outfp = _wfopen(szLogFile,_T("w+"));
    }
#else
    if (!bInit)
    {
        bInit = true;
    }
    outfp = stdout;
#endif

    return outfp;
}

bool debug_init(void)
{
    if (!debug_getOutPutFile())
        return false;

    return true;
}

void debug_exit(void)
{
    FILE* fp = debug_getOutPutFile();
    if (fp)
    {
#if TTEBUG_LOG_TOFILE
        fclose(fp);
#else
        //FreeConsole();
#endif
    }
}
/************************************************************************/
/*                                                                      */
/************************************************************************/

