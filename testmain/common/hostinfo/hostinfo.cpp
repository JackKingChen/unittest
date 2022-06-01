
#include "hostinfo.h"
#include <ShlObj.h>
#include <Shlwapi.h>
#include <TlHelp32.h>
#include <tchar.h>
#include <psapi.h>

#include <GL/GL.h>

#include "utils/regUtil.h"
#include "utils/util.h"
#include "utils/osProcess.h"
#include "vtchecker/OlsApiInit.h"
#include "ttinfo/ttinfo.h"

#include "model.h"

#pragma comment(lib,"Version.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"psapi.lib")

HostInfo::CpuInfo HostInfo::m_cpuInfo;
HostInfo::MemInfo HostInfo::m_memInfo;

HostInfo* HostInfo::localMachine = new HostInfo();

HostInfo::HostInfo(void)
{
    m_cpuInfo.cpuNum = 1;
    m_memInfo.phyMemTotalSz = 1024;
    m_memInfo.phyMemAvailSz = 1024;

    init();
}

HostInfo::~HostInfo(void)
{
    exit();
}

bool HostInfo::getHostOSVersion(LMOS& osVer)
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);

    OSVERSIONINFOEX osvi;
    BOOL bOsVersionInfoEx;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*) &osvi);

    osVer = LMOS_WIN8;

    if (!bOsVersionInfoEx)
    {/*this should never happen*/
        return false;
    }

    switch(osvi.dwMajorVersion)
    {
    case 4:
        {
            switch(osvi.dwMinorVersion)
            {
            case 0:/*Microsoft Windows 4.0*/
                {
                    osVer = LMOS_WIN40;
                    break;
                }
            case 10:/*Microsoft Windows 98*/
                {
                    osVer = LMOS_WIN98;
                    break;
                }
            case 90:/*Microsoft Windows Me*/
                {
                    osVer = LMOS_WINME;
                    break;
                }
            }

            break;
        }
    case 5:
        {
            switch(osvi.dwMinorVersion)
            {
            case 0:/*Microsoft Windows 2000*/
                {
                    osVer = LMOS_WIN2000;
                    break;
                }
            case 1:/*Microsoft Windows XP*/
                {
                    osVer = LMOS_WINXP;
                    break;
                }
            case 2:
                {
                    if(osvi.wProductType==VER_NT_WORKSTATION && info.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
                        osVer = LMOS_WINXP;/*Microsoft Windows XP Professional x64 Edition*/
                    else if(GetSystemMetrics(SM_SERVERR2)==0)
                        osVer = LMOS_WINSERV2003; /*"Microsoft Windows Server 2003"*/        //2003年3月发布
                    else if(GetSystemMetrics(SM_SERVERR2)!=0)
                        osVer = LMOS_WINSERV2003; /*"Microsoft Windows Server 2003 R2"*/
                    break;
                }
            }
            break;
        }
    case 6:
        {
            switch(osvi.dwMinorVersion)
            {
            case 0:
                {
                    osVer = LMOS_VISTA;
                    break;
                }
            case 1:
                {
                    osVer = LMOS_WIN7;
                    break;
                }
            case 2:
                {
                    osVer = LMOS_WIN8;
                    break;
                }
            }
            break;
        }
    }

    TT_LOG_DBUG(_T("version:%d.%d"),osvi.dwMajorVersion,osvi.dwMinorVersion);

    return true;
}

