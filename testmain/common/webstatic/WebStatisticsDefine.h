/********************************************************************
//  作者:           LiJY
//  CopyRight(c)    2013 靠谱工作室 All Rights Reserved 
//  创建时间:       2013/10/22 9:32
//  类描述:          后台统计-相关定义宏或参数
//  修改时间:       
//  修改目的:       
*********************************************************************/
#pragma once

#include "utils/osUtil.h"
#include "ttinfo/ttinfo.h"

#define  EMULATE_TYPE       (8)
#define  DOWNLOAD_SOURCE    (16)

//////////////////////////////////////////////////////////////////////////调用宏
//一般的调用宏
#define WEB_STATISTICS( DO_ , SZ_PARAM) \
{\
    wchar_t szInstallPath[MAX_PATH] = { 0 };\
    wchar_t szCmd[MAX_PATH]         = { 0 };\
    TTInfo::getTtInstallPath(szInstallPath);\
    if(lstrlen(SZ_PARAM) != 0) \
        wsprintf(szCmd,_T("\"%s\\WebStatistics.exe\" %d \"%s\""),szInstallPath, DO_, SZ_PARAM);\
    else\
        wsprintf(szCmd,_T("\"%s\\WebStatistics.exe\" %d"),szInstallPath, DO_ );\
    createProcessAsync(szCmd);\
}

#define WEB_STATISTICS_NOP( DO_ ) WEB_STATISTICS(DO_,_T(""))

#define WEB_STATISTICS_ANDROID(DO_,nEmuID) \
{\
    wchar_t szInstallPath[MAX_PATH] = { 0 };\
    wchar_t szCmd[MAX_PATH]         = { 0 };\
    TTInfo::getTtInstallPath(szInstallPath);\
    if (nEmuID==0)\
        wsprintf(szCmd,_T("\"%s\\UserData\\TianTian\\WebStatistics.exe\" %d"),szInstallPath, DO_);\
    else\
        wsprintf(szCmd,_T("\"%s\\UserData\\TianTian_%d\\WebStatistics.exe\" %d"),szInstallPath,nEmuID, DO_);\
    createProcessAsync(szCmd);\
}

//功能相关的调用宏
#define WEB_STATISTICS_FUN_SOFT(FUN_TYPE_ )  \
{\
    wchar_t szInstallPath[MAX_PATH] = { 0 };\
    wchar_t szCmd[MAX_PATH]={0};\
    TTInfo::getTtInstallPath(szInstallPath);\
    wsprintf(szCmd,_T("\"%s\\WebStatistics.exe\" %d %d"),szInstallPath, DO_RECORD_FUN_DATA_SOFT, FUN_TYPE_);\
    createProcessAsync(szCmd);\
}

#define WEB_STATISTICS_FUN_APP(FUN_TYPE_ , SZ_APPID, SZ_APPNAME, SZ_EMUTYPE)  \
{\
    wchar_t szInstallPath[MAX_PATH] = { 0 };\
    wchar_t szParam[MAX_PATH]       ={0};\
    wchar_t szCmd[MAX_PATH]         ={0};\
    TTInfo::getTtInstallPath(szInstallPath);\
    wsprintf(szParam,_T("%d,%s,%s"),SZ_APPID, SZ_APPNAME, CAppInfoManagement::GetStartEmulator(SZ_EMUTYPE));\
    wsprintf(szCmd,_T("\"%s\\WebStatistics.exe\" %d %d \"%s\""),szInstallPath, DO_RECORD_FUN_DATA_APP, FUN_TYPE_, szParam);\
    createProcessAsync(szCmd);\
}

#define WEB_STATISTICS_FUN_TIME(FUN_TYPE_ , SZ_PARAM)  \
{\
    wchar_t szInstallPath[MAX_PATH] = { 0 };\
    wchar_t szCmd[MAX_PATH]         ={ 0 };\
    TTInfo::getTtInstallPath(szInstallPath);\
    if(lstrlen(SZ_PARAM) != 0)\
        wsprintf(szCmd,_T("\"%s\\WebStatistics.exe\" %d %d \"%s\""),szInstallPath, DO_RECORD_FUN_DATA_TIME, FUN_TYPE_, SZ_PARAM );\
    else\
        wsprintf(szCmd,_T("\"%s\\WebStatistics.exe\" %d %d"),szInstallPath, DO_RECORD_FUN_DATA_TIME, FUN_TYPE_ );\
    createProcessAsync(szCmd);\
}

