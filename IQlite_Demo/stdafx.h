// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers




#ifdef WIN32
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#define __getch _getch
#define __kbhit _kbhit


#else
#include "lp_stdlib.h"
#include "lp_stdio.h"
#include "lp_conio.h"
#include "lp_tchar.h"
#endif


//#include <stdio.h>
//#include <stdlib.h>
//#include <conio.h>
//#include <tchar.h>


// Step 1: Register the WiFi Test functions with TestManager
//         So that all WiFi Test functions will be made available to Test Manager
void run_RegisterTechnologyDll(void);

// Step 2: Setup the global settings
//         Global settings are used by all test functions
//         They do not belong to any specific test function
void run_GLOBAL_SETTINGS(void);

// Step 3: Load path loss table from file
void run_LOAD_PATH_LOSS_TABLE(void);

// Step 4: Connect IQTester (IP address needed)
void run_CONNECT_IQ_TESTER(_TCHAR* ipAddress);

// Step 5: Insert DUT for test
void run_INSERT_DUT(void);

// Step 6: Initial DUT for test
void run_INITIALIZE_DUT(void);

// Step 7: TX Verify EVM Test     
void run_TX_VERIFY_EVM(void);

// Step 8: TX Verify Power Test     
void run_TX_VERIFY_POWER(void);

// Step 9: TX Verify Mask Test     
void run_TX_VERIFY_MASK(void);

// Step 10: TX Verify Spectrum Test     
void run_TX_VERIFY_SPECTRUM(void);

// Step 11: RX Verify PER Test   
void run_RX_VERIFY_PER(void);   

// Step 12: Remove DUT
void run_REMOVE_DUT(void);

// Step 13: Disconnect IQTester
void run_DISCONNECT_IQ_TESTER(void); 