DWORD HostInfo::getHostOSVersion(void)
{
    DWORD dwVersion = MAKELONG(0, 0);
    WCHAR szDLLName[MAX_PATH] = { 0 };
    HRESULT hr = SHGetFolderPath(NULL, CSIDL_SYSTEM, NULL, SHGFP_TYPE_CURRENT, szDLLName);
    if (SUCCEEDED(hr) && PathAppend(szDLLName, L"kernel32.dll")) 
    {
        DWORD dwVerInfoSize = GetFileVersionInfoSize(szDLLName, NULL);
        if (dwVerInfoSize > 0) 
        {
            LPVOID pvVerInfoData = (LPVOID)new BYTE[dwVerInfoSize];
            if (GetFileVersionInfo(szDLLName, 0, dwVerInfoSize, pvVerInfoData)) 
            {
                UINT ulLength = 0;
                VS_FIXEDFILEINFO *pvffi = NULL;
                if (VerQueryValue(pvVerInfoData, L"\\", (LPVOID *)&pvffi, &ulLength)) 
                {
                    dwVersion = pvffi->dwFileVersionMS;
                }
            }
            delete[] pvVerInfoData;
        }
    }
    return dwVersion;
}

bool HostInfo::getHostProductName(wstring& strProductName)
{
    wchar_t szProductName[256]={0};
    int ret=regRead(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",L"ProductName",szProductName);
    strProductName=szProductName;
    return ret==0;
}

void HostInfo::getHostCpuInfo(unsigned int& cpuNum)
{
    cpuNum = m_cpuInfo.cpuNum;
}

bool HostInfo::getHostCpuName(wstring& strCpuName)
{
    HKEY    hKey;
    LPCTSTR lpstrcpu = L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";
    DWORD   dwtype;
    long    lRet;
    wchar_t dirResult[MAX_PATH] = {0};
    DWORD   dwSize = 1000 ;

    lRet= RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpstrcpu, 0, KEY_READ|KEY_WOW64_64KEY , &hKey);
    if(lRet == ERROR_SUCCESS)
    {
        lRet = RegQueryValueExW(hKey, L"ProcessorNameString", NULL, &dwtype, (LPBYTE)dirResult, &dwSize);
        RegCloseKey(hKey);
        strCpuName = dirResult;
        m_cpuInfo.strCpuName = strCpuName;

        return true;
    }

    return false;
}

bool HostInfo::getPhysicalMemory(unsigned long& nTotal/*MB*/, unsigned long& nFree/*MB*/)
{
    MEMORYSTATUSEX        Meminfo;
    HINSTANCE		      h_dll;

    typedef BOOL (WINAPI * Fun_GlobalMemoryStatusEx)(
        LPMEMORYSTATUSEX lpBuffer);

    Fun_GlobalMemoryStatusEx        GlobalMemoryStatusEx_;

    h_dll = ::LoadLibrary(L"kernel32.dll");
    if (h_dll==NULL)
        return false;

    GlobalMemoryStatusEx_ = (Fun_GlobalMemoryStatusEx)::GetProcAddress(h_dll,"GlobalMemoryStatusEx");

    if(GlobalMemoryStatusEx_)
    {
        memset(&Meminfo, 0, sizeof(Meminfo));
        Meminfo.dwLength = sizeof(Meminfo);

        GlobalMemoryStatusEx_(&Meminfo);

        /*
         * FIXME:
         */
        nTotal = (unsigned long)(Meminfo.ullTotalPhys / 1024 / 1024);
        nFree  = (unsigned long)(Meminfo.ullAvailPhys / 1024 / 1024);
    }
    else
    {/*should never*/
        return false;
    }

    return true;
}

