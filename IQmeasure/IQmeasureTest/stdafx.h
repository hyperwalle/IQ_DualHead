// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifdef WIN32
#include <windows.h>
#include "Console.h"
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
enum CONSOLE_COLOR {CM_BLACK, CM_BLUE, CM_GREEN, CM_CYAN, CM_RED, CM_MEGNETA, CM_YELLOW, CM_WHITE, CM_GREY, CM_LIGHT_BLUE, CM_LIGHT_GREEN, CM_LIGHT_CYAN, CM_LIGHT_RED, CM_LIGHT_MAGENTA, CM_LIGHT_YELLOW, CM_LIGHT_WHITE};

#else
#include "lp_windows.h"
#include "lp_consoleMode.h"
#endif

#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <sys/timeb.h>
#include <time.h>
#include "IQlite_Logger.h"
#include <direct.h>

//#include "lp_windows.h"
//#include "lp_consoleMode.h"

// TODO: reference additional headers your program requires here
