
#include "ttinfo.h"

#include <Windows.h>
#include <tchar.h>

#include "error/tterrorcode.h"

#include "utils/regUtil.h"

#include "model.h"

std::map<int,TTInfo*> TTInfo::m_mapTtInfo;
HANDLE                TTInfo::m_hMapLock = TT_LOCK_CREATE();

TTInfo::TTInfo(int nEmuID) :
        m_nOpenglChanId(TT_CHANNEL_OPENGL),
        m_nTtAdbChanId(TT_CHANNEL_TTADB),
        m_nSensorChanId(TT_CHANNEL_SENSOR),
        m_nInputChanId(TT_CHANNEL_INPUT),
        m_nImeChanId(TT_CHANNEL_IME),
        m_nAppChangeId(TT_CHANNEL_APPCHANGE),
        m_nAdbChanId(TT_CHANNEL_ADB)
{
    m_nEmuID = nEmuID;
}

TTInfo::~TTInfo(void)
{

}

int  TTInfo::getClassObject(TTInfo*& pTtInfo,int nEmuID)
{
    TT_LOCK(m_hMapLock);

    std::map<int, TTInfo*>::iterator it = m_mapTtInfo.find(nEmuID);

    if (it == m_mapTtInfo.end())
    {
        pTtInfo = new TTInfo(nEmuID);

        m_mapTtInfo[nEmuID] = pTtInfo;

        if(!pTtInfo->parseIniFile())
        {
            TT_UNLOCK(m_hMapLock);
            return TT_ERROR(TET_OTHER,TEC_FILE_BROKEN);
        }
    }
    else
    {
        pTtInfo = it->second;

        if(!pTtInfo->parseIniFile())
        {
            TT_UNLOCK(m_hMapLock);
            return TT_ERROR(TET_OTHER,TEC_FILE_BROKEN);
        }
    }

    TT_UNLOCK(m_hMapLock);

    return 0;
}

int  TTInfo::getTtInstallPath(wchar_t* szInstallPath)
{
    wchar_t szRegPlace[MAX_PATH] = { 0 };

    wsprintf(szRegPlace,_T("SOFTWARE\\%ws\\Setup"),TT_PRODUCT_NAME);

    int retval = regRead32(HKEY_LOCAL_MACHINE,szRegPlace,_T("InstallPath"),szInstallPath);

    return TT_RETVAL(0,retval);
}

int  TTInfo::getVBoxInstallPath(wchar_t* szInstallPath)
{
    wchar_t szRegPlace[MAX_PATH] = { 0 };

    int retval = getTtInstallPath(szRegPlace);

    wsprintf(szInstallPath,_T("%s\\vbox"),szRegPlace);

    return retval;
}

int  TTInfo::getTtDeployedPath(wchar_t* szInstallPath)
{
    wchar_t szRegPlace[MAX_PATH] = { 0 };

    int retval = getTtInstallPath(szRegPlace);

    wsprintf(szInstallPath,_T("%s\\deployed"),szRegPlace);

    return retval;
}

int  TTInfo::getTtDataPath(wchar_t* szDataPath)
{
    wchar_t szPath[MAX_PATH] = { 0 };
    int     retval = 0;

#if TT_PRODECT==TT_PRODUCT_KAOPU
    retval = getTtInstallPath(szPath);
#else
    SHGetSpecialFolderPath(NULL,szPath,CSIDL_LOCAL_APPDATA,FALSE);
#endif
    if (retval != 0)
        return retval;

    wsprintf(szDataPath,_T("%s\\TianTianData"),szPath);

    return 0;
}

int  TTInfo::getTtVersion(wchar_t* szVersion, wchar_t* szVersionEx)
{
    wchar_t szRegPlace[MAX_PATH] = { 0 };

    wsprintf(szRegPlace,_T("SOFTWARE\\%ws\\Setup"),TT_PRODUCT_NAME);

    int retval1 = 0, retval2 = 0;

    retval1 = regRead32(HKEY_LOCAL_MACHINE,szRegPlace,_T("Version"),szVersion);
    retval2 = regRead32(HKEY_LOCAL_MACHINE,szRegPlace,_T("VersionEx"),szVersion);

    if (retval1 !=0)
        return TT_RETVAL(0,retval1);
    else if (retval2 !=0)
        return TT_RETVAL(0,retval1);

    return 0;
}

int  TTInfo::setTtVersion(wchar_t* szVersion, wchar_t* szVersionEx)
{/*NEED*/
    return true;
}

int  TTInfo::getTtAdbChanId(void)
{
    return m_nTtAdbChanId;
}

int  TTInfo::getOpenglChanId(void)
{
    return m_nOpenglChanId;
}

int  TTInfo::getSensorChanId(void)
{
    return m_nSensorChanId;
}

int  TTInfo::getInputChanId(void)
{
    return m_nInputChanId;
}

int  TTInfo::getImeChanId(void)
{
    return m_nImeChanId;
}

int  TTInfo::getAppChangeChanId(void)
{
    return m_nAppChangeId;
}

int  TTInfo::getAdbChanId(void)
{
    return m_nAdbChanId;
}

/*
 * FIXME: may ini not exist,may should be create
 */

bool TTInfo::parseIniFile(void)
{
    TCHAR szFileName[256]=_T("");

    getTtInstallPath(szFileName);

    if (m_nEmuID==0)
        wsprintf(szFileName,_T("%s\\UserData\\TianTian\\%s"),szFileName,_T("TianTian.ini"));
    else
        wsprintf(szFileName,_T("%s\\UserData\\TianTian_%d\\%s"),szFileName,m_nEmuID,_T("TianTian.ini"));

    m_nOpenglChanId = ::GetPrivateProfileInt(_T("Port"),_T("OpenglPort"),TT_CHANNEL_OPENGL,szFileName);
    m_nTtAdbChanId  = ::GetPrivateProfileInt(_T("Port"),_T("TianTianAdbPort"),TT_CHANNEL_TTADB,szFileName);
    m_nSensorChanId = ::GetPrivateProfileInt(_T("Port"),_T("SensorPort"),TT_CHANNEL_SENSOR,szFileName);
    m_nImeChanId    = ::GetPrivateProfileInt(_T("Port"),_T("InputMethodPort"),TT_CHANNEL_IME,szFileName);
    m_nInputChanId  = ::GetPrivateProfileInt(_T("Port"),_T("InputPort"),TT_CHANNEL_INPUT,szFileName);
    m_nAdbChanId    = ::GetPrivateProfileInt(_T("Port"),_T("AdbPort"),TT_CHANNEL_ADB,szFileName);
    m_nAppChangeId  = ::GetPrivateProfileInt(_T("Port"),_T("AppChangePort"),TT_CHANNEL_APPCHANGE,szFileName);

    return true;
}

bool TTInfo::isUseGreenVBox(void)
{
    wchar_t szTtInstallPath[MAX_PATH] = { 0 };
    wchar_t szVBoxSvcFile[MAX_PATH]   = { 0 };

    getTtInstallPath(szTtInstallPath);

#if  TT_PRODECT==TT_PRODUCT_KAOPU
    wsprintf(szVBoxSvcFile,_T("%s\\vbox\\kpzsVMSVC.exe"),szTtInstallPath);
#else
    wsprintf(szVBoxSvcFile,_T("%s\\vbox\\TTVMSVC.exe"),szTtInstallPath);
#endif

    if (_waccess(szVBoxSvcFile,0)==-1)
        return false;

    return true;
}