bool HostInfo::getNetCardInfo(list<wstring>& slNetCard)
{
    long    lRet;
    HKEY    kRet;

    lRet= RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards", 0, KEY_READ|KEY_WOW64_64KEY , &kRet);

    if(lRet == ERROR_SUCCESS)
    {
        for (int i=0; ;i++)
        {
            wchar_t strValue[MAX_PATH]     = { 0 };
            wchar_t szSubKeyPath[MAX_PATH] = { 0 };
            wchar_t szSubKeyName[MAX_PATH] = { 0 };
            DWORD   nSubNameLen = MAX_PATH;
            lRet = RegEnumKey(kRet, i, szSubKeyName, nSubNameLen);
            if (lRet != ERROR_SUCCESS)
                break;

            wsprintf(szSubKeyPath,L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\%s",szSubKeyName);

            if (regRead(HKEY_LOCAL_MACHINE,szSubKeyPath,L"Description",strValue)==0)
                slNetCard.push_back(strValue);
        }

        RegCloseKey(kRet);

        return true;
    }

    return false;
}

bool HostInfo::getHostGraphicsName(wstring& displayCardName)
{
    wchar_t szValue[MAX_PATH] = { 0 };
    regRead64(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winsat",L"PrimaryAdapterString",szValue);

    displayCardName = szValue;

    return true;
}

bool HostInfo::getMacAddress(wstring& strMacAddr)
{

    return true;
}

void HostInfo::getHostMemInfo(unsigned int& phymemTotal,unsigned int& phymemAvail)
{
    phymemTotal = m_memInfo.phyMemTotalSz;
    phymemAvail = m_memInfo.phyMemAvailSz;

    return;
}

void HostInfo::getScreenWorkSpaceInfo(int& x,int& y,int& workspaceWidth, int& workspaceHeight)
{
    RECT rt;
    SystemParametersInfo(SPI_GETWORKAREA,0,(PVOID)&rt,0);

    x = rt.left;
    y = rt.top;
    workspaceWidth = rt.right - rt.left;
    workspaceHeight= rt.bottom- rt.top;

    return;
}

void HostInfo::getScreenSize(int& width, int& height)
{
    width = GetSystemMetrics(SM_CXSCREEN);
    height= GetSystemMetrics(SM_CYSCREEN);

    return;
}

void HostInfo::getTitleBarHeight(int& height)
{
    height = GetSystemMetrics(SM_CYCAPTION);

    return;
}

void HostInfo::getBorderWidth(int& width)
{
    width  = GetSystemMetrics(SM_CXFRAME);

    return;
}

int HostInfo::getScreenNum(void)
{
    return 1;
}

int HostInfo::getHostColorDepth(void)
{
    HDC hdc = GetDC(NULL);
    int colour_depth = ::GetDeviceCaps(hdc, BITSPIXEL);
    ReleaseDC(NULL,hdc);

    return colour_depth;
}

bool HostInfo::killProcessTree(DWORD dwProcessID)
{
    PROCESSENTRY32 info;
    info.dwSize = sizeof(PROCESSENTRY32 );
    //结束进程句柄
    HANDLE hProcess = NULL;
    DWORD dwChildPID = 0;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, dwProcessID);
    if(hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    // 遍历进程快照，轮流显示每个进程的信息
    Process32First(hProcessSnap, &info);

    while( Process32Next(hProcessSnap, &info) != FALSE)
    {
        dwChildPID = 0;
        // 如果找个父进程句柄是需要关闭的ID，就已经完成查找
        if (dwProcessID == info.th32ParentProcessID)
        {
            dwChildPID = info.th32ProcessID;
        }

        if (dwChildPID)
        {
            // 如果有子线程先结束子线程
            hProcess=OpenProcess(PROCESS_TERMINATE, FALSE, dwChildPID);
            if (NULL == hProcess)
            {
                continue;
            }
            TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
        }
    }

    hProcess=OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessID);
    if (NULL == hProcess)
    {
        return false;
    }

    TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);

    return true;
}

bool HostInfo::IsAdministrator(void)
{
    HANDLE                   hAccessToken;
    BYTE                     InfoBuffer[1024];
    PTOKEN_GROUPS            ptgGroups;
    DWORD                    dwInfoBufferSize;
    PSID                     psidAdministrators;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    UINT                     i;
    bool                     bRet = false;

    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hAccessToken))
        return bRet;
    bRet = GetTokenInformation(hAccessToken, TokenGroups, InfoBuffer, 1024, &dwInfoBufferSize);
    CloseHandle(hAccessToken);
    if(!bRet)
        return bRet;
    if(!AllocateAndInitializeSid(&siaNtAuthority,2,SECURITY_BUILTIN_DOMAIN_RID,DOMAIN_ALIAS_RID_ADMINS,0,0,0,0,0,0,&psidAdministrators))
    {
        return false;
    }
    bRet = false;
    ptgGroups = (PTOKEN_GROUPS)InfoBuffer;
    for(i=0;i<ptgGroups->GroupCount;i++)
    {
        if(EqualSid(psidAdministrators,ptgGroups->Groups[i].Sid))
        {
            bRet = true;
        }
    }

    FreeSid(psidAdministrators);

    return bRet;
}

