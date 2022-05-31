// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>

#include "mhook-lib/mhook.h"
#include "ihook-lib/funchook.h"

typedef int (*func_AndroVM_FrameBuffer_setupSubWindow)(void* id, void* subid, int x, int y, int w, int h, float zrot);
func_AndroVM_FrameBuffer_setupSubWindow funcDef = nullptr;

int AndroVM_FrameBuffer_setupSubWindow(void* id, void* subid, int x, int y, int w, int h, float zrot)
{
    OutputDebugStringA("My Hook In\n");

    //if (funcDef)
    //    funcDef(id,subid,x,y,w,h,zrot);

    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
    )
{
    static struct func_hook sthook;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls(hModule);


            if (GetModuleHandle(L"VBoxHeadless.exe") != nullptr)
            {
                BOOL    bInjectSucc = TRUE;
                HMODULE hLibModule  = GetModuleHandle(L"libOpenglRender.dll");
                if (hLibModule)
                {/*find hook address*/ 
                    funcDef = (func_AndroVM_FrameBuffer_setupSubWindow)GetProcAddress(hLibModule,"AndroVM_FrameBuffer_setupSubWindow");

                    if (funcDef==nullptr) {
                        OutputDebugStringA("Find Address failed\n");
                        return false;
                    }

                    char szmsg[256] = { 0 };
                    sprintf(szmsg,"####0x%llx,0x%llx,0x%llx",(unsigned long long)hLibModule,(unsigned long long)funcDef,(unsigned long long)funcDef-(unsigned long long)hLibModule);

                    OutputDebugStringA(szmsg);
                    
#if 0
                    if (!Mhook_SetHook((PVOID*)&funcDef,AndroVM_FrameBuffer_setupSubWindow))
                    {
                        OutputDebugStringA("Mhook_SetHook Failed\n");
                        return false;
                    }
#else
                    hook_init(&sthook,funcDef,AndroVM_FrameBuffer_setupSubWindow,"AndroVM_FrameBuffer_setupSubWindow");

                    rehook(&sthook);

#endif

                }
            }
            break;
        }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
#if 0
        Mhook_Unhook((PVOID*)&funcDef);
        break;
#else
        unhook(&sthook);
#endif
    }
    return TRUE;
}
