/********************************************************************
//  ����:           LiJY
//  CopyRight(c)    2013 ���׹����� All Rights Reserved 
//  ����ʱ��:       2013/10/22 9:32
//  ������:          ��̨ͳ��-��ض��������
//  �޸�ʱ��:       
//  �޸�Ŀ��:       
*********************************************************************/
#pragma once

#include "utils/osUtil.h"
#include "ttinfo/ttinfo.h"

#define  EMULATE_TYPE       (8)
#define  DOWNLOAD_SOURCE    (16)

//////////////////////////////////////////////////////////////////////////���ú�
//һ��ĵ��ú�
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

//������صĵ��ú�
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

//��Կ
#define KEY_ENCRPYT		_T("ouid23l1kj59nmvcxkj123ssa012kjldsf")
#define RANDOM			_T("30360529")

//֧�ֺ�̨ͳ�Ƶ�����
enum {
    DO_SOFTWARE_INSTALL		= 1,		//�����װ	WebStatistics.exe DO_SOFTWARE_INSTALL 
    DO_SOFTWARE_UNINSTALL	= 2,		//���ж��	WebStatistics.exe DO_SOFTWARE_UNINSTALL 
    DO_SOFTWARE_STARTING	= 3,		//�������	WebStatistics.exe DO_SOFTWARE_STARTING
    DO_SOFTWARE_INSTALL_SUCCESS   = 4,        //�����װ�ɹ� WebStatistics.exe DO_SOFTWARE_INSTALL_SUCCESS
    DO_SOFTWARE_UNINSTALL_SUCCESS = 5,        //���ж�سɹ� WebStatistics.exe DO_SOFTWARE_UNINSTALL_SUCCESS
    DO_SOFTWARE_START_SUCCESS     = 6,        //��������ɹ� WebStatistics.exe DO_SOFTWARE_START_SUCCESS


    DO_UPDATE_FUN_YESTERDAY_DATA	= 11,		//�ύ���칦��ͳ������	WebStatistics.exe DO_UPDATE_FUN_YESTERDAY_DATA 
    DO_RECORD_FUN_DATA				= 12,		//��¼����ʹ�ü�¼		WebStatistics.exe DO_RECORD_FUN_DATA FUN_TYPE_USED_APP [_T("szParam")]
    DO_UPDATE_FUN_TODAY_DATA		= 13,		//�ύ���칦��ͳ������	WebStatistics.exe DO_UPDATE_FUN_TODAY_DATA 

    DO_RECORD_FUN_DATA_SOFT       = 14,        //��¼���������ؼ�¼        WebStatistics.exe DO_RECORD_FUN_DATA_SOFT FUN_TYPE_USED_APP [_T("szParam")]
    DO_RECORD_FUN_DATA_APP       = 15,        //��¼Ӧ��ͳ����ؼ�¼        WebStatistics.exe DO_RECORD_FUN_DATA_APP FUN_TYPE_USED_APP [_T("szParam")]
    DO_RECORD_FUN_DATA_TIME      = 16,        //��¼ʱ��ͳ����ؼ�¼        WebStatistics.exe DO_RECORD_FUN_DATA_TIME FUN_TYPE_USED_APP [_T("szParam")]

    DO_UPDATE_FUN_CHECK_MAC        = 21,        //��׼��BS��װ����Mac�����ύ WebStatistics.exe DO_UPDATE_FUN_CHECK_MAC  URL_CHECK_RESULT_MAC_SUBMIT_PARAM_ZS
    DO_UPDATE_FUN_CHECK_MAC_JS        = 22,        //���ٰ氲װ����Mac�����ύ WebStatistics.exe DO_UPDATE_FUN_CHECK_MAC  URL_CHECK_RESULT_MAC_SUBMIT_PARAM_ZSJS
    DO_UPDATE_FUN_CHECK_MAC_TT        = 23,        //��׼�����찲װ����Mac�����ύ WebStatistics.exe DO_UPDATE_FUN_CHECK_MAC  URL_CHECK_RESULT_MAC_SUBMIT_PARAM_TT