#ifdef VT_NO_USE_VTCHECKER
HostInfo::VT_STATUS HostInfo::getVtStatus(void)
{
    static VT_STATUS s_status = VT_FAILED;

    if (s_status != VT_FAILED)
    {
        return s_status;
    }

    OSVERSIONINFO OsVer;

    ZeroMemory(&OsVer, sizeof(OSVERSIONINFO));
    OsVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&OsVer);

    if(OsVer.dwPlatformId == VER_PLATFORM_WIN32_NT && !IsAdministrator())
    {
        return s_status;
    }


    HMODULE m_hOpenLibSys;

    if(InitOpenLibSys(&m_hOpenLibSys) != TRUE)
    {
        return VT_FAILED;
    }

    switch(GetDllStatus())
    {
    case OLS_DLL_NO_ERROR:
        break;
    case OLS_DLL_UNSUPPORTED_PLATFORM:
        return VT_FAILED;
    case OLS_DLL_DRIVER_NOT_LOADED:
        return VT_FAILED;
    case OLS_DLL_DRIVER_NOT_FOUND:
        return VT_FAILED;
    case OLS_DLL_DRIVER_UNLOADED:
        return VT_FAILED;
    case OLS_DLL_DRIVER_NOT_LOADED_ON_NETWORK:
        return VT_FAILED;
    case OLS_DLL_UNKNOWN_ERROR:
    default:
        return VT_FAILED;
    }

    if(IsIntelVtSupported() || IsAmdvSupported())
    {
        if(IsIntelVtEnabled() || IsAmdvEnabled())
        {
            s_status = VT_HAS_OPENED;
        }
        else
        {
            s_status = VT_NO_OPEN;
        }
    }
    else
    {
        s_status = VT_NO_SUPPORT;
    }

    DeinitOpenLibSys(&m_hOpenLibSys);

    return s_status;
}
#else

HostInfo::VT_STATUS HostInfo::getVtStatus(void)
{
    static VT_STATUS s_status = VT_FAILED;

    if (s_status != VT_FAILED)
    {
        return s_status;
    }

    OSProcess pro;
    wchar_t   szInstallPath[MAX_PATH] = { 0 };
    wchar_t   szVBoxPath[MAX_PATH] = { 0 };
    wchar_t   szCmdLine[1024] = { 0 };

    TTInfo::getTtInstallPath(szInstallPath);
    TTInfo::getVBoxInstallPath(szVBoxPath);

    wsprintf(szCmdLine, _T("%s\\Tools\\vtchecker.exe"),szInstallPath);

    if (pro.create(szCmdLine, szVBoxPath, true))
    {
        int  nExitCode = 0;
        VT_STATUS status = VT_HAS_OPENED;

        pro.wait(&nExitCode);

        switch (nExitCode)
        {
        case 0: status = VT_NO_OPEN; break;
        case 1: status = VT_HAS_OPENED; break;
        default:status = VT_FAILED; break;
        }

        s_status = status;
    }

    return s_status;
}

#endif

bool HostInfo::openService(LPWSTR serviceName, SERVICE_STATUS& status)
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

bool HostInfo::getServiceStatus(LPWSTR serviceName, SERVICE_STATUS& status)
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

bool HostInfo::startService(LPWSTR serviceName)
{
    OSProcess  pro;
    int        code;

    wchar_t   szInstallPath[MAX_PATH] = { 0 };
    wchar_t   szCmdline[1024] = { 0 };

    TTInfo::getTtInstallPath(szInstallPath);

    wsprintf(szCmdline,_T("\"%s\\%s\" startdrv"),szInstallPath,TT_REGSVR_EXE);

    wprintf(_T("%s\n"),szCmdline);

    if (!pro.create(szCmdline,NULL,true))
    {
        printf("create process:%d\n",GetLastError());
        return TT_ERROR(0,GetLastError());
    }

    if (!pro.wait(&code))
    {
        printf("wait process:%d\n",GetLastError());
        return TT_ERROR(0,GetLastError());
    }

    return 0;
}

