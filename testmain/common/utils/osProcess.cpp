
#include "osProcess.h"
#include <windows.h>
#include <string>
#include <stdlib.h>
#include <psapi.h>
#include <tchar.h>

#define OSPROCESS_MAX_BUFFER         (1*1024*1024) /* 1M pipe bufer */

OSProcess::OSProcess(void)
{
    m_hRead = NULL;
    m_hWrite= NULL;

    ZeroMemory((void*)&m_proc, sizeof(m_proc));
}
OSProcess::~OSProcess(void)
{
    if (m_proc.dwThreadId)
    {
        CloseHandle(m_proc.hThread);
    }

    if (m_proc.hProcess)
    {
        CloseHandle(m_proc.hProcess);
    }

    if (m_hRead)
    {
        CloseHandle(m_hRead);
    }
}

bool OSProcess::create(const wchar_t *p_cmdLine, const wchar_t *p_startdir, bool bIsHide)
{
    SECURITY_ATTRIBUTES sa; 
    sa.nLength = sizeof(SECURITY_ATTRIBUTES); 
    sa.lpSecurityDescriptor = NULL; 
    sa.bInheritHandle = TRUE; 
    if (!CreatePipe(&m_hRead, &m_hWrite, &sa, OSPROCESS_MAX_BUFFER))
        return NULL;

    STARTUPINFO        si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    GetStartupInfo(&si);
    si.wShowWindow = bIsHide ? SW_HIDE : SW_SHOW;
    si.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.hStdError   = m_hWrite;
    si.hStdOutput  = m_hWrite;

    BOOL ret = CreateProcess(
        NULL,
        (LPWSTR)p_cmdLine,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        (p_startdir != NULL ? p_startdir : _T(".\\")),
        &si,
        &m_proc);

    CloseHandle(m_hWrite);
    m_hWrite = NULL;
    if (ret == 0)
    {
        CloseHandle(m_hRead);
        m_hRead = NULL;
        return false;
    }

    // close the thread handle we do not need it,
    // keep the process handle for wait/trywait operations, will
    // be closed on destruction
    //CloseHandle(m_proc.hThread);

    return true;
}

bool OSProcess::wait(int *exitStatus)
{
    DWORD _exitStatus;

    if (WaitForSingleObject(m_proc.hProcess, INFINITE) == WAIT_FAILED)
    {
        return false;
    }

    if (!GetExitCodeProcess(m_proc.hProcess, &_exitStatus))
    {
        return false;
    }

    if (exitStatus)
        *exitStatus = _exitStatus;

    return true;
}


DWORD OSProcess::wait(int *exitStatus,int timeOut)
{
	DWORD _exitStatus;

	DWORD dw = WaitForSingleObject(m_proc.hProcess, timeOut);

	if (dw == WAIT_FAILED)
	{
		return -1;
	}
	else if(dw == WAIT_TIMEOUT)
	{
		return WAIT_TIMEOUT;
	}
	if (!GetExitCodeProcess(m_proc.hProcess, &_exitStatus))
	{
		return -1;
	}

	if (exitStatus)
		*exitStatus = _exitStatus;

	return 0;
}

int OSProcess::tryWait(bool& isAlive)
{
    DWORD status = WaitForSingleObject(m_proc.hProcess, 0);

    if(status == WAIT_OBJECT_0)
    {
        // process has exited
        isAlive = false;
        GetExitCodeProcess(m_proc.hProcess, &status);
    }
    else if (status == WAIT_TIMEOUT)
    {
        isAlive = true;
        status = 0;
    }

    return status;

}

bool OSProcess::readAllOutput(std::wstring& strOutput)
{
    if (m_hRead == NULL)
        return false;

    DWORD   bytesRead     = 0;
    DWORD   bytesReadToal = 0;
    char    *pRead        = new char[OSPROCESS_MAX_BUFFER];
    wchar_t *cResult      = new wchar_t[OSPROCESS_MAX_BUFFER];
    DWORD maxRead         = OSPROCESS_MAX_BUFFER;

    memset(pRead,0,OSPROCESS_MAX_BUFFER);
    memset(cResult,0,OSPROCESS_MAX_BUFFER*2);

    while (bytesReadToal<maxRead) 
    {
        if (!ReadFile(m_hRead, pRead+bytesReadToal, min(512,maxRead-bytesReadToal), &bytesRead,NULL) || bytesRead<512)
        {
            break;
        }

        bytesReadToal += bytesRead;
    }

    int len = MultiByteToWideChar(CP_ACP,0,pRead,strlen(pRead),NULL,0); 
    MultiByteToWideChar(CP_ACP,0,pRead,strlen(pRead),cResult,len);
    cResult[len]='\0';

    strOutput = cResult;

    delete pRead;
    delete cResult;

    CloseHandle(m_hRead);
    m_hRead = NULL;

    return true;
}

bool OSProcess::isRunning(void)
{
    return isRunning(m_proc.dwProcessId);
}

int  OSProcess::getPID(void)
{
    return m_proc.dwProcessId;
}

int OSProcess::getCurrentPID(void)
{
    return GetCurrentProcessId();
}

long OSProcess::getCurrentTID(void)
{
    return GetCurrentThreadId();
}

bool OSProcess::getName(wchar_t *p_outName, int p_outNameLen)
{
    return 0 != GetModuleFileNameEx( GetCurrentProcess(), NULL, p_outName, p_outNameLen);
}

int OSProcess::killProcess(int pid, bool wait)
{
    DWORD exitStatus = 1;
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    if (NULL == hProc) {
        return 0;
    }

    //
    // Terminate the process
    //
    TerminateProcess(hProc, 0x55);

    if (wait)
    {
        //
        // Wait for it to be terminated
        //
        if(WaitForSingleObject(hProc, INFINITE) == WAIT_FAILED)
        {
            CloseHandle(hProc);
            return 0;
        }

        if (!GetExitCodeProcess(hProc, &exitStatus))
        {
            CloseHandle(hProc);
            return 0;
        }
    }

    CloseHandle(hProc);

    return exitStatus;
}

bool OSProcess::isRunning(int pid)
{
    bool isRunning = false;

    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if (NULL != process)
    {
        DWORD ret = WaitForSingleObject(process, 0);
        CloseHandle(process);
        isRunning = (ret == WAIT_TIMEOUT);
    }
    return isRunning;
}

void OSProcess::sleep(int seconds)
{
    ::Sleep(seconds*1000);
}

