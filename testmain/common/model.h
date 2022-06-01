
#pragma  once;

/************************************************************************/
/* product defined                                                      */
/************************************************************************/

#define TT_PRODUCT_MAIN                 7
#define TT_PRODUCT_NONE                 TT_PRODUCT_MAIN
#define TT_PRODUCT_KAOPU                1
#define TT_PRODUCT_HPL                  2
#define TT_PRODUCT_KOPLAYER             3
#define TT_PRODUCT_OEM                  4
#define TT_PRODUCT_SINA                 5
#define TT_PRODUCT_ECY                  6

/*
 * vbox version defined
 */
#define TT_VBOX_VERSION4_3_34           (40334)
#define TT_VBOX_VERSION5_0_8            (50008)

#include "define.h"

#if  TT_PRODECT == TT_PRODUCT_KAOPU

#define TT_PRODUCT_NAME                _T("KPZSTianTian")

#define TT_EXT_PIPE_NAME               "kptiantianextsvcTianTian"
#define TT_EXT_NOTIFY_PIPE_NAME        "kptiantianextsvc-notifyTianTian"

#define TT_CHANNEL_ADB                  (40005)

#define TT_CHANNEL_PORT_BEGIN           (40000)

#define TT_PRODUCT_NAME                _T("KPZSTianTian")
#define TT_VBOX_DRV_NAME               _T("kpzsDrv")
#define TT_VBOX_DRV_DESC               _T("kpzs Support Driver")
#define TT_VMSVC_NAME                  _T("kpzsVMSVC")
#define TT_VBOX_GLOABLE_CFG            _T("kpzsVM.xml")
#define TT_VBOX_GLOABLE_CFG_NAME       _T(".kpzsVM")

#define TT_VBOX_VERSION                 TT_VBOX_VERSION5_0_8

#if TT_VBOX_VERSION == TT_VBOX_VERSION4_3_34

#define MSCOM_VBOX_SESSION_CLSID       _T("{9f2af5ca-48ef-4027-9fcf-11fe22ef57d0}")
#define MSCOM_VBOX_CLIENT_CLSID        _T("{4b1457fa-b77d-467b-babd-3ecd499d3f3d}")
#define MSCOM_VBOX_VIRTUALBOX          _T("{77413abd-7c2c-4cd0-b801-abc268123f19}")

#elif TT_VBOX_VERSION == TT_VBOX_VERSION5_0_8

#define MSCOM_VBOX_SESSION_CLSID       _T("{be2f4bb7-6213-4dc8-aed7-ed728ff10dbc}")
#define MSCOM_VBOX_CLIENT_CLSID        _T("{ad7d4410-d18c-4c75-889e-7b392bef95da}")
#define MSCOM_VBOX_VIRTUALBOX          _T("{59b7435c-2d3f-4d6b-a4b7-2eedc220ae5e}")

#else

#error("vbox version not defined\n");

#endif

#elif TT_PRODECT == TT_PRODUCT_HPL

#define TT_PRODUCT_NAME                 _T("HPLAssistant")

#define TT_EXT_PIPE_NAME               "tiantianextsvcTianTian"

#define TT_CHANNEL_ADB                  (6555)

#define TT_CHANNEL_PORT_BEGIN           (30000)

#define TT_PRODUCT_NAME                _T("TianTian")
#define TT_VBOX_DRV_NAME               _T("TTDrv")
#define TT_VBOX_DRV_DESC               _T("TianTian Support Driver")
#define TT_VMSVC_NAME                  _T("TTVMSVC")
#define MSCOM_VBOX_SESSION_CLSID       _T("{6cc574af-845b-4845-a3fc-4d607e38092d}")
#define MSCOM_VBOX_CLIENT_CLSID        _T("{d21309d2-212e-4a66-9a85-333f83413ab5}")
#define MSCOM_VBOX_VIRTUALBOX          _T("{7875ada3-6d4a-4b7d-ab09-be2a35358359}")

#elif TT_PRODECT == TT_PRODUCT_OEM

#define TT_VERSION                     _T("2.5.5")
#define TT_PRODUCT_NAME                _T("OEMTianTian")

#define TT_KEY_IGNORE_ESC

#define TT_EXT_PIPE_NAME               "oemtiantianextsvcTianTian"
#define TT_EXT_NOTIFY_PIPE_NAME        "oemiantianextsvc-notifyTianTian"

#define TT_CHANNEL_ADB                  (50005)

#define TT_CHANNEL_PORT_BEGIN           (50000)

#define TT_PRODUCT_NAME                _T("OEMTianTian")
#define TT_VBOX_DRV_NAME               _T("OEMDrv")
#define TT_VBOX_DRV_DESC               _T("TianTianOEM Support Driver")
#define TT_VMSVC_NAME                  _T("OEMVMSVC")
#define TT_VBOX_GLOABLE_CFG            _T("TianTianOEMVM.xml")
#define TT_VBOX_GLOABLE_CFG_NAME       _T(".TianTianOEMVM")

#define TT_VBOX_VERSION                TT_VBOX_VERSION5_0_8

