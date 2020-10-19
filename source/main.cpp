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

int main(int argc, const char* argv[])
{
    OSCon the_test("Test$ ");

    srand((unsigned int)time(NULL));

    signal(SIGPIPE, SIG_IGN);           // ignore sigpipe

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
