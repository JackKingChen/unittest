
#include <stdio.h>

#include <string>

#include <Windows.h>

#include <tlhelp32.h>
#include <Psapi.h>

#include <tchar.h>

bool createProcess(LPTSTR lpszCmd, DWORD &dwExitCode, wchar_t *cResult, DWORD dwMSTime, BOOL bIsHide, LPCTSTR lpszCurrentDir)
{
    SECURITY_ATTRIBUTES sa; 
    HANDLE hRead=NULL, hWrite=NULL;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES); 
    sa.lpSecurityDescriptor = NULL; 
    sa.bInheritHandle = TRUE; 

    if(cResult != NULL)
    {
        if (!CreatePipe(&hRead, &hWrite, &sa, 0)) 
        {
            return FALSE; 
        }
    }

    STARTUPINFO si = {0};
    si.cb = sizeof(STARTUPINFO);
    GetStartupInfo(&si);

    si.wShowWindow = bIsHide ? SW_HIDE : SW_SHOW;
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.hStdError = hWrite;
    si.hStdOutput = hWrite;


    PROCESS_INFORMATION pi = {0};

    BOOL bOK = ::CreateProcess(NULL, lpszCmd, NULL, NULL, TRUE, 0, NULL, lpszCurrentDir, &si, &pi);
    CloseHandle(hWrite); 
    if(dwMSTime==0 || !bOK)
    {
        CloseHandle(hRead); 
        goto END;
    }

    if(dwMSTime != INFINITE)
    {
        DWORD dwRes = WaitForSingleObject(pi.hProcess, dwMSTime);
        if(WAIT_TIMEOUT != dwRes)
        {
        }
        else
        {
            TerminateProcess(pi.hProcess,0);
        }

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }

    if(cResult != NULL)
    {
        DWORD bytesRead = 0;
        DWORD bytesReadToal = 0;
        char  pRead[4096];
        int   maxRead   = 4095;

        while (bytesReadToal<maxRead) 
        {
            if (!ReadFile(hRead, pRead+bytesReadToal, 4095, &bytesRead,NULL))
            {
                break;
            }
            bytesReadToal += bytesRead;
        }

        int len = MultiByteToWideChar(CP_ACP,0,pRead,strlen(pRead),NULL,0); 
        MultiByteToWideChar(CP_ACP,0,pRead,strlen(pRead),cResult,len);
        cResult[len]='\0';
    }
    CloseHandle(hRead); 

    if(dwMSTime == INFINITE)
    {
        if(cResult == NULL)
        {
            DWORD dwRes = WaitForSingleObject(pi.hProcess, dwMSTime);
        }
    }

END:

    return bOK;
}

bool createProcessAsync(LPTSTR lpszCmd, BOOL bIsHide, LPCTSTR lpszCurrentDir)
{
    DWORD  dwExitCode=0;
    return createProcess(lpszCmd,dwExitCode,NULL,0,bIsHide,lpszCurrentDir);
}

bool createProcessSync(LPTSTR lpszCmd, wchar_t *cResult,BOOL bIsHide, LPCTSTR lpszCurrentDir)
{
    DWORD  dwExitCode=0;
    return createProcess(lpszCmd,dwExitCode,cResult,INFINITE,bIsHide,lpszCurrentDir);
}

bool getServiceStatus(LPWSTR serviceName, SERVICE_STATUS& status)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database
        SERVICE_QUERY_STATUS);
    if(schSCManager==NULL)
    {
        return false;
    }
    schService = OpenService(
        schSCManager,            // SCM database
        serviceName,              // name of service
        SERVICE_QUERY_STATUS );
    if (schService == NULL)
    {
        CloseServiceHandle(schSCManager);
        return false;
    }

    BOOL fRc = QueryServiceStatus(schService, &status);
    if (!fRc)
    {
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return false;
    }
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);

    return true;
}

bool openService(LPWSTR serviceName, SERVICE_STATUS& status)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database
        GENERIC_EXECUTE);

    if (schSCManager==NULL)
    {
        return false;
    }

    schService = ::OpenService(schSCManager, serviceName,
        SERVICE_START | SERVICE_QUERY_STATUS);
    if (schService == NULL)
    {
        ::CloseServiceHandle(schSCManager);
        return false;
    }

    if (!::QueryServiceStatus(schService, &status))
    {
        ::CloseServiceHandle( schService);
        ::CloseServiceHandle( schSCManager);
        return false;
    }

    if (status.dwCurrentState == SERVICE_STOPPED)
    {
        // 启动服务
        if( ::StartService(schService, NULL, NULL) == FALSE)
        {
            ::CloseServiceHandle( schService);
            ::CloseServiceHandle( schSCManager);
            return false;
        }
        // 等待服务启动
        while( ::QueryServiceStatus(schService, &status) == TRUE)
        {
            ::Sleep(status.dwWaitHint);
            if( status.dwCurrentState == SERVICE_RUNNING)
            {
                ::CloseServiceHandle(schService);
                ::CloseServiceHandle(schSCManager);
                return false;
            }
        }
    }

    ::CloseServiceHandle(schService);
    ::CloseServiceHandle(schSCManager);

    return true;
}

