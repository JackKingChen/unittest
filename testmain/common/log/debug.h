
#pragma once;

/*******************************************************************
*
*    DESCRIPTION:Copyright(c) 2014-2024 KaoPu Network Technology Co,.Ltd
*
*    AUTHOR: chenxianfeng
*
*    HISTORY:
*
*    DATE:2014-12-25
*
*******************************************************************/

#ifndef  MYDEBUG_H_H_H
#define  MYDEBUG_H_H_H

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <fcntl.h>
#include <io.h>
#include <assert.h>
#include <tchar.h>

#define TTDEBUG_ENABLE 1
#if TTDEBUG_ENABLE

/************************************************************************/
/*                                                                      */
/************************************************************************/

FILE* debug_getOutPutFile(void);
bool  debug_init(void);
void  debug_exit(void);

#if TTEBUG_LOG_TOFILE
#define __SET_LOG_ATTR(handle,color)
#else
#define __SET_LOG_ATTR(handle,color) SetConsoleTextAttribute(handle,color)
#endif

#define _PRINT_WITH_COLOR(fore, back, fmt, ...)  \
    do{\
        FILE* fp = debug_getOutPutFile();\
        if (fp){\
            __SET_LOG_ATTR(GetStdHandle(STD_OUTPUT_HANDLE),fore | back << 4);\
            fwprintf(fp,fmt, ##__VA_ARGS__);fflush(fp);\
            __SET_LOG_ATTR(GetStdHandle(STD_OUTPUT_HANDLE),7);\
       }\
    }while(0)

#define LOG_INIT()   debug_init()
#define LOG_EXIT()   debug_exit()
#define LG_ALL(...)  _PRINT_WITH_COLOR(1,0,##__VA_ARGS__)

#if  TTDEBUG_LOG_POSITON
#define _MYDEBUG_LOG_POSITION()  LG_ALL(_T("(%s,%d)\t"),__FUNCTION__,__LINE__)
#else
#define _MYDEBUG_LOG_POSITION()
#endif /*TTDEBUG_LOG_POSITON*/

#if  TTDEBUG_LOG_DETAIL
#define _MYDEBUG_LOG_DETAIL(logType,module)\
    do{\
        SYSTEMTIME sys; \
        GetLocalTime( &sys ); \
        LG_ALL(_T("%s:(%4d-%02d-%02d %02d:%02d:%02d.%03d) [%s] "),logType,sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute, sys.wSecond,sys.wMilliseconds,\
        module);\
    }while(0)
#else
#define _MYDEBUG_LOG_DETAIL(logType,module)
#endif /*TTDEBUG_LOG_DETAIL*/

/************************************************************************/
/*                                                                      */
/************************************************************************/

#define LG_ERR(module,...)  \
        do{\
            if (TTDEBUG_LOG_LEVEL>0){\
                _MYDEBUG_LOG_POSITION();\
                _MYDEBUG_LOG_DETAIL(_T("Fatal"),module);\
                _PRINT_WITH_COLOR(4,0,##__VA_ARGS__);\
            }\
        }while(0)
#define LG_WARN(module,...)  \
        do{\
            if (TTDEBUG_LOG_LEVEL>0){\
                _MYDEBUG_LOG_POSITION();\
                _MYDEBUG_LOG_DETAIL(_T("Warning"),module);\
                _PRINT_WITH_COLOR(6,0,##__VA_ARGS__);\
            }\
        }while(0)
#define LG_INFO(module,...)  \
        do{\
            if (TTDEBUG_LOG_LEVEL>0){\
                _MYDEBUG_LOG_POSITION();\
                _MYDEBUG_LOG_DETAIL(_T("info"),module);\
                _PRINT_WITH_COLOR(7,0,##__VA_ARGS__);\
            }\
        }while(0)

#define LG_DBUG(module,...)  \
        do{\
            if (TTDEBUG_LOG_LEVEL>0){\
                _MYDEBUG_LOG_POSITION();\
                _MYDEBUG_LOG_DETAIL(_T("Debug"),module);\
                _PRINT_WITH_COLOR(5,0,##__VA_ARGS__);\
            }\
        }while(0)


#else /*MYDEBUG_ENBLE*/

#define LG_ALL(...)
#define LG_ERR(...)
#define LG_WARN(...)
#define LG_INFO(...)
#define LG_DBUG(...)

#endif /*MYDEBUG_ENBLE*/

/************************************************************************/
/*                                                                      */
/************************************************************************/

#endif /*MYDEBUG_H_H_H*/