bool HostInfo::installService(LPWSTR serviceName)
{
    OSProcess  pro;
    int        code;

    wchar_t   szInstallPath[MAX_PATH] = { 0 };
    wchar_t   szCmdline[1024] = { 0 };

    TTInfo::getTtInstallPath(szInstallPath);

    wsprintf(szCmdline,_T("\"%s\\%s\" fixdrv"),szInstallPath,TT_REGSVR_EXE);

    wprintf(_T("%s\n"),szCmdline);

    if (!pro.create(szCmdline,NULL,true))
    {
        printf("create process:%d\n",GetLastError());
        return TT_ERROR(0,GetLastError());
    }

    if (!pro.wait(&code))
    {
        printf("wait process:%d\n",GetLastError());
        return TT_ERROR(0,GetLastError());
    }

    return 0;
}

bool HostInfo::getMemInfo(unsigned int& phymemTotal,unsigned int& phymemAvail)
{
    MEMORYSTATUSEX        Meminfo;
    HINSTANCE		      h_dll;

    typedef BOOL (WINAPI * Fun_GlobalMemoryStatusEx)(
        LPMEMORYSTATUSEX lpBuffer);

    Fun_GlobalMemoryStatusEx        GlobalMemoryStatusEx_;

    h_dll = ::LoadLibrary(_T("kernel32.dll"));
    if (h_dll==NULL)
        return false;

    GlobalMemoryStatusEx_ = (Fun_GlobalMemoryStatusEx)::GetProcAddress(h_dll,"GlobalMemoryStatusEx");

    if(GlobalMemoryStatusEx_)
    {
        memset(&Meminfo, 0, sizeof(Meminfo));
        Meminfo.dwLength = sizeof(Meminfo);

        GlobalMemoryStatusEx_(&Meminfo);

        /*
         * FIXME:
         */
        phymemTotal = (unsigned long)(Meminfo.ullTotalPhys / 1024 / 1024);
        phymemAvail = (unsigned long)(Meminfo.ullAvailPhys / 1024 / 1024);
    }
    else
    {/*should never*/
        TT_LOG_ERR(_T("get host system info failed\n"));

        return false;
    }

    return true;
}

wstring HostInfo::getStardantpath(int sid)
{
    wstring strRet;

    LPITEMIDLIST pidl;
    LPMALLOC pShellMalloc;

    if (SUCCEEDED(SHGetMalloc(&pShellMalloc)))
    {
        wchar_t Path[MAX_PATH + 1];

        if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, sid, &pidl)))
        {
            if (SHGetPathFromIDList(pidl, Path))
            {
                strRet = Path;
            }
            pShellMalloc->Free(pidl);
        }

        pShellMalloc->Release();
    }

    return strRet;
}

