/********************************************************************
*
*    DESCRIPTION:
*
*    AUTHOR: 
*
*    HISTORY:
*
*    DATE:2012-04-21
*
*******************************************************************/
/*for stander*/
#include <stdlib.h>
#include <string.h>

/*for local*/
#include "unittest.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/
#define CONFIG_PROGRAM "unittest"
#define CONFIG_VERSION "1.0.0.0"


/************************************************************************/
/*                     static function                                  */
/************************************************************************/
static void display_help (void)
{
    /*
    * show version
    */
    printf("runtest:\n");
    printf(CONFIG_PROGRAM " " CONFIG_VERSION"\n"
        "[" __DATE__ " " __TIME__ "]\n"
        "      --help            display this help and exit\n"
        "      --version         output version information and exit\n"
        "      --exec            execute command line\n"
        "\n"
        );
    exit(0);
}

static void display_version (void)
{
    printf(CONFIG_PROGRAM " " CONFIG_VERSION"\n"
        "[" __DATE__ " " __TIME__ "]\n"
        ""
        "\n");
    exit(0);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

#define ADDR_MAX_NUM 100
#include <execinfo.h>
void call_back_signal(int sig_no)
{
    printf("CALLBACK: SIGNAL: %d\n", sig_no);
    void *pBuf[ADDR_MAX_NUM] = {0};
    int addr_num = backtrace(pBuf, ADDR_MAX_NUM);
    printf("BACKTRACE : NUMBER OF ADDRESS IS %d\n\n", addr_num);
    char** str_symbols = backtrace_symbols(pBuf, addr_num);

    if(str_symbols == nullptr)
    {
        printf("BACKTRACE : CANNOT GET BACKTRACE SYMBOLS\n");
        return;
    }

    int i = 0;
    for(i = 0; i < addr_num; i++)
    {
        printf("%03d %s\n", addr_num  - i, str_symbols[i]);
    }
    printf("\n");
    free(str_symbols);
    str_symbols = nullptr;
    exit(1);    // QUIT PROCESS. IF NOT, MAYBE ENDLESS LOOP

}

int main(int argc, const char* argv[])
{
    OSCon the_test("Test$ ");

    srand((unsigned int)time(NULL));

    signal(SIGPIPE, SIG_IGN);           // ignore sigpipe
    signal(SIGSEGV, call_back_signal);  

    /*
    * action
    */
    {
        const char *cmdline;

        if(argc>1)
        {
            if(strcmp(argv[1],"help")==0
                || strcmp(argv[1],"?")==0)
            {
                display_help();
            }
            if(strcmp(argv[1],"version")==0
                || strcmp(argv[1],"v")==0)
            {
                display_version();
            }

            cmdline = argv[1];
        }
        else
        {
            cmdline = NULL;
        }

        if(cmdline)
            return the_test.exe(cmdline,argc-2,argv+2);
        else
            return the_test.run();
    }

    return 0;
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
