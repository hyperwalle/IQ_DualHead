// stdafx.h : include file for standard system include files,
// or project-specific include files that are used frequently and
// changed infrequently
//

#pragma once

// Modify the following "defines" if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest information on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows ME or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers



#ifdef WIN32
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <string.h>
#include <stdarg.h>
#include <sys/timeb.h>
#include <time.h>

#define __getch _getch
#define __kbhit _kbhit
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#else
#include "lp_stdlib.h"
#include "lp_stdio.h"
#include "lp_conio.h"
#include "lp_windows.h"
#include "lp_string.h" // strcpy_s
#include "lp_time.h"
#endif

// Windows Header Files:
//#include "lp_stdlib.h"
//#include "lp_stdio.h"
//#include "lp_conio.h"
//#include "lp_windows.h"
#include <string>
#include <map>
#include <vector>
#include <limits>
//#include "lp_string.h" // strcpy_s
//#include <atltime.h>


// TODO: reference additional headers your program requires here

namespace highrestimer
{

#ifdef WIN32
	typedef struct __timeb64 lp_time_t;
#else
	typedef uint64_t lp_time_t;
#endif


	// one time call per app run.  Okay if called multiple times.
	bool HighResTimerInit(); 

	// Get a start or stop time.  
	void GetTime(lp_time_t& time);
	
	// Get a duration between two GetTime() calls in seconds
	double GetElapsedSeconds(lp_time_t start, lp_time_t end);

	// Get a duration between two GetTime() calls in milli-seconds
	unsigned int GetElapsedMSec(lp_time_t start, lp_time_t end);
}


#ifdef WIN32
inline bool operator ==(const highrestimer::lp_time_t& a, const highrestimer::lp_time_t& b) { return a.time == b.time; }
inline bool operator <=(const highrestimer::lp_time_t& a, const highrestimer::lp_time_t& b) { return a.time <= b.time; }
inline bool operator >=(const highrestimer::lp_time_t& a, const highrestimer::lp_time_t& b) { return a.time >= b.time; }
#endif