#if TT_VBOX_VERSION == TT_VBOX_VERSION4_3_34

#define MSCOM_VBOX_SESSION_CLSID       _T("{ec13b751-ea8e-4015-9120-6e015bf99572}")
#define MSCOM_VBOX_CLIENT_CLSID        _T("{d53e327b-7849-4eb2-a764-78a78197e139}")
#define MSCOM_VBOX_VIRTUALBOX          _T("{88b232ff-e1ab-4fe6-94cb-88b19ec5d79a}")

#elif TT_VBOX_VERSION == TT_VBOX_VERSION5_0_8

#define MSCOM_VBOX_SESSION_CLSID       _T("{33ab65f2-644a-41cf-b654-ad53eb0a2113}")
#define MSCOM_VBOX_CLIENT_CLSID        _T("{5be79459-3b68-478c-adf9-cb63dec97f48}")
#define MSCOM_VBOX_VIRTUALBOX          _T("{0001b269-d769-411f-9587-bf1572f338f7}")

#else

#error("vbox version not defined\n");

#endif

#elif TT_PRODECT == TT_PRODUCT_SINA

#define TT_PRODUCT_NAME                _T("SINATianTian")

#define TT_KEY_IGNORE_ESC

#define TT_EXT_PIPE_NAME               "sinatiantianextsvcTianTian"
#define TT_EXT_NOTIFY_PIPE_NAME        "sinatiantianextsvc-notifyTianTian"

#define TT_CHANNEL_ADB                  (60005)

#define TT_CHANNEL_PORT_BEGIN           (60000)

#define TT_PRODUCT_NAME                _T("SINATianTian")
#define TT_VBOX_DRV_NAME               _T("sinaEDrv")
#define TT_VBOX_DRV_DESC               _T("TianTianSINA Support Driver")
#define TT_VMSVC_NAME                  _T("sinaEVMSVC")
#define TT_VBOX_GLOABLE_CFG            _T("sinaEVM.xml")
#define TT_VBOX_GLOABLE_CFG_NAME       _T(".sinaEVM")

#define MSCOM_VBOX_SESSION_CLSID       _T("{c9b53c3d-a889-4ff8-9d85-905530ada8e4}")
#define MSCOM_VBOX_CLIENT_CLSID        _T("{22a61e5d-a360-4eeb-9e25-31f7e5444b0b}")
#define MSCOM_VBOX_VIRTUALBOX          _T("{320bf857-e85d-417f-8201-d2c443f04695}")

#elif TT_PRODECT == TT_PRODUCT_ECY

#define ANDROIDVERION                  40404       /*4.04.04, 模拟器3.0之前版本*/

#define TT_CFG_MONITOR_AUTO

#define TT_PRODUCT_NAME                _T("ECYTianTian")

#define TT_KEY_IGNORE_ESC

#define TT_EXT_PIPE_NAME               "ecytiantianextsvcTianTian"
#define TT_EXT_NOTIFY_PIPE_NAME        "ecytiantianextsvc-notifyTianTian"

#define TT_CHANNEL_ADB                  (55005)

#define TT_CHANNEL_PORT_BEGIN           (55000)

#define TT_PRODUCT_NAME                _T("ECYTianTian")
#define TT_VBOX_DRV_NAME               _T("ecyDrv")
#define TT_VBOX_DRV_DESC               _T("TianTianECY Support Driver")
#define TT_VMSVC_NAME                  _T("ecyVMSVC")
#define TT_VBOX_GLOABLE_CFG            _T("ecyVM.xml")
#define TT_VBOX_GLOABLE_CFG_NAME       _T(".ecyVM")

#define MSCOM_VBOX_SESSION_CLSID       _T("{8eb1bc01-de53-470c-a74f-b08ab9468cc8}")
#define MSCOM_VBOX_CLIENT_CLSID        _T("{b387b29c-36bc-4c8c-97c3-cebf998e7217}")
#define MSCOM_VBOX_VIRTUALBOX          _T("{02e34d4f-0cea-4bd9-834c-d888157a7fb7}")

#else

#define TT_PRODUCT_NAME                _T("ZhangYu")
#define TT_FILEANDDIR_NAME             _T("Android")
#define TT_BASEDATA_NAME               _T("AndroidBase")
#define TT_USER_DATA_NAME              _T("ZhangYuData")
#define TT_GPS_FILE                    _T("gps.xml")
#define TT_ADB_EXE                     _T("zyadb.exe")
#define TT_EXT_HELPER_DLL              _T("zyextclient.dll")
#define TT_REGSVR_EXE                  _T("zyregsvr.exe")
#define TT_JOYSTICK_DLL                _T("zyjoystick.dll")
#define TT_KILLALL_EXE                 _T("zykillall.exe")

#define TT_CFG_MONITOR_AUTO

#define TT_EXT_PIPE_NAME               "ZhangYuextsvc"
#define TT_EXT_NOTIFY_PIPE_NAME        "ZhangYuextsvc-notify"

