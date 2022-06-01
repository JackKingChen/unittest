
#include "common/utils/util.h"

#include "asm/arch/x86/ollyasm/disasm.h"

#include <stdio.h>

#include <string>
#include <vector>

#include <Windows.h>

bool Asm2Machinecode(const char* pAsmcode, unsigned long dwAddrress, t_asmmodel* pMachinecode)
{
    bool bResult;
    char errtext[TEXTLEN];
    Assemble((char*)pAsmcode,dwAddrress,pMachinecode,0,0,errtext);
    Assemble((char*)pAsmcode,dwAddrress,pMachinecode,0,0,errtext);
    bResult = pMachinecode->length > 0;
    return bResult;
}

bool AsmInject(int pid, const std::string& asmcode)
{
    if (pid == 0)
        return false;

    std::vector<std::string> vasmcode;

    stringSplit(asmcode,'\n',vasmcode);

    int    nMaxCmdSize = vasmcode.size() * MAXCMDSIZE;//FIXME
    HANDLE hProcess    = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    PVOID  pLibRemote  = ::VirtualAllocEx(hProcess, NULL, nMaxCmdSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    DWORD  dwStartAddr = (DWORD)(PDWORD)pLibRemote;
    t_asmmodel am;
    memset(&am, 0, sizeof(am));

    for (int i=0; i<vasmcode.size(); i++)
    {
        if (vasmcode[i].length()>1)
        {
            if (Asm2Machinecode(vasmcode[i].c_str(), dwStartAddr, &am))
            {
                ::WriteProcessMemory(hProcess, (void*)dwStartAddr, (void*)am.code, am.length, NULL);
                dwStartAddr += am.length;
            }
            else
            {
                ::VirtualFreeEx(hProcess, pLibRemote, nMaxCmdSize, MEM_RELEASE);
                CloseHandle(hProcess);

                fprintf(stderr,"Asm2Machinecode failed:%s\n",vasmcode[i].c_str());
                return false;
            }
        }

    }

    Asm2Machinecode("xor eax,eax", dwStartAddr, &am);
    ::WriteProcessMemory(hProcess, (void*)dwStartAddr, (void*)am.code, am.length, NULL);
    dwStartAddr += am.length;
    Asm2Machinecode("retn 0", dwStartAddr, &am);
    ::WriteProcessMemory(hProcess, (void*)dwStartAddr, (void*)am.code, am.length, NULL);

    HANDLE hThread;
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLibRemote, NULL, 0, 0);
    printf("%08X\n",pLibRemote);
    CloseHandle(hThread);


    ::VirtualFreeEx(hProcess, pLibRemote, nMaxCmdSize, MEM_RELEASE);
    CloseHandle(hProcess);

    return true;
}