#define WEB_STATISTICS_FUN_EMU(FUN_TYPE_ , SZ_EMUTYPE)  \
{\
    wchar_t szInstallPath[MAX_PATH] = { 0 };\
    wchar_t szCmd[MAX_PATH]={0};\
    TTInfo::getTtInstallPath(szInstallPath);\
    wsprintf(szCmd,_T("\"%s\\WebStatistics.exe\" %d %d"),szInstallPath, FUN_TYPE_, SZ_EMUTYPE);\
    createProcessAsync(szCmd);\
}

//密钥
#define KEY_ENCRPYT		_T("ouid23l1kj59nmvcxkj123ssa012kjldsf")
#define RANDOM			_T("30360529")

//支持后台统计的类型
enum {
    DO_SOFTWARE_INSTALL		= 1,		//软件安装	WebStatistics.exe DO_SOFTWARE_INSTALL 
    DO_SOFTWARE_UNINSTALL	= 2,		//软件卸载	WebStatistics.exe DO_SOFTWARE_UNINSTALL 
    DO_SOFTWARE_STARTING	= 3,		//软件启动	WebStatistics.exe DO_SOFTWARE_STARTING
    DO_SOFTWARE_INSTALL_SUCCESS   = 4,        //软件安装成功 WebStatistics.exe DO_SOFTWARE_INSTALL_SUCCESS
    DO_SOFTWARE_UNINSTALL_SUCCESS = 5,        //软件卸载成功 WebStatistics.exe DO_SOFTWARE_UNINSTALL_SUCCESS
    DO_SOFTWARE_START_SUCCESS     = 6,        //软件启动成功 WebStatistics.exe DO_SOFTWARE_START_SUCCESS


    DO_UPDATE_FUN_YESTERDAY_DATA	= 11,		//提交昨天功能统计数据	WebStatistics.exe DO_UPDATE_FUN_YESTERDAY_DATA 
    DO_RECORD_FUN_DATA				= 12,		//记录功能使用记录		WebStatistics.exe DO_RECORD_FUN_DATA FUN_TYPE_USED_APP [_T("szParam")]
    DO_UPDATE_FUN_TODAY_DATA		= 13,		//提交今天功能统计数据	WebStatistics.exe DO_UPDATE_FUN_TODAY_DATA 

    DO_RECORD_FUN_DATA_SOFT       = 14,        //记录软件功能相关记录        WebStatistics.exe DO_RECORD_FUN_DATA_SOFT FUN_TYPE_USED_APP [_T("szParam")]
    DO_RECORD_FUN_DATA_APP       = 15,        //记录应用统计相关记录        WebStatistics.exe DO_RECORD_FUN_DATA_APP FUN_TYPE_USED_APP [_T("szParam")]
    DO_RECORD_FUN_DATA_TIME      = 16,        //记录时间统计相关记录        WebStatistics.exe DO_RECORD_FUN_DATA_TIME FUN_TYPE_USED_APP [_T("szParam")]

    DO_UPDATE_FUN_CHECK_MAC        = 21,        //标准版BS安装检测的Mac数据提交 WebStatistics.exe DO_UPDATE_FUN_CHECK_MAC  URL_CHECK_RESULT_MAC_SUBMIT_PARAM_ZS
    DO_UPDATE_FUN_CHECK_MAC_JS        = 22,        //极速版安装检测的Mac数据提交 WebStatistics.exe DO_UPDATE_FUN_CHECK_MAC  URL_CHECK_RESULT_MAC_SUBMIT_PARAM_ZSJS
    DO_UPDATE_FUN_CHECK_MAC_TT        = 23,        //标准版天天安装检测的Mac数据提交 WebStatistics.exe DO_UPDATE_FUN_CHECK_MAC  URL_CHECK_RESULT_MAC_SUBMIT_PARAM_TT