    DO_EMULATOR_INSTALL             =31,                 //ģ������װ     WebStatistics.exe DO_EMULATOR_INSTALL  [iEmuType]
    DO_EMULATOR_UNINSTALL             =32,          //ģ����ж��    WebStatistics.exe DO_EMULATOR_INSTALL   [iEmuType]
    DO_EMULATOR_STARTING            =33,             //ģ��������     WebStatistics.exe DO_EMULATOR_INSTALL  [iEmuType]
    DO_EMULATOR_DOWNLOAD_BEGING            =34,             //ģ������ʼ����     WebStatistics.exe DO_EMULATOR_INSTALL  [iEmuType]
    DO_EMULATOR_DOWNLOAD_SUCCESS            =35,             //ģ�������سɹ�     WebStatistics.exe DO_EMULATOR_INSTALL  [iEmuType]
    DO_EMULATOR_INSTALL_BEGING            =36,             //ģ������ʼ��װ     WebStatistics.exe DO_EMULATOR_INSTALL  [iEmuType]
    DO_EMULATOR_STARTING_SUCCESS            =37,             //ģ���������ɹ�     WebStatistics.exe DO_EMULATOR_INSTALL  [iEmuType]

    DO_EXTEND_RECORD                          =41,            //��չͳ��

    DO_SET_DAT_INFO              = 98,        //����ͳ�Ƶ�dat�������Ϣ    WebStatistics.exe DO_SET_DAT_INFO "{\"ProductVersion\":\"2.5.1138\"}"
    DO_GET_DAT_INFO              = 99,        //��ȡͳ�Ƶ�dat�������Ϣ    WebStatistics.exe DO_GET_DAT_INFO "ProductVersion"
};

//////////////////////////////////////////////////////////////////////////������������װ��ж��
//ͳ�Ƶ����:�������֣�����ģ����
enum {
    //SOFT_TYPE_BS=1,
    SOFT_TYPE_KPEMU=2,
    SOFT_TYPE_KPZS=3,
    SOFT_TYPE_KPZSJS=4,	//���ٰ�
};

//;������ID��1�������� 2�ٱ��� 3����
enum {
    ORG_TYPE_KPZS=1,	//��������
    ORG_TYPE_BBX=2,
    ORG_TYPE_JDY=3,
    ORG_TYPE_52PK=4,
    ORG_TYPE_KPZSJS=5,	//����(���µİٶ�ͳ�ƣ�

    ORG_TYPE_WEIBO=6,
    ORG_TYPE_FNDXNXQDZ=7,	//��ŭ��С��2 fndxnxqdz2
    ORG_TYPE_JMTGCS=8,		//��Ĭ��װ-�ƹ����
};


enum e_functionType
{
    FUN_TYPE_INCREASE_VOLUME = 0,  //��������
    FUN_TYPE_DECREASE_VOLUME = 1,   //��������
    FUN_TYPE_ROTATE_SCREEN = 2,     //ת��
    FUN_TYPE_BACK = 3,              //����
    FUN_TYPE_RECENT_APPS = 4,       //���Ӧ��
    FUN_TYPE_MENU = 5,              //�˵�
    FUN_TYPE_HOME = 6,              //HOME(��������)
    FUN_TYPE_POWER = 7,             //��Դ��������

    FUN_TYPE_DRAG_FILE = 8,         //��ק�ļ�
    FUN_TYPE_DRAG_APK = 9,         //��קAPK

    FUN_TYPE_APPCENTER = 10,        //Ӧ������
    FUN_TYPE_KEYBORD_SET = 11,      //���̱༭
    FUN_TYPE_SCREENSHOT = 12,       //��ͼ
    FUN_TYPE_SHARE_FOLDER = 13,     //�����ļ���
    FUN_TYPE_CAMERA_ON  = 14,       //������ͷ
    FUN_TYPE_CAMERA_OFF = 15,       //�ر�����ͷ
    FUN_TYPE_GPS        = 16,       //��GPS

    FUN_TYPE_FULLSCREEN = 17,       //ȫ��

    FUNC_TYPE_BOSS_KEY  = 18,       //�ϰ��

    FUNC_TYPE_TITLE_MENU = 19,      //�������˵�
    FUNC_TYPE_SETTING    = 20,      //����
    FUNC_TYPE_REPORT     = 21,      //���ⷴ��
    FUNC_TYPE_GUIDE      = 22,     //��������
    FUNC_TYPE_VERSION_CHECK = 23,   //�汾���

    FUNC_TYPE_SETTING_NORNAL = 24,  //��������
    FUNC_TYPE_SETTING_ASVANCE = 25, //�߼�����
    FUNC_TYPE_SETTING_ABOUT = 26,   //����

    FUNC_TYPE_SETTING_SAVE   = 27,  //��������
    FUNC_TYPE_SETTING_CACEL  = 28,  //ȡ������

