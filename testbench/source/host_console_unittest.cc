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

/*for unittest*/
#include "unittest.h"

/*for local*/
#include "host.h"

using namespace host;

/************************************************************************/
/*                                                                      */
/************************************************************************/
#define  ONEKEY(x) \
    case OSTTY::x : \
    { \
    printf(#x"\n");break;\
    }

/************************************************************************/
/*                                                                      */
/************************************************************************/
UNITTEST(console)
{
    int key;

    while(OSTTY::getkey(key))
    {
        switch(key)
        {
            ONEKEY(TTY_VK_BACKSPACE)
            ONEKEY(TTY_VK_ENTER)
            ONEKEY(TTY_VK_ESC)
            ONEKEY(TTY_VK_SPACE)
            ONEKEY(TTY_VK_UP)
            ONEKEY(TTY_VK_DOWN)
            ONEKEY(TTY_VK_RIGHT)
            ONEKEY(TTY_VK_LEFT)
            ONEKEY(TTY_VK_HOME)
            ONEKEY(TTY_VK_END)
            ONEKEY(TTY_VK_INSERT)
            ONEKEY(TTY_VK_DELETE)
            ONEKEY(TTY_VK_PAGEUP)
            ONEKEY(TTY_VK_PAGEDOWN)
            ONEKEY(TTY_VK_FUN1)
            ONEKEY(TTY_VK_FUN2)
            ONEKEY(TTY_VK_FUN3)
            ONEKEY(TTY_VK_FUN4)
            ONEKEY(TTY_VK_FUN5)
            ONEKEY(TTY_VK_FUN6)
            ONEKEY(TTY_VK_FUN7)
            ONEKEY(TTY_VK_FUN8)
            ONEKEY(TTY_VK_FUN9)
            ONEKEY(TTY_VK_FUN10)
            ONEKEY(TTY_VK_FUN11)
            ONEKEY(TTY_VK_FUN12)

        default:
            {
                printf("OSTTY::getkey(%d)\n",key);
            }
        }
    }
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