bool stopService(LPWSTR serviceName)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    int       tryTimes = 10;

    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database
        GENERIC_EXECUTE);

    if (schSCManager==NULL)
    {
        return false;
    }

    schService = ::OpenService(schSCManager, serviceName,SERVICE_ALL_ACCESS);

    if (schService==NULL)
        return true;

    SERVICE_STATUS status;
    if (::QueryServiceStatus(schService, &status) == FALSE)
    {
        ::CloseServiceHandle(schService);
        ::CloseServiceHandle(schSCManager);
        return false;
    }

    if (status.dwCurrentState == SERVICE_RUNNING)
    {
        if( ::ControlService(schService, SERVICE_CONTROL_STOP, &status) == FALSE)
        {
            ::CloseServiceHandle(schService);
            ::CloseServiceHandle(schSCManager);
            return false;
        }

        while( ::QueryServiceStatus(schService, &status) == TRUE && tryTimes>0)
        {
            ::Sleep(status.dwWaitHint);
            if (status.dwCurrentState == SERVICE_STOPPED)
            {
                break;
            }

            tryTimes--;
        }
    }

    DeleteService(schService);

    ::CloseServiceHandle(schService);
    ::CloseServiceHandle(schSCManager);

    return true;
}

BOOL getProcessFilePathByPId( const DWORD dwProcessId, wchar_t* szPath)
{
    HANDLE hProcess = NULL;
    BOOL bSuccess = FALSE;

    hProcess = OpenProcess( 
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , 
        FALSE, dwProcessId );

    do 
    {
        HMODULE hMod  = NULL;
        DWORD cbNeeded = 0;

        if ( NULL == hProcess )
        {
            break;
        }

        if( FALSE == EnumProcessModulesEx( hProcess, &hMod, 
            sizeof( hMod ), &cbNeeded, LIST_MODULES_ALL) )
        {
            break;
        }

        if ( 0 == GetModuleFileNameEx( hProcess, hMod, szPath, MAX_PATH) )
        {
            break;
        }

        bSuccess = TRUE;
    } while( 0 );

    if ( NULL != hProcess )
    {
        CloseHandle( hProcess );
        hProcess = NULL;
    }

    return bSuccess;
}

bool closeProcess(LPCTSTR lpszName, LPTSTR lpszPointPath, LPCTSTR lpzsIgnoreName)
{
    if(lstrlen(lpszPointPath) == 0)
        return FALSE;

    for (int i=0; i<lstrlen(lpszPointPath); i++)
    {
        if (lpszPointPath[i]==_T('/'))
            lpszPointPath[i]=_T('\\');
    }

    HANDLE hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    PROCESSENTRY32 procList;
    procList.dwSize=sizeof(PROCESSENTRY32);
    BOOL bRet=Process32First(hSnapshot,&procList);

    if(!bRet) 
    {
        CloseHandle(hSnapshot);
        return FALSE;
    }

    BOOL isSuccess = FALSE;
    do
    {
        //允许忽略lpszName参数
        if(lpszName != NULL && 0 != lstrcmp(lpszName,procList.szExeFile))
            continue;

        HANDLE hProcess=OpenProcess(PROCESS_TERMINATE,FALSE,procList.th32ProcessID);
        if(hProcess)
        {
            wchar_t szFilePath[MAX_PATH];

            if(!getProcessFilePathByPId(procList.th32ProcessID, szFilePath))
            {
                CloseHandle(hProcess);
                continue;
            }

            if (wcsstr(szFilePath,lpszPointPath)==NULL)
            {
                CloseHandle(hProcess);
                continue;
            }

            if(!TerminateProcess(hProcess,0))
            {
                //                win32lib::wstringex strError;
                //                strError.Format(_T("错误号:%d"),GetLastError());
                //                //AfxMessageBox(strError,MB_OK|MB_ICONINFORMATION,NULL);
                //                TRACE(strError);
            }
            else
            {
                isSuccess = TRUE;
            }
        }

    }while(Process32Next(hSnapshot,&procList));

    CloseHandle(hSnapshot);
    return isSuccess;
}