    FUNC_TYPE_SETTING_REBOOT_NOW = 29,  //���ú���������
    FUNC_TYPE_SETTING_REBOOT_LATER = 30,//���ú��Ժ�����

    FUN_TYPE_MYAPP_INS_WEB_APP      = 33,        //��װĳӦ�����ĵ�Ӧ��  ��������ţ���װ�ɹ�

    FUN_TYPE_MYAPP_INS_PC_APP        = 36,
    FUN_TYPE_MYAPP_RUN_PC_APP = 37,        //����ĳ���ذ�װ��Ӧ��  ������Ӧ����

    FUN_TYPE_MULTI_START_VM        = 50,    //�ڶ࿪����������������
    FUN_TYPE_MULTI_CLOSE_VM        = 51,    //�ڶ࿪�������йرվ���
    FUN_TYPE_MULTI_DELET_VM        = 52,    //�ڶ࿪��������ɾ������
    FUN_TYPE_MULTI_CREAT_VM        = 53,    //�ڶ࿪�������д�������
    FUN_TYPE_MULTI_CLONE_VM        = 54,    //�ڶ࿪�������п�¡����
    FUN_TYPE_MULTI_CLONEALL_VM     = 55,    //�ڶ࿪�������йر����о���
    FUN_TYPE_MULTI_LAUNCH          = 56,    //�����࿪������
    FUN_TYPE_MULTI_CLOSE           = 57,    //�رն࿪������
    FUN_TYPE_MULTI_MINIMIZE        = 58,    //��С���࿪������
    FUN_TYPE_MULTI_CREAT_FAILED    = 59,    //�½�ģ����ʧ��
    FUN_TYPE_MULTI_CLONE_FAILED    = 60,    //��¡ģ����ʧ��

    FUN_TYPE_MYAPP_DOWNLOAD_START   =344,        //��ʼ����Ӧ�ã����������͸���
    FUN_TYPE_MYAPP_DOWNLOAD_SUCCESS = 340,      //Ӧ�����سɹ�
    FUN_TYPE_MYAPP_START_INS        = 341,      //Ӧ�ÿ�ʼ��װ
    FUN_TYPE_MYAPP_START_INS_SUCCESS  = 342,      //Ӧ�ÿ�ʼ��װ
    FUN_TYPE_MYAPP_INTALL_FAIL       =347,        //Ӧ�ð�װʧ��ԭ��

    //////////////////////////////////////////////////////////////////////////�����ع���(900~999)
    FUN_TYPE_PLUGIN_INSTALL                    = 900,          //��װ(������ģ������
    FUN_TYPE_PLUGIN_START                      = 901,          //����(������ģ������
    FUN_TYPE_PLUGIN_UNINSTALL                  = 902,          //ж��

    FUN_TYPE_PLUGIN_EDITCFG_COMBOSETTING       = 903,        //����-���̲���-��������

    FUN_TYPE_PLUGIN_DOWNLOAD                   = 904,          //����(������ģ������
    FUN_TYPE_PLUGIN_UPDATE                     = 905,          //����(������ģ������

    ///////////
    FUN_TYPE_MYAPP_RUN_APP        = 1000,        //��ʼ����Ӧ��  ���������
    FUN_TYPE_MYAPP_RUN_APP_FAIL        = 1001,   //����Ӧ��ʧ��  ���������

    FUN_TYPE_MYAPP_UNINS_APP_SUCCESS    = 1002,   //ж��Ӧ�óɹ�  ���������
    FUN_TYPE_MYAPP_UNINS_APP_FAIL    = 1003,       //ж��Ӧ��ʧ��  ���������
    FUN_TYPE_MYAPP_UNINS_APP         = 1004,       //��ʼж��Ӧ��

