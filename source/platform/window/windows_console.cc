/*/*******************************************************************
*
*    DESCRIPTION:
*
*    AUTHOR: 
*
*    HISTORY:
*
*    DATE:2013-04-07
*
*******************************************************************/

#include <windows.h>
#include <process.h>
#include <wincon.h>

#include "host.h"

namespace host
{
    /************************************************************************/
    /* class of OSTTY                                                       */
    /************************************************************************/
    static void tty_config(DWORD *oldIO)
    {
        HANDLE handle;
        /*
        * setup io
        */
        handle = GetStdHandle(STD_INPUT_HANDLE); 

        GetConsoleMode( handle, oldIO);
        SetConsoleMode( handle, 0);
    }

    static void tty_restore(DWORD *oldIO)
    {
        HANDLE handle;

        /*
        * restore tty
        */
        handle = GetStdHandle(STD_INPUT_HANDLE);
        SetConsoleMode( handle, *oldIO);
    }

    int OSTTY::read(unsigned char *buff,int len)
    {
        HANDLE       handle;
        INPUT_RECORD vConInputRec;
        DWORD        dwReadCount = 0;
        int          total;
        int          key;

        handle = GetStdHandle(STD_INPUT_HANDLE);
        total  = 0;

        while(total<len)
        {
            if(!PeekConsoleInput(handle, &vConInputRec, 1, &dwReadCount))
            {
                Sleep(1);
                continue;
            }
            if (dwReadCount == 0 ||  !ReadConsoleInput( handle, &vConInputRec, 1, &dwReadCount ))
            {
                Sleep(1);
                continue;
            }
            if (dwReadCount == 0 || vConInputRec.EventType != KEY_EVENT || !vConInputRec.Event.KeyEvent.bKeyDown)
            {
                Sleep(1);
                continue;
            }

            switch(vConInputRec.Event.KeyEvent.wVirtualKeyCode)
            {
            case VK_F1:      key = OSTTY::TTY_VK_FUN1;break;
            case VK_F2:      key = OSTTY::TTY_VK_FUN2;break;
            case VK_F3:      key = OSTTY::TTY_VK_FUN3;break;
            case VK_F4:      key = OSTTY::TTY_VK_FUN4;break;
            case VK_F5:      key = OSTTY::TTY_VK_FUN5;break;
            case VK_F6:      key = OSTTY::TTY_VK_FUN6;break;
            case VK_F7:      key = OSTTY::TTY_VK_FUN7;break;
            case VK_F8:      key = OSTTY::TTY_VK_FUN8;break;
            case VK_F9:      key = OSTTY::TTY_VK_FUN9;break;
            case VK_F10:     key = OSTTY::TTY_VK_FUN10;break;
            case VK_F11:     key = OSTTY::TTY_VK_FUN11;break;
            case VK_F12:     key = OSTTY::TTY_VK_FUN12;break;

            case VK_BACK:    key = OSTTY::TTY_VK_BACKSPACE;break;
            case VK_TAB:     key = OSTTY::TTY_VK_TAB;   break;
            case VK_RETURN:  key = OSTTY::TTY_VK_RETURN;break;
            case VK_ESCAPE:  key = OSTTY::TTY_VK_ESC;   break;
            case VK_PRIOR:   key = OSTTY::TTY_VK_RETURN;break;
            case VK_NEXT:    key = OSTTY::TTY_VK_RETURN;break;
            case VK_END:     key = OSTTY::TTY_VK_END;   break;
            case VK_HOME:    key = OSTTY::TTY_VK_HOME;  break;
            case VK_LEFT:    key = OSTTY::TTY_VK_LEFT;  break;
            case VK_UP:      key = OSTTY::TTY_VK_UP;    break;
            case VK_RIGHT:   key = OSTTY::TTY_VK_RIGHT; break;
            case VK_DOWN:    key = OSTTY::TTY_VK_DOWN;  break;
            case VK_INSERT:  key = OSTTY::TTY_VK_INSERT;break;
            case VK_DELETE:  key = OSTTY::TTY_VK_DELETE;break;
            case VK_NUMPAD0: key = '0';break;
            case VK_NUMPAD1: key = '1';break;
            case VK_NUMPAD2: key = '2';break;
            case VK_NUMPAD3: key = '3';break;
            case VK_NUMPAD4: key = '4';break;
            case VK_NUMPAD5: key = '5';break;
            case VK_NUMPAD6: key = '6';break;
            case VK_NUMPAD7: key = '7';break;
            case VK_NUMPAD8: key = '8';break;
            case VK_NUMPAD9: key = '9';break;
            case VK_MULTIPLY:key = '*';break;
            case VK_ADD:     key = '+';break;
            case VK_SUBTRACT:key = '-';break;
            case VK_DECIMAL: key = '.';break;
            case VK_DIVIDE:  key = '/';break;
            default:key = vConInputRec.Event.KeyEvent.uChar.AsciiChar;
            }

            buff[total++] = static_cast<unsigned char>(key);

            /*
            * test for next key
            */
            if(total<len)
            {
                Sleep(1);
                if(!PeekConsoleInput(handle, &vConInputRec, 1, &dwReadCount) || dwReadCount==0)
                {
                    break;
                }
            }
        }

        return total;
    }

    int OSTTY::draw(const char * prompt,int prompt_len,const char * line,int pos)
    {
        HANDLE                     handle = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD                      coord;
        CONSOLE_SCREEN_BUFFER_INFO info;

        /*print line*/
        fputs("\r",  stdout);
        fputs(prompt,stdout);
        fputs(line,  stdout);

        /*get cursor info*/
        GetConsoleScreenBufferInfo(handle,&info);
        /*clear rest of line*/
        for (int i=info.dwCursorPosition.X;i<info.dwMaximumWindowSize.X-1;i++)
            fputs(" ",  stdout);

        /*put cursor*/
        coord.X = prompt_len+pos;
        coord.Y = info.dwCursorPosition.Y;
        SetConsoleCursorPosition(handle,coord);

        fflush(stdout);
        return 0;
    }

    bool OSTTY::color(Color color)
    {
        const HANDLE               stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO buffer_info;
        WORD                       stdout_color;

        switch(color)
        {
        case RED   :stdout_color=FOREGROUND_RED;   break;
        case GREEN :stdout_color=FOREGROUND_GREEN; break;
        case YELLOW:stdout_color=FOREGROUND_RED | FOREGROUND_GREEN;break;
        case WHITE :
        default:
            stdout_color=FOREGROUND_INTENSITY;break;
        }

        // Gets the current text color.
        GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
        SetConsoleTextAttribute(stdout_handle,stdout_color | FOREGROUND_INTENSITY);

        return true;
    }

    /************************************************************************/
    /* class of OSCon                                                       */
    /************************************************************************/
    int OSCon::complete_path(char *word)
    {

        return 0;
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
};