    DO_EMULATOR_INSTALL             =31,                 //模拟器安装     WebStatistics.exe DO_EMULATOR_INSTALL  [iEmuType]
    DO_EMULATOR_UNINSTALL             =32,          //模拟器卸载    WebStatistics.exe DO_EMULATOR_INSTALL   [iEmuType]
    DO_EMULATOR_STARTING            =33,             //模拟器启动     WebStatistics.exe DO_EMULATOR_INSTALL  [iEmuType]
    DO_EMULATOR_DOWNLOAD_BEGING            =34,             //模拟器开始下载     WebStatistics.exe DO_EMULATOR_INSTALL  [iEmuType]
    DO_EMULATOR_DOWNLOAD_SUCCESS            =35,             //模拟器下载成功     WebStatistics.exe DO_EMULATOR_INSTALL  [iEmuType]
    DO_EMULATOR_INSTALL_BEGING            =36,             //模拟器开始安装     WebStatistics.exe DO_EMULATOR_INSTALL  [iEmuType]
    DO_EMULATOR_STARTING_SUCCESS            =37,             //模拟器启动成功     WebStatistics.exe DO_EMULATOR_INSTALL  [iEmuType]

    DO_EXTEND_RECORD                          =41,            //扩展统计

    DO_SET_DAT_INFO              = 98,        //更新统计的dat中软件信息    WebStatistics.exe DO_SET_DAT_INFO "{\"ProductVersion\":\"2.5.1138\"}"
    DO_GET_DAT_INFO              = 99,        //获取统计的dat中软件信息    WebStatistics.exe DO_GET_DAT_INFO "ProductVersion"
};

//////////////////////////////////////////////////////////////////////////靠谱相关软件安装与卸载
//统计的软件:靠谱助手，靠谱模拟器
enum {
    //SOFT_TYPE_BS=1,
    SOFT_TYPE_KPEMU=2,
    SOFT_TYPE_KPZS=3,
    SOFT_TYPE_KPZSJS=4,	//极速版
};

//途径类型ID：1靠谱助手 2百宝箱 3简单游
enum {
    ORG_TYPE_KPZS=1,	//靠谱网络
    ORG_TYPE_BBX=2,
    ORG_TYPE_JDY=3,
    ORG_TYPE_52PK=4,
    ORG_TYPE_KPZSJS=5,	//极速(有新的百度统计）

    ORG_TYPE_WEIBO=6,
    ORG_TYPE_FNDXNXQDZ=7,	//愤怒的小鸟2 fndxnxqdz2
    ORG_TYPE_JMTGCS=8,		//静默安装-推广测试
};


enum e_functionType
{
    FUN_TYPE_INCREASE_VOLUME = 0,  //音量增加
    FUN_TYPE_DECREASE_VOLUME = 1,   //音量降低
    FUN_TYPE_ROTATE_SCREEN = 2,     //转屏
    FUN_TYPE_BACK = 3,              //返回
    FUN_TYPE_RECENT_APPS = 4,       //最近应用
    FUN_TYPE_MENU = 5,              //菜单
    FUN_TYPE_HOME = 6,              //HOME(返回桌面)
    FUN_TYPE_POWER = 7,             //电源（锁屏）

    FUN_TYPE_DRAG_FILE = 8,         //拖拽文件
    FUN_TYPE_DRAG_APK = 9,         //拖拽APK

    FUN_TYPE_APPCENTER = 10,        //应用中心
    FUN_TYPE_KEYBORD_SET = 11,      //键盘编辑
    FUN_TYPE_SCREENSHOT = 12,       //截图
    FUN_TYPE_SHARE_FOLDER = 13,     //共享文件夹
    FUN_TYPE_CAMERA_ON  = 14,       //打开摄像头
    FUN_TYPE_CAMERA_OFF = 15,       //关闭摄像头
    FUN_TYPE_GPS        = 16,       //打开GPS

    FUN_TYPE_FULLSCREEN = 17,       //全屏

    FUNC_TYPE_BOSS_KEY  = 18,       //老板键

    FUNC_TYPE_TITLE_MENU = 19,      //标题栏菜单
    FUNC_TYPE_SETTING    = 20,      //设置
    FUNC_TYPE_REPORT     = 21,      //问题反馈
    FUNC_TYPE_GUIDE      = 22,     //新手引导
    FUNC_TYPE_VERSION_CHECK = 23,   //版本检测

    FUNC_TYPE_SETTING_NORNAL = 24,  //常用设置
    FUNC_TYPE_SETTING_ASVANCE = 25, //高级设置
    FUNC_TYPE_SETTING_ABOUT = 26,   //关于

