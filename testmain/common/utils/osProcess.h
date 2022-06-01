
#pragma once

#include <Windows.h>

#include <string>

class OSProcess
{
public:
    OSProcess(void);
    ~OSProcess(void);

public:
    bool create(const wchar_t *p_cmdLine, const wchar_t *p_startdir, bool bIsHide);

    int  tryWait(bool& isAlive);
    bool wait(int *exitStatus);
	DWORD wait(int *exitStatus,int timeOut);
    bool isRunning(void);
    int  getPID(void);

    bool readAllOutput(std::wstring& strOutput);

    static bool isRunning(int pid);
    static int  getCurrentPID(void);
    static long getCurrentTID(void);
    static bool getName(wchar_t *p_outName, int p_outNameLen);
    static int  killProcess(int pid, bool wait);
    static void sleep(int seconds);

private:
    PROCESS_INFORMATION m_proc;

    HANDLE  m_hRead;
    HANDLE  m_hWrite;
};
