//-----------------------------------------------------------------------------
//     Author : hiyohiyo
//       Mail : hiyohiyo@crystalmark.info
//        Web : http://openlibsys.org/
//    License : The modified BSD license
//
//                     Copyright 2007-2010 OpenLibSys.org. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

//-----------------------------------------------------------------------------
//
// DLL Status Code
//
//-----------------------------------------------------------------------------

#define OLS_DLL_NO_ERROR						0
#define OLS_DLL_UNSUPPORTED_PLATFORM			1
#define OLS_DLL_DRIVER_NOT_LOADED				2
#define OLS_DLL_DRIVER_NOT_FOUND				3
#define OLS_DLL_DRIVER_UNLOADED					4
#define OLS_DLL_DRIVER_NOT_LOADED_ON_NETWORK	5
#define OLS_DLL_UNKNOWN_ERROR					9

//-----------------------------------------------------------------------------
//
// Driver Type
//
//-----------------------------------------------------------------------------

#define OLS_DRIVER_TYPE_UNKNOWN			0
#define OLS_DRIVER_TYPE_WIN_9X			1
#define OLS_DRIVER_TYPE_WIN_NT			2
#define OLS_DRIVER_TYPE_WIN_NT4			3	// Obsolete
#define OLS_DRIVER_TYPE_WIN_NT_X64		4
#define OLS_DRIVER_TYPE_WIN_NT_IA64		5	// Reseved
