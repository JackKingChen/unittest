
#include "InjectProc.h"

#include <Windows.h>
#include <stdio.h>

//查找指定进程中的指定模块
VOID WINAPI FindModuleByModuleShortName(DWORD dwProcessId, LPCTSTR szName, PMODULEENTRY32 psModEntry)
{
	HANDLE hSnapshot = NULL;

	__try
	{
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);
		if(hSnapshot == INVALID_HANDLE_VALUE) __leave;

		MODULEENTRY32 me = {sizeof(me)};

		if(!Module32First(hSnapshot, &me)) __leave;

		while(Module32Next(hSnapshot, &me))
		{
			PTCHAR Pos = strchr(me.szExePath, '\\');
			if(Pos)
			{	
				if(!stricmp(Pos + 1, szName))
				{
					memcpy(psModEntry,&me,sizeof(me));
					break;
				}
			}
		}

	}__finally{

		if(hSnapshot){
			CloseHandle(hSnapshot);
		}
	}
}

//通过进程名获取进程ID
DWORD WINAPI GetPidByName(LPCTSTR szName)
{
	HANDLE hProcessSnap = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 pe32 = {0};
	DWORD dwPid=0;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return 0;

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First (hProcessSnap, &pe32)) {
		do {
			if (stricmp(szName, pe32.szExeFile) == 0) {
				dwPid=pe32.th32ProcessID;
				break;
			}
		}while (Process32Next(hProcessSnap,&pe32));
	}
	else {
		return 0;
	}

	if(hProcessSnap !=INVALID_HANDLE_VALUE) {
		CloseHandle(hProcessSnap);
	}

	return dwPid;
}

//检测DLL是否已经被注入
DWORD WINAPI CheckDllHasInjected(DWORD dwPid,LPCTSTR dllName)
{
	if (dllName==NULL || lstrlen(dllName)<=0)
	{
		return 0;
	}
	if (dwPid==0)
	{
		return 1;
	}
	MODULEENTRY32 modEntry = {0};
	FindModuleByModuleShortName(dwPid, dllName, &modEntry);
	//	HMODULE hMonoDll = GetModuleHandle("mono.dll");
	//DebugPrint("GetMyProfession modEntry.modBaseAddr = %p\n", modEntry.modBaseAddr);

	if(!modEntry.modBaseAddr)
		return 2;

	return 3;
}

//注入指定dll到指定进程
BOOL WINAPI CreateThreadForInjectDll(HANDLE hProcess,LPCTSTR dllPath)
{
	BOOL bRet=FALSE;
	do 
	{
		LPVOID lpszRemoteFile = NULL;
		HANDLE hThread = NULL;
		DWORD_PTR dwSize;
		dwSize=sizeof(TCHAR)*(lstrlen(dllPath)+1);
		lpszRemoteFile = VirtualAllocEx(hProcess, NULL, 
			dwSize, MEM_COMMIT, PAGE_READWRITE);
		if (lpszRemoteFile == NULL)
		{
			printf("CreateThreadForInjectDll VirtualAllocEx Failed");
			break;
		}
		DWORD_PTR dwWritten;
		if (!WriteProcessMemory(hProcess, lpszRemoteFile, 
			(PVOID)dllPath, dwSize, &dwWritten) || dwSize!=dwWritten)
		{
			printf("CreateThreadForInjectDll WriteProcessMemory Failed GetLastError=%d,dwSize=%d,dwWritten=%d",GetLastError(),dwSize,dwWritten);
			goto end;
		}
#ifdef UNICODE
		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress( 
			GetModuleHandleW(L"Kernel32.dll"),"LoadLibraryW"); 
#else
		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress( 
			GetModuleHandleA("Kernel32.dll"),"LoadLibraryA"); 
#endif
		hThread = CreateRemoteThread(hProcess,NULL, 0, pfnThreadRtn,lpszRemoteFile,0,NULL);
		if(hThread==NULL)
		{
			printf("CreateThreadForInjectDll CreateRemoteThread Failed");
			goto end;
		}
		// 等待线程返回 
		WaitForSingleObject(hThread, INFINITE); 
		bRet=TRUE;

end:
		// 释放进程空间中的内存
		if(lpszRemoteFile!=NULL)
		{
			VirtualFreeEx(hProcess, lpszRemoteFile, 0, MEM_RELEASE);
		}
		if(hThread!=NULL)
		{
			CloseHandle(hThread);
		}
	} while (FALSE);
	return bRet;
}

//注入指定dll到指定进程
BOOL WINAPI MyInjectProcess(DWORD dwPid,LPCTSTR lpDllPath)
{
	if (lpDllPath==NULL || lstrlen(lpDllPath)<=0)
	{
		return FALSE;
	}

	if (dwPid==0)
	{
		return FALSE;
	}
	HANDLE hProcess = OpenProcess(STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0xFFF , FALSE, dwPid);
	if (!hProcess)
	{
		printf("MyInjectProcess OpenProcess Failed ProcessId=%X,GetLastError=%d",dwPid,GetLastError());
		return FALSE;
	}

	return CreateThreadForInjectDll(hProcess,lpDllPath);
}