    FUNC_TYPE_SETTING_SAVE   = 27,  //保存设置
    FUNC_TYPE_SETTING_CACEL  = 28,  //取消设置

    FUNC_TYPE_SETTING_REBOOT_NOW = 29,  //设置后立即重启
    FUNC_TYPE_SETTING_REBOOT_LATER = 30,//设置后稍后重启

    FUN_TYPE_MYAPP_INS_WEB_APP      = 33,        //安装某应用中心的应用  参数：序号，安装成功

    FUN_TYPE_MYAPP_INS_PC_APP        = 36,
    FUN_TYPE_MYAPP_RUN_PC_APP = 37,        //启动某本地安装的应用  参数：应用名

    FUN_TYPE_MULTI_START_VM        = 50,    //在多开管理器中启动镜像
    FUN_TYPE_MULTI_CLOSE_VM        = 51,    //在多开管理器中关闭镜像
    FUN_TYPE_MULTI_DELET_VM        = 52,    //在多开管理器中删除镜像
    FUN_TYPE_MULTI_CREAT_VM        = 53,    //在多开管理器中创建镜像
    FUN_TYPE_MULTI_CLONE_VM        = 54,    //在多开管理器中克隆镜像
    FUN_TYPE_MULTI_CLONEALL_VM     = 55,    //在多开管理器中关闭所有镜像
    FUN_TYPE_MULTI_LAUNCH          = 56,    //启动多开管理器
    FUN_TYPE_MULTI_CLOSE           = 57,    //关闭多开管理器
    FUN_TYPE_MULTI_MINIMIZE        = 58,    //最小化多开管理器
    FUN_TYPE_MULTI_CREAT_FAILED    = 59,    //新建模拟器失败
    FUN_TYPE_MULTI_CLONE_FAILED    = 60,    //克隆模拟器失败

    FUN_TYPE_MYAPP_DOWNLOAD_START   =344,        //开始下载应用，包括继续和更新
    FUN_TYPE_MYAPP_DOWNLOAD_SUCCESS = 340,      //应用下载成功
    FUN_TYPE_MYAPP_START_INS        = 341,      //应用开始安装
    FUN_TYPE_MYAPP_START_INS_SUCCESS  = 342,      //应用开始安装
    FUN_TYPE_MYAPP_INTALL_FAIL       =347,        //应用安装失败原因

    //////////////////////////////////////////////////////////////////////////插件相关功能(900~999)
    FUN_TYPE_PLUGIN_INSTALL                    = 900,          //安装(无区分模拟器）
    FUN_TYPE_PLUGIN_START                      = 901,          //启动(有区分模拟器）
    FUN_TYPE_PLUGIN_UNINSTALL                  = 902,          //卸载

    FUN_TYPE_PLUGIN_EDITCFG_COMBOSETTING       = 903,        //工具-键盘操作-连招设置

    FUN_TYPE_PLUGIN_DOWNLOAD                   = 904,          //下载(无区分模拟器）
    FUN_TYPE_PLUGIN_UPDATE                     = 905,          //更新(无区分模拟器）

    ///////////
    FUN_TYPE_MYAPP_RUN_APP        = 1000,        //开始启动应用  参数：序号
    FUN_TYPE_MYAPP_RUN_APP_FAIL        = 1001,   //启动应用失败  参数：序号

    FUN_TYPE_MYAPP_UNINS_APP_SUCCESS    = 1002,   //卸载应用成功  参数：序号
    FUN_TYPE_MYAPP_UNINS_APP_FAIL    = 1003,       //卸载应用失败  参数：序号
    FUN_TYPE_MYAPP_UNINS_APP         = 1004,       //开始卸载应用

