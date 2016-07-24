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
#ifndef __HOST_H__
#define __HOST_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <math.h>

#include <string>
#include <istream>
#include <ostream>
#include <iostream>
#include <vector>
#include <list>
#include <algorithm>

/************************************************************************/
/* OS platform type                                                     */
/************************************************************************/
#undef OS_WINDOWS
#undef OS_LINUX
#undef OS_MAC

/*depends on makefile*/
#ifdef  WIN32
#define  OS_WINDOWS     1
#define  OS_WINDOWS_AFX 1
#include <io.h>
#else
#define  OS_LINUX       1
#endif

/************************************************************************/
/* OS fixup                                                             */
/************************************************************************/
#ifdef OS_WINDOWS
#define strcasecmp   _stricmp
#define strncasecmp  _strnicmp 
#define access       _access 
#endif

#ifndef TABLESIZE
#define TABLESIZE(_a) (sizeof(_a)/sizeof(_a[0]))
#endif

/************************************************************************/
/*                                                                      */
/************************************************************************/
#include "host_os.h"
#include "host_console.h"
#include "host_media.h"
#include "host_netjib.h"
#include "host_netfile.h"
#include "host_csvfile.h"
#include "host_device.h"
#include "host_sip.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/
using namespace host;

/************************************************************************/
/*                                                                      */
/************************************************************************/
#endif /*__HOST_H__*/
