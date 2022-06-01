//-----------------------------------------------------------------------------
//     Author : hiyohiyo
//       Mail : hiyohiyo@crystalmark.info
//        Web : http://openlibsys.org/
//    License : The modified BSD license
//
//                     Copyright 2007-2010 OpenLibSys.org. All rights reserved.
//-----------------------------------------------------------------------------
// for WinRing0 2.0.x

#pragma once

//-----------------------------------------------------------------------------
//
// Type Defines
//
//-----------------------------------------------------------------------------

// DLL
typedef DWORD (WINAPI *_GetDllStatus) ();
typedef DWORD (WINAPI *_GetDllVersion) (PBYTE major, PBYTE minor, PBYTE revision, PBYTE release);
typedef DWORD (WINAPI *_GetDriverVersion) (PBYTE major, PBYTE minor, PBYTE revision, PBYTE release);
typedef DWORD (WINAPI *_GetDriverType) ();

typedef BOOL (WINAPI *_InitializeOls) ();
typedef VOID (WINAPI *_DeinitializeOls) ();

// CPU
typedef BOOL (WINAPI *_IsCpuid) ();
typedef BOOL (WINAPI *_IsMsr) ();
typedef BOOL (WINAPI *_IsTsc) ();

typedef DWORD (WINAPI *_Rdmsr) (DWORD index, PDWORD eax, PDWORD edx);
typedef DWORD (WINAPI *_Cpuid) (DWORD indexEax, DWORD indexEcx, PDWORD eax, PDWORD ebx, PDWORD ecx, PDWORD edx);
typedef DWORD (WINAPI *_Rdtsc) (PDWORD eax, PDWORD edx);

typedef DWORD (WINAPI *_RdmsrTx) (DWORD index, PDWORD eax, PDWORD edx, DWORD_PTR threadAffinityMask);
typedef DWORD (WINAPI *_CpuidTx) (DWORD indexEax, DWORD indexEcx, PDWORD eax, PDWORD ebx, PDWORD ecx, PDWORD edx, DWORD_PTR threadAffinityMask);
typedef DWORD (WINAPI *_RdtscTx) (PDWORD eax, PDWORD edx, DWORD_PTR threadAffinityMask);

typedef DWORD (WINAPI *_RdmsrPx) (DWORD index, PDWORD eax, PDWORD edx, DWORD_PTR processAffinityMask);
typedef DWORD (WINAPI *_CpuidPx) (DWORD indexEax, DWORD indexEcx, PDWORD eax, PDWORD ebx, PDWORD ecx, PDWORD edx, DWORD_PTR processAffinityMask);
typedef DWORD (WINAPI *_RdtscPx) (PDWORD eax, PDWORD edx, DWORD_PTR processAffinityMask);
