
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

#include <Windows.h>

int  regRead(HKEY hkey, LPCTSTR subKey, LPCTSTR valuename, wchar_t* value)
{
    long    lRet;
    HKEY    kRet;
    DWORD   dwtype;
    DWORD   dwSize = 1000 ;

    lRet= RegOpenKeyEx(hkey, subKey, 0, KEY_READ, &kRet);

    if(lRet == ERROR_SUCCESS)
    {
        lRet = RegQueryValueExW(kRet, valuename, NULL, &dwtype, (LPBYTE)value, &dwSize);
        RegCloseKey(kRet);

        return 0;
    }

    return GetLastError();
}

int  regRead(HKEY hkey, LPCTSTR subKey, LPCTSTR valuename, int *value)
{
    long    lRet;
    HKEY    kRet;
    DWORD   dwtype;
    DWORD   dwSize = 1000 ;
    lRet= RegOpenKeyEx(hkey, subKey, 0, KEY_READ, &kRet);

    if(lRet == ERROR_SUCCESS)
    {
        lRet = RegQueryValueExW(kRet, valuename, NULL, &dwtype, (LPBYTE)value, &dwSize);
        RegCloseKey(kRet);

        return 0;
    }

    return GetLastError();
}

int  regRead(HKEY hKey, LPCTSTR lpSubKey, LPCTSTR lpName, LPBYTE pbuf, DWORD ValueSize)
{
    int  nRet = 0;

    HKEY hKeyTmp;

    LONG lRet = RegOpenKeyEx(hKey, lpSubKey, 0, KEY_READ, &hKeyTmp);
    if(lRet==ERROR_SUCCESS)
    {
        DWORD dwType = REG_BINARY;
        if(RegQueryValueEx(hKeyTmp, lpName, NULL, &dwType,(LPBYTE)pbuf, &ValueSize)!=ERROR_SUCCESS)
        {
            nRet = GetLastError();
        }
        RegCloseKey(hKeyTmp);
    }

    return nRet;
}

int  regRead64(HKEY hkey, LPCTSTR subKey, LPCTSTR valuename, wchar_t* value)
{
    long    lRet;
    HKEY    kRet;
    DWORD   dwtype;
    DWORD   dwSize = 1000 ;

    lRet= RegOpenKeyEx(hkey, subKey, 0, KEY_READ|KEY_WOW64_64KEY , &kRet);

    if(lRet == ERROR_SUCCESS)
    {
        lRet = RegQueryValueExW(kRet, valuename, NULL, &dwtype, (LPBYTE)value, &dwSize);
        RegCloseKey(kRet);

        return 0;
    }

    return GetLastError();
}

int  regRead32(HKEY hkey, LPCTSTR subKey, LPCTSTR valuename, wchar_t* value)
{
    long    lRet;
    HKEY    kRet;
    DWORD   dwtype;
    DWORD   dwSize = 1000 ;

    lRet= RegOpenKeyEx(hkey, subKey, 0, KEY_READ|KEY_WOW64_32KEY , &kRet);

    if(lRet == ERROR_SUCCESS)
    {
        lRet = RegQueryValueExW(kRet, valuename, NULL, &dwtype, (LPBYTE)value, &dwSize);
        RegCloseKey(kRet);

        return 0;
    }

    return GetLastError();
}

int  regWrite(HKEY hkey, LPCWSTR subkey, LPCTSTR valuename,LPCTSTR value)
{
    HKEY    hKey;
    DWORD   dwtype;
    long    lRet;
    wchar_t dirResult[MAX_PATH] = {0};
    DWORD   dwSize = 1000;
    DWORD   dwRet = REG_OPENED_EXISTING_KEY;

    lRet= RegCreateKeyEx(hkey, subkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwRet);
    if(lRet == ERROR_SUCCESS)
    {
        lRet = RegQueryValueExW(hKey, valuename, NULL, &dwtype, (LPBYTE)dirResult, &dwSize);

        if (wcscmp(dirResult,value) != 0)
        {
            lRet = RegSetValueEx(hKey, valuename, 0, REG_SZ, (const BYTE *)value, wcslen(value)*2+1);
            if (lRet!=ERROR_SUCCESS)
            {
                int nErrCode = GetLastError();
                RegCloseKey(hKey);
                return nErrCode;
            }
        }
        RegCloseKey(hKey);
    }

    return 0;
}

