
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
    NF_TOPAPP,          /*Ӧ������ͨ��*/
    NF_WNDPOS,
    NF_STARTPROGRESS,
    NF_ERRMSG,
    NF_SCRIPT_ENABLE,
    NF_SIZE,
    NF_FULLSCREEN,
    NF_FORCES,
    NF_IME_ENABLE,
    NF_STARTED,
    NF_FENGWO,           /*Ӧ�ÿ�ʼ��װͨ��*/
    NF_STAERVMSUCCESS,
    NF_SHOWFPS,
	NF_APP_START,       /*Ӧ������ͨ��*/
    NF_APP_CLOSED,       /*Ӧ�ùر�ͨ��*/
    NF_APP_INSTALLED,    /*Ӧ�ð�װ�ɹ�ͨ��*/
    NF_APP_UNINSTALLED,  /*Ӧ��ж�سɹ�ͨ��*/
    NF_TEXTURE_CHANGE,   /*��Ⱦ�������仯*/
    NF_NEED_RESTART,   /*��Ҫ����*/
    NF_CHECK_STATUS,   /*�������״̬*/
	NF_VT_STATUS,
	NF_EMU_SHUTDOWN,
    NF_MAX,
};

typedef void (*tt_notifyExtFunc)(NF_TYPE nftype,WPARAM wParam,LPARAM lParam);