bool HostInfo::createDestopShortcut(wstring szPath, wstring szLinkName, wstring szArgument/*=""*/, wstring szIconFile/*=""*/)
{
    HRESULT         hres;
    IShellLink*     psl;
    IPersistFile*   ppf;

    TCHAR strDeskTop[MAX_PATH +1];
    SHGetSpecialFolderPath(0,strDeskTop,CSIDL_DESKTOPDIRECTORY,FALSE);
    wstring strLinkPath= wstring(strDeskTop) + wstring(_T("/")) + wstring(szLinkName);
    TTInfo *pTtInfo=NULL;
    TTInfo::getClassObject(pTtInfo,0);
    TCHAR szAppPath[MAX_PATH]={0};
    pTtInfo->getTtInstallPath(szAppPath);
    wstring strAppPath = szAppPath;

    FILE *fLinkFile=_wfopen(strLinkPath.c_str(),_T("r"));

    if (fLinkFile)
        return true;

    CoInitialize(NULL);

    hres = CoCreateInstance(CLSID_ShellLink,NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void   **)&psl);
    if(FAILED(hres))
    {
        CoUninitialize();
        return FALSE;
    }

    //设置目标应用程序
    psl->SetPath(szPath.c_str());
    psl->SetWorkingDirectory(strAppPath.c_str());
    psl->SetArguments(szArgument.c_str());
    psl->SetIconLocation(szIconFile.c_str(),0);

    //从IShellLink获取其IPersistFile接口
    //用于保存快捷方式的数据文件   (*.lnk)
    hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
    if(FAILED(hres))
    {
        CoUninitialize();
        return FALSE;
    }

    //调用IPersistFile::Save
    //保存快捷方式的数据文件   (*.lnk)
    hres = ppf->Save(strLinkPath.c_str(), STGM_READWRITE);
    //释放IPersistFile和IShellLink接口
    ppf->Release();
    psl->Release();

    CoUninitialize();

    return true;
}

bool HostInfo::createSetupMenuShortcut(wstring szPath, wstring szLinkName, wstring szArgument/*=""*/, wstring szIconFile/*=""*/)
{
    HRESULT         hres;
    IShellLink*     psl;
    IPersistFile*   ppf;

    wstring strStartmenuProgram = getStardantpath(CSIDL_COMMON_PROGRAMS);

    if (strStartmenuProgram.empty())
        return false;

    strStartmenuProgram = strStartmenuProgram + wstring(_T("/"))/* + TT_PRODUCT_NAME*/;

    wstring strLinkPath= strStartmenuProgram + wstring(_T("/")) + szLinkName;
    TTInfo *pTtInfo=NULL;
    TTInfo::getClassObject(pTtInfo,0);
    TCHAR szAppPath[MAX_PATH]={0};
    pTtInfo->getTtInstallPath(szAppPath);
    wstring strAppPath = szAppPath;

    FILE *fLinkFile=_wfopen(strLinkPath.c_str(),_T("r"));

    if (fLinkFile)
        return true;

    CoInitialize(NULL);

    hres = CoCreateInstance(CLSID_ShellLink,NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void   **)&psl);
    if(FAILED(hres))
    {
        CoUninitialize();
        return FALSE;
    }

    //设置目标应用程序
    psl->SetPath(szPath.c_str());
    psl->SetWorkingDirectory(strAppPath.c_str());
    psl->SetArguments(szArgument.c_str());
    psl->SetIconLocation(szIconFile.c_str(),0);

    //从IShellLink获取其IPersistFile接口
    //用于保存快捷方式的数据文件   (*.lnk)
    hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
    if(FAILED(hres))
    {
        CoUninitialize();
        return FALSE;
    }

    //调用IPersistFile::Save
    //保存快捷方式的数据文件   (*.lnk)
    hres = ppf->Save(strLinkPath.c_str(), STGM_READWRITE);
    //释放IPersistFile和IShellLink接口
    ppf->Release();
    psl->Release();

    CoUninitialize();

    return true;
}

bool HostInfo::isProcessRunning(DWORD processID)
{
    DWORD exitCode = 0;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE, processID );

    if (hProcess != NULL)
    {
        if (!GetExitCodeProcess(hProcess,&exitCode))
        {
            /*
             * FIXME: when it failed
             */
            TT_LOG_DBUG(_T("GetExitCodeProcess failed:%d"),GetLastError());
            CloseHandle(hProcess);
            return false;
        }

        if (exitCode == STILL_ACTIVE)
        {
            char    path[_MAX_PATH+1]="";
            GetModuleFileNameExA(hProcess,NULL,path,MAX_PATH+1);

            CloseHandle(hProcess);

            if (strstr(path,"TianTianPlayer") != NULL)
            {
                return true;
            }
        }

        return false;
    }

    return true;
}

