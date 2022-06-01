
#pragma once;

#include <list>

typedef void (*tt_notifyFunc)(WPARAM wParam,LPARAM lParam);
typedef void (*tt_notifyFunc2)(int nEmuID,WPARAM wParam,LPARAM lParam);

typedef std::list<tt_notifyFunc2> tt_notifyList;

#define TS_SHOWED        0
#define TS_HIDE          1
#define TS_STARTING      2
#define TS_STOPED        3
#define TS_READY         4

enum NF_TYPE
{
    NF_STATUS = 0,
    NF_TOPAPP,          /*应用启动通告*/
    NF_WNDPOS,
    NF_STARTPROGRESS,
    NF_ERRMSG,
    NF_SCRIPT_ENABLE,
    NF_SIZE,
    NF_FULLSCREEN,
    NF_FORCES,
    NF_IME_ENABLE,
    NF_STARTED,
    NF_FENGWO,           /*应用开始安装通告*/
    NF_STAERVMSUCCESS,
    NF_SHOWFPS,
	NF_APP_START,       /*应用启动通告*/
    NF_APP_CLOSED,       /*应用关闭通告*/
    NF_APP_INSTALLED,    /*应用安装成功通告*/
    NF_APP_UNINSTALLED,  /*应用卸载成功通告*/
    NF_TEXTURE_CHANGE,   /*渲染纹理发生变化*/
    NF_NEED_RESTART,   /*需要重启*/
    NF_CHECK_STATUS,   /*检查启动状态*/
	NF_VT_STATUS,
	NF_EMU_SHUTDOWN,
    NF_MAX,
};

typedef void (*tt_notifyExtFunc)(NF_TYPE nftype,WPARAM wParam,LPARAM lParam);
