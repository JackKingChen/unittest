
/*******************************************************************
*
*    DESCRIPTION:Copyright(c) 2014-2024 KaoPu Network Technology Co,.Ltd
*
*    AUTHOR: chenxianfeng (ebdsoft@163.com)
*
*    HISTORY:
*
*    DATE:2015-12-02
*
*******************************************************************/

#pragma once

#ifndef WIN32_REGEDIT_H
#define WIN32_REGEDIT_H


/*
 * describe : win32 read register,value should long enough (MAX_PATH)
 */
int  regRead(HKEY hkey, LPCTSTR subKey, LPCTSTR valuename, wchar_t* value);
int  regRead(HKEY hkey, LPCTSTR subKey, LPCTSTR valuename, int* value);
int  regRead64(HKEY hkey, LPCTSTR subKey, LPCTSTR valuename, wchar_t* value);
int  regRead32(HKEY hkey, LPCTSTR subKey, LPCTSTR valuename, wchar_t* value);
int  regRead(HKEY hKey, LPCTSTR lpSubKey, LPCTSTR lpName, LPBYTE pbuf, DWORD ValueSize);
int  regWrite(HKEY hkey, LPCWSTR subkey, LPCTSTR valuename,LPCTSTR value);
int  regWrite(HKEY hkey, LPCWSTR subkey, LPCTSTR valuename, DWORD value);
int  regWrite64(HKEY hkey, LPCWSTR subkey, LPCTSTR valuename,LPCTSTR value);
int  regWrite64(HKEY hkey, LPCWSTR subkey, LPCTSTR valuename, DWORD value);
int  regDelete(HKEY hkey, LPCWSTR subkey, LPCTSTR valuename);
int  regDelete64(HKEY hkey, LPCWSTR subkey, LPCTSTR valuename);


#endif /*WIN32_REGEDIT_H*/
