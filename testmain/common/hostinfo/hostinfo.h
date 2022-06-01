
#pragma once;

#include <Windows.h>
#include <string>
#include <list>

using namespace std;

class HostInfo
{
    typedef struct
    {
        unsigned int cpuNum;
        wstring      strCpuName;
    }CpuInfo;

    typedef struct
    {
        unsigned int phyMemTotalSz; /*MB*/
        unsigned int phyMemAvailSz; /*MB*/
    }MemInfo;

    typedef struct
    {
        unsigned int workspaceWidth;
        unsigned int workspaceHeight;
        unsigned int totalWidth;
        unsigned int totalHeight;
    }ScreenInfo;

public:
    enum LMOS
    {
        LMOS_WIN40,
        LMOS_WIN98,
        LMOS_WINME,
        LMOS_WIN2000,
        LMOS_WINSERV2003,
        LMOS_WINXP,
        LMOS_VISTA,
        LMOS_WIN7,
        LMOS_WIN8,
        LMOS_WIN10
    };

    enum VT_STATUS
    {
        VT_HAS_OPENED = 0,
        VT_NO_SUPPORT = 1,
        VT_NO_OPEN    = 2,
        VT_NO_COMPATIABLE = 3,
        VT_FAILED     = 4
    };

private:
    HostInfo(void);
    ~HostInfo(void);

private:
    static HostInfo* localMachine;

public:
    static bool getHostOSVersion(LMOS& osVer);
    static DWORD getHostOSVersion(void);
    static bool getHostProductName(wstring& strProductName);
    static void getHostCpuInfo(unsigned int& cpuNum);
    static bool getHostCpuName(wstring& strCpuName);
    static bool getPhysicalMemory(unsigned long& nTotal/*MB*/, unsigned long& nFree/*MB*/);
    static bool getNetCardInfo(list<wstring>& slNetCard);
    static bool getHostGraphicsName(wstring& displayCardName);
    static bool getMacAddress(wstring& strMacAddr);
    static void getHostMemInfo(unsigned int& phymemTotal,unsigned int& phymemAvail);
    static void getScreenWorkSpaceInfo(int& x,int& y,int& workspaceWidth, int& workspaceHeight);
    static void getScreenSize(int& width, int& height);
    static void getTitleBarHeight(int& height);
    static void getBorderWidth(int& width);
    static int  getScreenNum(void);
    static int  getHostColorDepth(void);
    static bool killProcessTree(DWORD dwProcessID);
    static VT_STATUS getVtStatus(void);
    /*FIXME: should always return true*/
    static bool openService(LPWSTR serviceName, SERVICE_STATUS& status);
    static bool getServiceStatus(LPWSTR serviceName, SERVICE_STATUS& status);
    static bool startService(LPWSTR serviceName);
    static bool installService(LPWSTR serviceName);
    static bool getMemInfo(unsigned int& phymemTotal,unsigned int& phymemAvail);

    static wstring getStardantpath(int sid);
    static bool createDestopShortcut(wstring szPath, wstring szLinkName, wstring szArgument=L"", wstring szIconFile=L"");
    static bool createSetupMenuShortcut(wstring szPath, wstring szLinkName, wstring szArgument=L"", wstring szIconFile=L"");

    static bool isProcessRunning(DWORD processID);

    static bool isWow64(void);

    static bool is360VtSafeProtected(void);
    static bool enableOpenGL(HWND hWnd, HDC *hDC, HGLRC *hRC);
    static void disableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC);
    static bool getOpenglInfo(HWND hWnd,wstring& strVersion,wstring& strVendor,wstring& strRender);
    static std::wstring getAlivedAntiVenusSoftwareName();

public:
    static bool changeHostColorDepth(int nColorDepth=32);

private:
    static bool IsIntelVtSupported(void);
    static bool IsAmdvSupported(void);
    static bool IsIntelVtEnabled(void);
    static bool IsAmdvEnabled(void);

private:
    void init(void);
    void exit(void);

private:
    static void getCpuInfo(void);
    static void getMemInfo(void);

private:
    static CpuInfo  m_cpuInfo;
    static MemInfo  m_memInfo;
    static ScreenInfo m_screenInfo;

private:
    static bool IsAdministrator(void);

};