int  regWrite(HKEY hkey, LPCWSTR subkey, LPCTSTR valuename, DWORD value)
{
    HKEY    hKey;
    DWORD   dwtype;
    long    lRet;
    DWORD   dirResult = 64;
    DWORD   dwSize = MAX_PATH;
    DWORD   dwRet = REG_OPENED_EXISTING_KEY;

    lRet= RegCreateKeyEx(hkey, subkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwRet);
    if(lRet == ERROR_SUCCESS)
    {
        lRet = RegQueryValueExW(hKey, valuename, NULL, &dwtype, (LPBYTE)&dirResult, &dwSize);

        if (lRet!=ERROR_SUCCESS || dirResult != value)
        {
            lRet = RegSetValueEx(hKey, valuename, 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
            if ( lRet!= ERROR_SUCCESS)
            {
                RegCloseKey(hKey);
                return GetLastError();
            }
        }
        RegCloseKey(hKey);
    }

    return 0;
}

int  regWrite64(HKEY hkey, LPCWSTR subkey, LPCTSTR valuename,LPCTSTR value)
{
    HKEY    hKey;
    DWORD   dwtype;
    long    lRet;
    wchar_t dirResult[MAX_PATH] = {0};
    DWORD   dwSize = 1000;
    DWORD   dwRet = REG_OPENED_EXISTING_KEY;

    lRet= RegCreateKeyEx(hkey, subkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS|KEY_WOW64_64KEY, NULL, &hKey, &dwRet);
    if(lRet == ERROR_SUCCESS)
    {
        lRet = RegQueryValueExW(hKey, valuename, NULL, &dwtype, (LPBYTE)dirResult, &dwSize);

        if (lRet!=ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return GetLastError();
        }

        if (wcscmp(dirResult,value) != 0)
        {
            lRet = RegSetValueEx(hKey, valuename, 0, REG_SZ, (const BYTE *)value, wcslen(value)*2+1);
            if (lRet!=ERROR_SUCCESS)
            {
                RegCloseKey(hKey);
                return GetLastError();
            }
        }
        RegCloseKey(hKey);
    }

    return 0;
}

int  regWrite64(HKEY hkey, LPCWSTR subkey, LPCTSTR valuename, DWORD value)
{
    HKEY    hKey;
    DWORD   dwtype;
    long    lRet;
    DWORD   dirResult = 64;
    DWORD   dwSize = MAX_PATH;
    DWORD   dwRet = REG_OPENED_EXISTING_KEY;

    lRet= RegCreateKeyEx(hkey, subkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS|KEY_WOW64_64KEY, NULL, &hKey, &dwRet);
    if(lRet == ERROR_SUCCESS)
    {
        lRet = RegQueryValueExW(hKey, valuename, NULL, &dwtype, (LPBYTE)&dirResult, &dwSize);

        if (lRet!=ERROR_SUCCESS || dirResult != value)
        {
            lRet = RegSetValueEx(hKey, valuename, 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
            if ( lRet!= ERROR_SUCCESS)
            {
                RegCloseKey(hKey);
                return GetLastError();
            }
        }
        RegCloseKey(hKey);
    }

    return 0;
}


int  regDelete(HKEY hkey, LPCWSTR subkey, LPCTSTR valuename)
{
    HKEY retKey;
    LONG regRet = RegOpenKeyEx(hkey,subkey,0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE|KEY_SET_VALUE,&retKey);

    if (regRet==ERROR_SUCCESS)
    {
        int ret = RegDeleteValue(retKey,valuename);

        RegCloseKey(retKey);

        if (ret != ERROR_SUCCESS && ret != 2)
        {
            return GetLastError();
        }

        return 0;
    }

    return 0;
}

int  regDelete64(HKEY hkey, LPCWSTR subkey, LPCTSTR valuename)
{
    HKEY retKey;
    LONG regRet = RegOpenKeyEx(hkey,subkey,0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE|KEY_SET_VALUE|KEY_WOW64_64KEY,&retKey);

    if (regRet==ERROR_SUCCESS)
    {
        int ret = RegDeleteValue(retKey,valuename);

        RegCloseKey(retKey);

        if (ret != ERROR_SUCCESS && ret != 2)
        {
            return GetLastError();
        }

        return 0;
    }

    return 0;
}