    FUN_TYPE_FIXER_TIMES             = 1100,       //触发异常修复界面次数
    FUN_TYPE_FIXER_MULTI_TIMES       = 1101,       //同一用户触发多次异常界面次数
    FUN_TYPE_FIXER_FIXNOW            = 1102,       //立即修复次数
    FUN_TYPE_FIXER_CONNECT_SERVER_QQ = 1103,       //找客服求助次数
    FUN_TYPE_FIXER_CHANGE_KPZS       = 1104,       //更换模拟器次数
    FUN_TYPE_FIXER_360VT             = 1105,       //360VT未开同时无法获取IP
    FUN_TYPE_FIXER_MEMLOW_START      = 1106,       //启动前可用内存不足无法启动
    FUN_TYPE_FIXER_MEMLOW_RUN        = 1107,       //启动中可用内存不足无法启动
    FUN_TYPE_FIXER_MEMLOW_START_EXIT = 1108,       //启动前内存不足，点击退出按钮
    FUN_TYPE_FIXER_MEMLOW_RUN_EXIT   = 1109,       //启动中内存不足，点击退出按钮
    FUN_TYPE_KA99_BUT_SUCCESS        = 1110,       //卡99但启动成功数据
    FUN_TYPE_KA99_BUT_NOT_SHOW_FIXER = 1111,       //进度条走到99(80s~150s间)用户关闭模拟器
    FUN_TYPE_SET_DX_ENABLE           = 1112,       //设置界面选中DX模式，并生效的次数
    FUN_TYPE_FUNBAR_HIDEN            = 1113,       //侧边栏隐藏按钮点击次数
    FUN_TYPE_FUNBAR_INSTALLBTN       = 1114,       //本地安装按钮点击次数
    FUN_TYPE_SET_DX_ENABLE_WHENSTART = 1115,       //启动前选中DX模式，并生效的次数
    FUN_TYPE_SET_GL_ENABLE_WHENSTART = 1116,       //启动前选中GL模式，并生效的次数
    FUN_TYPE_SET_GL_ENABLE           = 1117,       //设置界面选中GL模式，并生效的次数
    FUN_TYPE_VT_NO_OPEN              = 1118,       //每日显示VT未开启提示icon的MAC数
    FUN_TYPE_VT_BTN_CLICK            = 1119,       //VT按钮的点击数\MAC数
    FUN_TYPE_DETAILED_STRATEGY_CLICK = 1120,       //详细攻略按钮点击数\MAC数
    FUN_TYPE_CONTACT_CUSTOM_SERVICE  = 1121,       //联系客服的点击数\MAC数
    FUN_TYPE_HAND_SHAKEBTN           = 1122,        //摇一摇按钮点击次数
    FUN_TYPE_BAIDU_BROWSER           = 1123,         //百度浏览器启动次数
    FUN_TYPE_PLUGINTOOLS_OPEN        = 1124,        //工具箱点击次数
    FUN_TYPE_PLUGIN_FEIWO            = 1125,       //蜂窝助手按钮点击次数
    FUN_TYPE_FEIWOTIPS_CHECKED       = 1126,       //蜂窝弹框选择下次不再提示
    FUN_TYPE_FEIWOTIPS_CLOSE         = 1127,       //蜂窝弹框关闭按钮点击次数
    FUN_TYPE_FEIWOTIPS_STARTINSTLL   = 1128,       //蜂窝弹框点击一键试用次数
    FUN_TYPE_FEIWOTIPS_SHOWDETAIL    = 1129,       //蜂窝弹框点击查看详情次数
    FUN_TYPE_LOADING_BTNCLICK        = 1130,       //loading图点击下载
    FUN_TYPE_SHOW_RECOMMENDDLG       = 1131,       //推荐apk界面显示次数\Mac数
    FUN_TYPE_RECDLG_INSTALL_BTNCLICK = 1132,       //一键安装按钮，点击次数\Mac数
    FUN_TYPE_RECDLG_CONTINUE_BTNCLICK= 1133,       //仍然继续按钮，点击次数
    FUN_TYPE_NOSHOW_RECOMMENDDLG     = 1134,       //不再提示，勾选并且生效的次数\Mac数
    FUN_TYPE_BACKUP_SUCCESS          = 1135,       //导出成功
    FUN_TYPE_RECOVERY_SUCCESS        = 1136,       //导入成功
    FUN_TYPE_BACKUP_FAIL             = 1137,       //导出失败
    FUN_TYPE_RECOVERY_FAIL           = 1138,       //导入失败
    FUN_TYPE_BACKUP_TOTAL            = 1139,       //导出总次数
    FUN_TYPE_RECOVERY_TOTAL          = 1140,       //导入总次数
    FUN_TYPE_ADVERTDLG_SHOW          = 1141,       //广告弹窗出现次数
    FUN_TYPE_ADVERTDLG_VIEW          = 1142        //查看内容次数

    // 镜像内部统计区间[2049,4098]

};