bool isDirectory(const wchar_t* filename)
{

    DWORD dwAttr = ::GetFileAttributes(filename);

    if (dwAttr == 0xFFFFFFFF)
        return false;

    else if (dwAttr&FILE_ATTRIBUTE_DIRECTORY)
        return true; 

    return false;
}

bool deleteDirectory(const wchar_t* szDirName)
{
    wchar_t         szDataReg[MAX_PATH];
    WIN32_FIND_DATA FindFileData;
    ZeroMemory(&FindFileData, sizeof(WIN32_FIND_DATA));

    wsprintf(szDataReg,_T("%s\\*.*"),szDirName);
    HANDLE hFile = FindFirstFile(szDataReg, &FindFileData);
    BOOL IsFinded = TRUE;

    while(IsFinded && hFile !=INVALID_HANDLE_VALUE)
    {
        IsFinded = FindNextFile(hFile, &FindFileData);
        if( wcscmp(FindFileData.cFileName, _T(".")) && wcscmp(FindFileData.cFileName, _T("..")) )
        {
            std::wstring strFileName = _T("");
            strFileName = strFileName + szDirName + _T("\\") + FindFileData.cFileName;  
            std::wstring strTemp;  
            strTemp = strFileName;  
            if( isDirectory(strFileName.c_str()) )
            {
                deleteDirectory(strTemp.c_str());
            }
            else
            {
                DeleteFile(strTemp.c_str());
            }
        }
    }
    FindClose(hFile);  
  
    BOOL bRet = RemoveDirectory(szDirName);  
    if( bRet == 0 )
    {
        return FALSE;  
    }

    return TRUE;  
}

std::wstring TrimString(const std::wstring &str)
{ 
    if(str.empty()) {
        return std::wstring();
    }

    std::wstring::size_type startPos = 0;
    for(int i = 0; i < (int)str.length(); ++i) {
        if(str.at(i) == _T(' ')) {
            ++startPos;
        } else {
            break;
        }
    }

    int iCount = str.length() - startPos; 
    for(int i = (int)str.length() - 1; i > -1; --i) {
        if(str.at(i) == _T(' ')) {
            --iCount;
        } else {
            break;
        }
    }
    if(iCount > 0) {
        return str.substr(startPos, iCount);
    }
    return std::wstring(); 
} 

bool GetSingleLine(FILE* fp, std::wstring& outLine)
{
    if(NULL == fp) {
        return false;
    }

    register wint_t nCurrChar = 0;

    if(!outLine.empty()) {
        outLine.clear();
    }

    bool bEof = false;

    while(true)
    {
        nCurrChar = getwc(fp);

        if(nCurrChar == (wint_t)EOF)
        {
            bEof = true;
            break;
        }
        else if (nCurrChar == (wint_t)0x0A)
        {
            break;
        }
        else
        {
            outLine += nCurrChar;
        }
    }

    if(!bEof) {
        return true;
    }

    return false;
}


DWORD GetIniFileString(LPCTSTR lpAppName,LPCTSTR lpKeyName,LPCTSTR lpDefault,LPTSTR lpReturnedString,DWORD nSize,LPCTSTR lpFileName)
{
    FILE* fp = _wfopen(lpFileName, _T("r,ccs=UTF-8"));

    if(NULL == fp) return 0; 

    // open 的时候加了ccs=UTF-8 会自动把 BOM 过滤掉，但是这边还是在判断一次
    // Ignore UTF-8 BOM
    int nOffset = ftell(fp);
    wchar_t utf8Bom[4] = {0,0,0,0};
    fgetws(utf8Bom, 4, fp);
    if(utf8Bom[0] != _T('\xEF') 
        || utf8Bom[1] != _T('\xBB') 
        || utf8Bom[2] != _T('\xBF')) 
    {
        fseek(fp, nOffset, SEEK_SET);
    }

    std::wstring strLine; 
    std::wstring strRoot;

    bool bFoundRoot = false;

    while(GetSingleLine(fp, strLine)) { 

        std::wstring::size_type leftPos = 0; 
        std::wstring::size_type rightPos = 0; 
        std::wstring::size_type equalDivPos = 0; 

        std::wstring strKey; 
        std::wstring strValue; 

        if((strLine.npos != (leftPos = strLine.find(_T("["))))
            && (strLine.npos != (rightPos = strLine.find(_T("]")))))
        { 
            strRoot = strLine.substr(leftPos+1, rightPos-1); 

            if(strRoot == lpAppName) {
                bFoundRoot = true;
            } else {
                bFoundRoot = false;
            }
            continue;
        } 

        if(!bFoundRoot) {
            continue;
        }

        if(strLine.npos != (equalDivPos = strLine.find(_T("=")))) {

            strKey = strLine.substr(0, equalDivPos); 
            strValue = strLine.substr(equalDivPos+1, strLine.size()-1); 
            strKey = TrimString(strKey); 
            strValue = TrimString(strValue);

            if(strKey == lpKeyName)  {
                fclose(fp);
                wcsncpy_s(lpReturnedString, nSize, strValue.c_str(), nSize);
                return strValue.size();
            }
        } 
    } 
    fclose(fp);
    wcsncpy_s(lpReturnedString, nSize, lpDefault, nSize);
    return wcslen(lpDefault);
}

