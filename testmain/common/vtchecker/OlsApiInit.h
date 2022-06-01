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

#include "OlsDef.h"
#include "OlsApiInitDef.h"

//-----------------------------------------------------------------------------
//
// Prototypes
//
//-----------------------------------------------------------------------------

BOOL InitOpenLibSys(HMODULE *hModule);
BOOL DeinitOpenLibSys(HMODULE *hModule);

//-----------------------------------------------------------------------------
//
// Funtions
//
//-----------------------------------------------------------------------------

// DLL
_GetDllStatus GetDllStatus = NULL;
_GetDllVersion GetDllVersion = NULL;
_GetDriverVersion GetDriverVersion = NULL;
_GetDriverType GetDriverType = NULL;

_InitializeOls InitializeOls = NULL;
_DeinitializeOls DeinitializeOls = NULL;

// CPU
_IsCpuid IsCpuid = NULL;
_IsMsr IsMsr = NULL;
_IsTsc IsTsc = NULL;

_Rdmsr Rdmsr = NULL;
_Cpuid Cpuid = NULL;
_Rdtsc Rdtsc = NULL;

_RdmsrTx RdmsrTx = NULL;
_CpuidTx CpuidTx = NULL;
_RdtscTx RdtscTx = NULL;

_RdmsrPx RdmsrPx = NULL;
_CpuidPx CpuidPx = NULL;
_RdtscPx RdtscPx = NULL;

//-----------------------------------------------------------------------------
//
// Initialize
//
//-----------------------------------------------------------------------------

BOOL InitOpenLibSys(HMODULE *hModule)
{
    *hModule = LoadLibrary(L"WinRing0.dll");

	if(*hModule == NULL)
	{
		return FALSE;
	}

	//-----------------------------------------------------------------------------
	// GetProcAddress
	//-----------------------------------------------------------------------------
	// DLL
	GetDllStatus =			(_GetDllStatus)			GetProcAddress (*hModule, "GetDllStatus");
	GetDllVersion =			(_GetDllVersion)		GetProcAddress (*hModule, "GetDllVersion");
	GetDriverVersion =		(_GetDriverVersion)		GetProcAddress (*hModule, "GetDriverVersion");
	GetDriverType =			(_GetDriverType)		GetProcAddress (*hModule, "GetDriverType");

	InitializeOls =			(_InitializeOls)		GetProcAddress (*hModule, "InitializeOls");
	DeinitializeOls =		(_DeinitializeOls)		GetProcAddress (*hModule, "DeinitializeOls");

	// CPU
	IsCpuid =				(_IsCpuid)				GetProcAddress (*hModule, "IsCpuid");
	IsMsr =					(_IsMsr)				GetProcAddress (*hModule, "IsMsr");
	IsTsc =					(_IsTsc)				GetProcAddress (*hModule, "IsTsc");
	Rdmsr =					(_Rdmsr)				GetProcAddress (*hModule, "Rdmsr");
	Cpuid =					(_Cpuid)				GetProcAddress (*hModule, "Cpuid");
	Rdtsc =					(_Rdtsc)				GetProcAddress (*hModule, "Rdtsc");
	RdmsrTx =				(_RdmsrTx)				GetProcAddress (*hModule, "RdmsrTx");
	CpuidTx =				(_CpuidTx)				GetProcAddress (*hModule, "CpuidTx");
	RdtscTx =				(_RdtscTx)				GetProcAddress (*hModule, "RdtscTx");
	RdmsrPx =				(_RdmsrPx)				GetProcAddress (*hModule, "RdmsrPx");
	CpuidPx =				(_CpuidPx)				GetProcAddress (*hModule, "CpuidPx");
	RdtscPx =				(_RdtscPx)				GetProcAddress (*hModule, "RdtscPx");

	//-----------------------------------------------------------------------------
	// Check Functions
	//-----------------------------------------------------------------------------
	if(!(
		GetDllStatus
	&&	GetDllVersion
	&&	GetDriverVersion
	&&	GetDriverType
	&&	InitializeOls
	&&	DeinitializeOls
	&&	IsCpuid
	&&	IsMsr
	&&	IsTsc
	&&	Rdmsr
	&&	RdmsrTx
	&&	RdmsrPx
	&&	Cpuid
	&&	CpuidTx
	&&	CpuidPx
	&&	Rdtsc
	&&	RdtscTx
	&&	RdtscPx
	))
	{
		return FALSE;
	}

	return InitializeOls();
}

//-----------------------------------------------------------------------------
//
// Deinitialize
//
//-----------------------------------------------------------------------------

BOOL DeinitOpenLibSys(HMODULE *hModule)
{
	BOOL result = FALSE;

	if(*hModule == NULL)
	{
		return TRUE;
	}
	else
	{
		DeinitializeOls();
		result = FreeLibrary(*hModule);
		*hModule = NULL;

		return result;
	}
}
