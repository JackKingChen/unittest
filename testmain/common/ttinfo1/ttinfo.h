
#pragma once;

#include <string>
#include <map>

#include <Windows.h>

class TTInfo
{
private:
    TTInfo(int nEmuID);
    ~TTInfo(void);

public:
    static  int  getClassObject(TTInfo*& pTtInfo,int nEmuID);

    static  int  getTtInstallPath(wchar_t* szInstallPath);
    static  int  getVBoxInstallPath(wchar_t* szInstallPath);
    static  int  getTtDeployedPath(wchar_t* szInstallPath);

    static  int  getTtDataPath(wchar_t* szDataPath);
    //static  int  getVBoxInstallPath(wchar_t* szInstallPath);
    static  int  getTtVersion(wchar_t* szVersion, wchar_t* szVersionEx);
    static  int  setTtVersion(wchar_t* szVersion, wchar_t* szVersionEx);
    static  bool isUseGreenVBox(void);

    int  getTtAdbChanId(void);
    int  getOpenglChanId(void);
    int  getSensorChanId(void);
    int  getInputChanId(void);
    int  getImeChanId(void);
    int  getAppChangeChanId(void);
    int  getAdbChanId(void);

private:
    bool parseIniFile(void);

private:
    static std::map<int,TTInfo*> m_mapTtInfo;
    static HANDLE                m_hMapLock;

private:
    std::wstring m_szPortCfgPath;
    std::wstring m_szTTInstallPath;
    std::wstring m_szVBoxInstallPath;

    int  m_nTtAdbChanId;
    int  m_nOpenglChanId;
    int  m_nSensorChanId;
    int  m_nInputChanId;
    int  m_nImeChanId;
    int  m_nAppChangeId;
    int  m_nAdbChanId;

private:
    int  m_nEmuID;
};

/************************************************************************/
/* product defined                                                      */
/************************************************************************/

#define TT_PRODUCT_NONE                 0
#define TT_PRODUCT_KAOPU                1
#define TT_PRODUCT_HPL                  2

#define TT_PRODECT                      TT_PRODUCT_KAOPU

#if  TT_PRODECT==TT_PRODUCT_KAOPU

#define TT_PRODUCT_NAME                _T("KPZSTianTian")

#define TT_EXT_PIPE_NAME               "kptiantianextsvcTianTian"
#define TT_EXT_NOTIFY_PIPE_NAME        "kptiantianextsvc-notifyTianTian"

#define TT_CHANNEL_OPENGL               (40000)
#define TT_CHANNEL_TTADB                (40001)
#define TT_CHANNEL_SENSOR               (40002)
#define TT_CHANNEL_IME                  (40003)
#define TT_CHANNEL_INPUT                (40004)
#define TT_CHANNEL_ADB                  (40005)
#define TT_CHANNEL_APPCHANGE            (40006)

#define TT_CHANNEL_PORT_BEGIN           (40000)

#elif TT_PRODECT == TT_PRODUCT_HPL

#define TT_PRODUCT_NAME                 _T("HPLAssistant")

#define TT_EXT_PIPE_NAME               "tiantianextsvcTianTian"

#define TT_CHANNEL_OPENGL               (22468)
#define TT_CHANNEL_TTADB                (9874)
#define TT_CHANNEL_SENSOR               (22471)
#define TT_CHANNEL_IME                  (6321)
#define TT_CHANNEL_INPUT                (22469)
#define TT_CHANNEL_ADB                  (6555)
#define TT_CHANNEL_APPCHANGE            (9873)

#define TT_CHANNEL_PORT_BEGIN           (30000)

#else

#define TT_PRODUCT_NAME                "TianTian"

#define TT_EXT_PIPE_NAME               "tiantianextsvcTianTian"

#define TT_CHANNEL_OPENGL               (22468)
#define TT_CHANNEL_TTADB                (9874)
#define TT_CHANNEL_SENSOR               (22471)
#define TT_CHANNEL_IME                  (6321)
#define TT_CHANNEL_INPUT                (22469)
#define TT_CHANNEL_ADB                  (6555)
#define TT_CHANNEL_APPCHANGE            (9873)

#define TT_CHANNEL_PORT_BEGIN           (30000)

#endif /*TT_PRODECT_KAOPU*/

/************************************************************************/
/*                                                                      */
/************************************************************************/