#define TT_VBOX_DRV_NAME                _T("octopusDrv")
#define TT_VBOX_DRV_DESC                _T("octopus Support Driver")
#define TT_VMSVC_NAME                   _T("octopusVMSVC")
#define TT_VBOX_GLOABLE_CFG             _T("octopusVM.xml")
#define TT_VBOX_GLOABLE_CFG_NAME        _T(".octopusVM")

#define TT_VBOX_NETLWF_NAME             _T("octopusNetLwf")
#define TT_VBOX_NETLWF_DESC             _T("octopus NDIS6 Bridged Networking Driver")

#define TT_CHANNEL_ADB                  (6555)

#define TT_CHANNEL_PORT_BEGIN           (56000)

#define TT_VBOX_DRV_NAME               _T("octopusDrv")
#define TT_VMSVC_NAME                  _T("octopusVMSVC")

#define TT_VBOX_VERSION                 TT_VBOX_VERSION5_0_8

#define MSCOM_VBOX_SESSION_CLSID       _T("{69a6be63-38be-4a6c-b67f-beac379dae15}")
#define MSCOM_VBOX_CLIENT_CLSID        _T("{f0c1d5d1-2af8-417f-9f30-78b66a31ce56}")
#define MSCOM_VBOX_VIRTUALBOX          _T("{d243f25f-ecf9-4a6e-9906-9c937013ed39}")

#endif /*TT_PRODECT_KAOPU*/

/************************************************************************/
/*                                                                      */
/************************************************************************/

#define  WM_SHOW_FPS       (WM_USER + 200)
#define  WM_JK_PLUG        (WM_USER + 201)

#define  WM_IME_ENABLE     (WM_USER + 210)
#define  WM_MSG_ENABLE     (WM_USER + 211)
/************************************************************************/
/*                                                                      */
/************************************************************************/
#include "error/tterrorcode.h"

#if 0
/*
 * lock defined
 */
#define TT_LOCK_CREATE()        ::CreateMutex(NULL,false,NULL)
#define TT_GLOCK_CREATE(name)   ::CreateMutex(NULL,false,name)
#define TT_LOCK_DESTROY(lock)   ::CloseHandle(lock)
#define TT_LOCK(lock)           ::WaitForSingleObject(lock, INFINITE)
#define TT_UNLOCK(lock)         ::ReleaseMutex(lock)

#else

#include "utils/lock.h"

#define TT_LOCK_CREATE()        (HANDLE)(new Mutex());
//#define TT_GLOCK_CREATE(name)   ::CreateMutex(NULL,false,name)
#define TT_LOCK_DESTROY(ttlock)   delete (Mutex*)ttlock;
#define TT_LOCK(ttlock)           ((Mutex*)ttlock)->lock();
#define TT_UNLOCK(ttlock)         ((Mutex*)ttlock)->unlock();

#endif

/*
 * retval
 */
static inline int TT_RETVAL(int rettype, int retval)
{
    if (retval != 0)
        retval = TT_ERROR(rettype,retval);

    return retval;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/


/*
 * debug log
 */
#define TTDEBUG_ENABLE                 1
#define TTDEBUG_LOG_TOFILE             1
#define TTDEBUG_LOG_DETAIL             1
#define TTDEBUG_LOG_POSITON            0
#define TTDEBUG_LOG_LEVEL              4

#include <tchar.h>

#ifndef TT_DEBUG_EXTERNAL

#if    TTDEBUG_ENABLE

#include "ttlog.h"
#define  TT_LOG_ERR(...)                LG_ERR(_T("TianTian"),__VA_ARGS__)
#define  TT_LOG_WARN(...)               LG_WARN(_T("TianTian"),__VA_ARGS__)
#define  TT_LOG_INFO(...)               LG_INFO(_T("TianTian"),__VA_ARGS__)
#define  TT_LOG_DBUG(...)               LG_DBUG(_T("TianTian"),__VA_ARGS__)
#else
#define  ttlog_init(nEmuID)
#define  ttlog_exit()
#define  TT_LOG_ERR(...)
#define  TT_LOG_WARN(...)
#define  TT_LOG_INFO(...)
#define  TT_LOG_DBUG(...)
#endif /*TT_DEBUG_DISABLE_OUTSIDE*/

#else

#define  ttlog_init(nEmuID)
#define  ttlog_exit()
#define  TT_LOG_ERR(...)
#define  TT_LOG_WARN(...)
#define  TT_LOG_INFO(...)
#define  TT_LOG_DBUG(...)

#endif /*TT_DEBUG_EXTERNAL*/



#include "error/tterrorcode.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/



#define RET_STRING_OUTPUT(str)      fprintf(stdout,"%s\n",str)

#define CS_ERROR_PARAM              "{\"szResMsg\":\"error: cann't analyse cmd.\"}"
#define CS_ERROR_NO_RUNNING         "{\"szResMsg\":\"error: tiantian not runing.\"}"
#define CS_SUCCESS_PARAM            "{\"szResMsg\":\"success\"}"

#define PLUS_INFINITY               (0xEFFFFFFF)
