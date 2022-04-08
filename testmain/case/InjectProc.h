#pragma once

#include <Windows.h>
#include <TlHelp32.h>

DWORD WINAPI CheckDllHasInjected(DWORD dwPid,LPCTSTR dllName);
BOOL WINAPI CreateThreadForInjectDll(HANDLE hProcess,LPCTSTR wDllPath);
BOOL WINAPI MyInjectProcess(DWORD dwPid,LPCTSTR lpDllPath);
DWORD WINAPI GetPidByName(LPCTSTR szName);
VOID WINAPI FindModuleByModuleShortName(DWORD dwProcessId, LPCTSTR szName, PMODULEENTRY32 psModEntry);