    FUN_TYPE_FIXER_TIMES             = 1100,       //�����쳣�޸��������
    FUN_TYPE_FIXER_MULTI_TIMES       = 1101,       //ͬһ�û���������쳣�������
    FUN_TYPE_FIXER_FIXNOW            = 1102,       //�����޸�����
    FUN_TYPE_FIXER_CONNECT_SERVER_QQ = 1103,       //�ҿͷ���������
    FUN_TYPE_FIXER_CHANGE_KPZS       = 1104,       //����ģ��������
    FUN_TYPE_FIXER_360VT             = 1105,       //360VTδ��ͬʱ�޷���ȡIP
    FUN_TYPE_FIXER_MEMLOW_START      = 1106,       //����ǰ�����ڴ治���޷�����
    FUN_TYPE_FIXER_MEMLOW_RUN        = 1107,       //�����п����ڴ治���޷�����
    FUN_TYPE_FIXER_MEMLOW_START_EXIT = 1108,       //����ǰ�ڴ治�㣬����˳���ť
    FUN_TYPE_FIXER_MEMLOW_RUN_EXIT   = 1109,       //�������ڴ治�㣬����˳���ť
    FUN_TYPE_KA99_BUT_SUCCESS        = 1110,       //��99�������ɹ�����
    FUN_TYPE_KA99_BUT_NOT_SHOW_FIXER = 1111,       //�������ߵ�99(80s~150s��)�û��ر�ģ����
    FUN_TYPE_SET_DX_ENABLE           = 1112,       //���ý���ѡ��DXģʽ������Ч�Ĵ���
    FUN_TYPE_FUNBAR_HIDEN            = 1113,       //��������ذ�ť�������
    FUN_TYPE_FUNBAR_INSTALLBTN       = 1114,       //���ذ�װ��ť�������
    FUN_TYPE_SET_DX_ENABLE_WHENSTART = 1115,       //����ǰѡ��DXģʽ������Ч�Ĵ���
    FUN_TYPE_SET_GL_ENABLE_WHENSTART = 1116,       //����ǰѡ��GLģʽ������Ч�Ĵ���
    FUN_TYPE_SET_GL_ENABLE           = 1117,       //���ý���ѡ��GLģʽ������Ч�Ĵ���
    FUN_TYPE_VT_NO_OPEN              = 1118,       //ÿ����ʾVTδ������ʾicon��MAC��
    FUN_TYPE_VT_BTN_CLICK            = 1119,       //VT��ť�ĵ����\MAC��
    FUN_TYPE_DETAILED_STRATEGY_CLICK = 1120,       //��ϸ���԰�ť�����\MAC��
    FUN_TYPE_CONTACT_CUSTOM_SERVICE  = 1121,       //��ϵ�ͷ��ĵ����\MAC��
    FUN_TYPE_HAND_SHAKEBTN           = 1122,        //ҡһҡ��ť�������
    FUN_TYPE_BAIDU_BROWSER           = 1123,         //�ٶ��������������
    FUN_TYPE_PLUGINTOOLS_OPEN        = 1124,        //������������
    FUN_TYPE_PLUGIN_FEIWO            = 1125,       //�������ְ�ť�������
    FUN_TYPE_FEIWOTIPS_CHECKED       = 1126,       //���ѵ���ѡ���´β�����ʾ
    FUN_TYPE_FEIWOTIPS_CLOSE         = 1127,       //���ѵ���رհ�ť�������
    FUN_TYPE_FEIWOTIPS_STARTINSTLL   = 1128,       //���ѵ�����һ�����ô���
    FUN_TYPE_FEIWOTIPS_SHOWDETAIL    = 1129,       //���ѵ������鿴�������
    FUN_TYPE_LOADING_BTNCLICK        = 1130,       //loadingͼ�������
    FUN_TYPE_SHOW_RECOMMENDDLG       = 1131,       //�Ƽ�apk������ʾ����\Mac��
    FUN_TYPE_RECDLG_INSTALL_BTNCLICK = 1132,       //һ����װ��ť���������\Mac��
    FUN_TYPE_RECDLG_CONTINUE_BTNCLICK= 1133,       //��Ȼ������ť���������
    FUN_TYPE_NOSHOW_RECOMMENDDLG     = 1134,       //������ʾ����ѡ������Ч�Ĵ���\Mac��
    FUN_TYPE_BACKUP_SUCCESS          = 1135,       //�����ɹ�
    FUN_TYPE_RECOVERY_SUCCESS        = 1136,       //����ɹ�
    FUN_TYPE_BACKUP_FAIL             = 1137,       //����ʧ��
    FUN_TYPE_RECOVERY_FAIL           = 1138,       //����ʧ��
    FUN_TYPE_BACKUP_TOTAL            = 1139,       //�����ܴ���
    FUN_TYPE_RECOVERY_TOTAL          = 1140,       //�����ܴ���
    FUN_TYPE_ADVERTDLG_SHOW          = 1141,       //��浯�����ִ���
    FUN_TYPE_ADVERTDLG_VIEW          = 1142        //�鿴���ݴ���

    // �����ڲ�ͳ������[2049,4098]

};