bool HostInfo::isWow64(void)
{
    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, BOOL*);
    BOOL bIsWow64 = false;

    LPFN_ISWOW64PROCESS fnIsWow64Process = NULL;

    if(fnIsWow64Process == NULL)
    {
        fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
            GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
    }

    if (NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
        {
            // handle error
        }
    }

    return bIsWow64;
}

bool HostInfo::is360VtSafeProtected(void)
{
    wchar_t strValue[256]=_T("");

    int iRet;

    if (regRead(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\services\\360Hvm",L"Start",&iRet) != 0)
    {/*should never happen*/
        return false;
    }
    if (iRet == 1)
        return true;

    return false;
}

bool HostInfo::enableOpenGL(HWND hWnd, HDC *hDC, HGLRC *hRC)
{
#ifdef OPENGL32_LIB_REFER
    static BOOL wasException = FALSE;

    if(wasException)
        return FALSE;

    PIXELFORMATDESCRIPTOR pfd;
    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC (hWnd);

    /* set the pixel format for the DC */
    ZeroMemory (&pfd, sizeof (pfd));
    pfd.nSize = sizeof (pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    iFormat = ChoosePixelFormat (*hDC, &pfd);
    SetPixelFormat (*hDC, iFormat, &pfd);

    *hRC = wglCreateContext( *hDC );

    if (*hRC==NULL || !wglMakeCurrent( *hDC, *hRC ))
    {
        wasException = TRUE;
    }

    return !wasException;
#else
    return true;
#endif
}

void HostInfo::disableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC)
{
#ifdef OPENGL32_LIB_REFER
    wglMakeCurrent (NULL, NULL);
    wglDeleteContext (hRC);
    ReleaseDC (hWnd, hDC);
#endif
}

bool HostInfo::getOpenglInfo(HWND hWnd,wstring& strVersion,wstring& strVendor,wstring& strRender)
{
#ifdef OPENGL32_LIB_REFER
    HDC   hDC;
    HGLRC hRC;
    /*
     * init opengl first
     */
    if (!enableOpenGL(hWnd,&hDC,&hRC))
        return false;

    const GLubyte* byteGlVersion = glGetString(GL_VERSION);
    const GLubyte* byteGlVendor  = glGetString(GL_VENDOR);
    const GLubyte* byteGlRenderer= glGetString(GL_RENDERER);

    strVersion = stringToWString((const char*)byteGlVersion);
    strVendor  = stringToWString((const char*)byteGlVendor);
    strRender  = stringToWString((const char*)byteGlRenderer);

    /* shutdown OpenGL */
    disableOpenGL (hWnd, hDC, hRC);
#endif

    return true;
}

bool HostInfo::changeHostColorDepth(int nColorDepth/*=32*/)
{
    DEVMODE dm;
    dm.dmSize = sizeof(DEVMODE);
    dm.dmDriverExtra = 0;
    dm.dmFields = DM_BITSPERPEL;

    //修改色位为32色
    dm.dmBitsPerPel = nColorDepth;
    if(DISP_CHANGE_SUCCESSFUL != ChangeDisplaySettings(&dm, CDS_UPDATEREGISTRY))
    {
        return false;
    }

    return true;
}

bool HostInfo::IsIntelVtSupported(void)
{
    DWORD eax, ebx, ecx, edx;
    if(CpuidPx(0x1, 0, &eax, &ebx, &ecx, &edx, 1) && (ecx & (1 << 5)))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool HostInfo::IsAmdvSupported(void)
{
    DWORD eax, ebx, ecx, edx;
    CpuidPx(0x80000000, 0, &eax, &ebx, &ecx, &edx, 1);
    if(eax < 0x80000001)
    {
        return false;
    }
    if(CpuidPx(0x80000001, 0, &eax, &ebx, &ecx, &edx, 1) && (ecx & (1 << 2)))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool HostInfo::IsIntelVtEnabled(void)
{
    DWORD eax, edx;
    if(RdmsrPx(0x3A, &eax, &edx, 1) && (eax & (1 << 2)))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool HostInfo::IsAmdvEnabled(void)
{
    DWORD eax, edx;
    if(RdmsrPx(0xC0010114, &eax, &edx, 1) && (! (eax & (1 << 4))))
        //	if(RdmsrPx(0xC0000080, &eax, &edx, 1) && (eax & (1 << 12)))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void HostInfo::init(void)
{
    getCpuInfo();
    getMemInfo();

    return;
}

void HostInfo::exit(void)
{
    return;
}

void HostInfo::getCpuInfo(void)
{
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    m_cpuInfo.cpuNum = systemInfo.dwNumberOfProcessors;
    return;
}

void HostInfo::getMemInfo(void)
{
    MEMORYSTATUSEX        Meminfo;
    HINSTANCE             h_dll;

    typedef BOOL (WINAPI * Fun_GlobalMemoryStatusEx)(LPMEMORYSTATUSEX lpBuffer);

    Fun_GlobalMemoryStatusEx        GlobalMemoryStatusEx_;

    h_dll = ::LoadLibrary(_T("kernel32.dll"));
    if (h_dll==NULL)
        return;

    GlobalMemoryStatusEx_ = (Fun_GlobalMemoryStatusEx)::GetProcAddress(h_dll,"GlobalMemoryStatusEx");

    if(GlobalMemoryStatusEx_)
    {
        memset(&Meminfo, 0, sizeof(Meminfo));
        Meminfo.dwLength = sizeof(Meminfo);

        GlobalMemoryStatusEx_(&Meminfo);

        /*
         * FIXME:
         */
        m_memInfo.phyMemTotalSz = (unsigned long)(Meminfo.ullTotalPhys / 1024 / 1024);
        m_memInfo.phyMemAvailSz = (unsigned long)(Meminfo.ullAvailPhys / 1024 / 1024);
    }
    else
    {/*should never*/
        TT_LOG_ERR(_T("get host system info failed\n"));
    }

    return;
}

#define CPU_LIST \
{\
    "Athlon","Phenom","Sempron","AMD*E2","AMD*E1",\
    "A8*3550","A8*3560","A8*3800","A8*3820","A8*3850","A8*3870K",\
    "A6*3500","A6*3600","A6*3620","A6*3670K",\
    "A4*3300","A4*3400","A4*3420",\
    "E2*3200",\
    "Intel*N280","Intel*E5200"\
}

#define ANTI_VIRUSSFNAME_LIST \
{\
    L"360杀毒软件", \
    L"百度杀毒软件", \
    L"瑞星杀毒软件", \
    L"金山毒霸", \
    L"瑞星杀毒软件" \
}

#define ANTI_VIRUSSFIMG_LIST \
{\
    L"360sd.exe",\
    L"BaiduSdUProxy.exe", \
    L"rsagent.exe", \
    L"kxetray.exe", \
    L"rsmain.exe" \
}

std::wstring HostInfo::getAlivedAntiVenusSoftwareName()
{
    HANDLE          hProcesses;
    wstring         strName;

    wstring strAntiVenusIMGList[]  = ANTI_VIRUSSFIMG_LIST;
    wstring strAntiVenusNameList[] = ANTI_VIRUSSFNAME_LIST;

    PROCESSENTRY32  lpe =
    {
        sizeof(PROCESSENTRY32)
    };

    hProcesses = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcesses == INVALID_HANDLE_VALUE)
    {/*should never happen*/
        return L"";
    }

    BOOL isExist   = ::Process32First(hProcesses, &lpe);

    while (isExist)
    {
        strName = lpe.szExeFile;

        for (int i=0; i<sizeof(strAntiVenusIMGList)/sizeof(wstring); i++)
        {
            if (strName.find(strAntiVenusIMGList[i])!=std::wstring::npos)
            {
                CloseHandle(hProcesses);
                return strAntiVenusNameList[i];
            }
        }

        // 遍历下一个进程实例
        isExist = ::Process32Next(hProcesses, &lpe);
    }

    CloseHandle(hProcesses);

    return L"";
}