INT GetIniFileInt(LPCTSTR lpAppName,LPCTSTR lpKeyName,INT nDefault,LPCTSTR lpFileName)
{
    FILE* fp = _wfopen(lpFileName, _T("r,ccs=UTF-8"));

    if(NULL == fp) return nDefault; 

    // open 的时候加了ccs=UTF-8 会自动把 BOM 过滤掉，但是这边还是在判断一次
    // Ignore UTF-8 BOM
    int nOffset = ftell(fp);
    wchar_t utf8Bom[4] = {0,0,0,0};
    fgetws(utf8Bom, 4, fp);
    if(utf8Bom[0] != _T('\xEF') 
        || utf8Bom[1] != _T('\xBB') 
        || utf8Bom[2] != _T('\xBF')) 
    {
        fseek(fp, nOffset, SEEK_SET);
    }

    std::wstring strLine; 
    std::wstring strRoot;

    bool bFoundRoot = false;

    while(GetSingleLine(fp, strLine)) { 

        std::wstring::size_type leftPos = 0; 
        std::wstring::size_type rightPos = 0; 
        std::wstring::size_type equalDivPos = 0; 

        std::wstring strKey; 
        std::wstring strValue; 

        if((strLine.npos != (leftPos = strLine.find(_T("["))))
            && (strLine.npos != (rightPos = strLine.find(_T("]")))))
        { 
            strRoot = strLine.substr(leftPos+1, rightPos-1); 

            if(strRoot == lpAppName) {
                bFoundRoot = true;
            } else {
                bFoundRoot = false;
            }
            continue;
        } 

        if(!bFoundRoot) {
            continue;
        }

        if(strLine.npos != (equalDivPos = strLine.find(_T("=")))) {

            strKey = strLine.substr(0, equalDivPos); 
            strValue = strLine.substr(equalDivPos+1, strLine.size()-1); 
            strKey = TrimString(strKey); 
            strValue = TrimString(strValue);

            if(strKey == lpKeyName)  {
                fclose(fp);
                return _wtoi(strValue.c_str());
            }
        } 
    } 
    fclose(fp);
    return nDefault;
}


DWORD GetIniFileSection(LPCTSTR lpAppName,LPTSTR lpReturnedString,DWORD nSize,LPCTSTR lpFileName)
{
    FILE* fp = _wfopen(lpFileName, _T("r,ccs=UTF-8"));

    if(NULL == fp) return 0; 

    // open 的时候加了ccs=UTF-8 会自动把 BOM 过滤掉，但是这边还是在判断一次
    // Ignore UTF-8 BOM
    int nOffset = ftell(fp);
    wchar_t utf8Bom[4] = {0,0,0,0};
    fgetws(utf8Bom, 4, fp);
    if(utf8Bom[0] != _T('\xEF') 
        || utf8Bom[1] != _T('\xBB') 
        || utf8Bom[2] != _T('\xBF')) 
    {
        fseek(fp, nOffset, SEEK_SET);
    }

    std::wstring strLine; 
    std::wstring strRoot;

    bool bFoundRoot = false;

    while(GetSingleLine(fp, strLine)) { 

        std::wstring::size_type leftPos = 0; 
        std::wstring::size_type rightPos = 0; 
        std::wstring::size_type equalDivPos = 0; 

        std::wstring strKey; 
        std::wstring strValue; 

        if((strLine.npos != (leftPos = strLine.find(_T("["))))
            && (strLine.npos != (rightPos = strLine.find(_T("]")))))
        { 
            strRoot = strLine.substr(leftPos+1, rightPos-1); 

            if(strRoot == lpAppName) {
                bFoundRoot = true;
            } else {
                bFoundRoot = false;
            }
            continue;
        } 

        if(!bFoundRoot) {
            continue;
        }

        if(strLine.npos != (equalDivPos = strLine.find(_T("=")))) {

            _tcscat_s(lpReturnedString,nSize,strLine.c_str());
            _tcscat_s(lpReturnedString,nSize,_T(";"));
        } 
    } 
    fclose(fp);
    return wcslen(lpReturnedString);
}
