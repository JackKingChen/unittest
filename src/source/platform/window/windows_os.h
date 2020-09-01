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
#ifndef __WINDOWS_OS_H__
#define __WINDOWS_OS_H__

/*
* host header
*/
#include "host.h"

/*
* win32 header
*/
#ifdef OS_WINDOWS_AFX
/************************************************************************/
/*                                                                      */
/************************************************************************/
#ifndef _SECURE_ATL
#define _SECURE_ATL     1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#ifndef WINVER
#define WINVER          0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT    0x0501
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS  0x0410
#endif

#ifndef _WIN32_IE
#define _WIN32_IE       0x0600
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>       // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>         // MFC support for Windows Common Controls
#endif

#include <afxsock.h>        // MFC socket extensions


#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif


/************************************************************************/
/*                                                                      */
/************************************************************************/
#else 
#include <windows.h>
#include <process.h>
#include <wincon.h>

#endif /*OS_WINDOWS_AFX*/

/************************************************************************/
/*                                                                      */
/************************************************************************/
/*
* win32 lib
*/
#pragma comment( lib, "ws2_32.lib"  )

/************************************************************************/
/*                                                                      */
/************************************************************************/


/************************************************************************/
/*                                                                      */
/************************************************************************/
#endif /*__WINDOWS_OS_H__*/
