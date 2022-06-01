
#include "unittest.h"
#include "InjectProc.h"

//injectdll 19500 E:/Work/unittest/x64/Release/testhook.dll
UNITTEST(injectdll)
{
    if (argc < 2)
    {
        printf("usage: injectdll pid dll\n");
        return;
    }

    DWORD dwPid        = atoi(argv[0]);
    const char *dllPath = argv[1];

    DWORD dwStatus=CheckDllHasInjected(dwPid,dllPath);
    if (dwStatus==1)
    {
        printf("InjectDllToExe ָ���������Ľ��̲�δ����");
        return;
    }
    else if (dwStatus==2)
    {
        BOOL bOk=MyInjectProcess(dwPid,dllPath);
        if (!bOk)
        {
            printf("InjectDllToExe MyInjectProcess Failed!");
            return ;
        }
        return;
    }
    else if (dwStatus==3)
    {
        printf("InjectDllToExe �Ѿ�ע���");
        return;
    }
    return;

}

UNITTEST(uninjectdll)
{

}

