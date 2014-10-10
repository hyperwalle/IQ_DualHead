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
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <time.h>
#include "shlwapi.h"
using namespace std;

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
// stdafx.h : include file for standard system include files,
#define BUFFER_SIZE 512

//#define __this__version__ "FoxLite_13.04.16"
//#define __this__version__ "FoxLite_13.07.10"
// this version change
// 1. Fix the bug 'can not test mask'
// 2. Support IQFlex / IQxel type 
#define __this__version__ "FoxLite_13.07.27"
// this version change
// 1. delete log_all.txt before test run to make sure data is accordance.
#define __this__author__   "dai-xin.gao@foxconn.com"
/*
Version Histroy

*/

typedef struct 
{
	double dPathLoss1;
	double dPathLoss2;
	double dPathLoss3;
	double dPathLoss4;
}PATH_LOSS;

typedef struct 
{
	// new format for 11ac.
	int BSS_FREQ_MHZ_PRIMARY;
	int BSS_FREQ_MHZ_SECONDARY;
	int CH_FREQ_MHZ;
	int CH_FREQ_MHZ_PRIMARY_20MHz;
	int NUM_STREAM_11AC;
	int nTx1;
	int nTx2;
	int nTx3;
	int nTx4;
	double CABLE_LOSS_DB_1;
	double CABLE_LOSS_DB_2;
	double CABLE_LOSS_DB_3;
	double CABLE_LOSS_DB_4;
	double SAMPLING_TIME_US;
	//double TX_POWER_DBM;
	char  BSS_BANDWIDTH[12];
	char  CH_BANDWIDTH[12];
	char  DATA_RATE[12];
	char  GUARD_INTERVAL[12];
	char  PACKET_FORMAT[12];
	char  PREAMBLE[12];
	char  STANDARD[12];

	char szSubItem[24];
	double dTargetPower;
	double dEvmSpec;
	PATH_LOSS g_PathLoss;
}TX_PARAM_IN;

typedef struct 
{
	double dPowerUpper2g;
	double dPowerLower2g;
	double dPowerUpper5g;
	double dPowerLower5g;
	double dMaskPercent;
	double dPerPercent;
	double dPerPercentOFDM;
	double dFreqencyShift;
	double dEvmLower;
}TS_WIFI_LIMIT;

typedef struct 
{
	char szSubItem[24];
	char BSS_BANDWIDTH[12];
	char CH_BANDWIDTH[12];
	char DATA_RATE[8];
	char PACKET_FORMAT[12];
	char PREAMBLE[8];
	char STANDARD[12];
	int  CH_FREQ_MHZ;
	int  BSS_FREQ_MHZ_PRIMARY;
	int  BSS_FREQ_MHZ_SECONDARY;
	int  CH_FREQ_MHZ_PRIMARY_20MHz;
	int  NUM_STREAM_11AC;
	int  FRAME_COUNT;
	double dRxPowerLevel;
	int nRx1;
	int nRx2;
	int nRx3;
	int nRx4;
	PATH_LOSS g_PathLoss;
}RX_PARAM_IN;


typedef pair<string, TX_PARAM_IN> Tx_Pair;
typedef pair<string, RX_PARAM_IN> Rx_Pair;
typedef vector<string> STRING_TESTITEM_VECTOR;
typedef struct 
{
	char szSubItem[24];
	double EVM_AVG_ALL;
	double EVM_AVG_1;
	double EVM_AVG_2;
	double EVM_AVG_3;
	double EVM_AVG_4;
	double POWER_AVG_ALL;
	double POWER_AVG_1;
	double POWER_AVG_2;
	double POWER_AVG_3;
	double POWER_AVG_4;
	double FREQ_ERROR;
	double MASK_PERCENT;
}TX_PARAM_RETURN;

typedef struct 
{
	double dPerPercent;
	double dRxPowerLevel;
}RX_PARAM_RETURN;
typedef struct
{
	int RunMode;	// = 0
	int RepeatTimes; //= 1
	int ExitWhenDone; //= 0
	int ShowFailInfo; //= 0
	int RetryMode;
	int nRetryTimes;
	int ReloadDutDLL;

	int IQ_TESTER_RECONNECT;
	int IQTESTER_TYPE;
	TCHAR szDUT_DLL_FILENAME[BUFFER_SIZE];
	TCHAR szConnectionString[BUFFER_SIZE];	
	TCHAR szTxPathLossFile[BUFFER_SIZE];
	TCHAR szRxPathLossFile[BUFFER_SIZE];
	TCHAR IQ_TESTER_IP1[BUFFER_SIZE];
	TCHAR IQ_TESTER_IP2[BUFFER_SIZE];
	TCHAR IQ_TESTER_IP3[BUFFER_SIZE];
	TCHAR IQ_TESTER_IP4[BUFFER_SIZE];
	TCHAR PACKET_FORMAT[BUFFER_SIZE];

	// add by daixin to support dual test, 2012-12-10
	int DH_ENABLE; //= 0
	int DH_OBTAIN_CONTROL_TIMEOUT_MS;
	int DH_PROBE_TIME_MS;
	int DH_TOKEN_ID;
	// add end

}TS_TEST_CTRL;

typedef struct 
{
	// new format for 11ac.
	int ANALYSIS_11AC_AMPLITUDE_TRACKING;
	int ANALYSIS_11AC_DECODE_PSDU;
	int ANALYSIS_11AC_FREQUENCY_CORRELATION;
	int ANALYSIS_11AC_FULL_PACKET_CHANNEL_EST;
	int ANALYSIS_11AC_MASK_ACCORDING_CBW;
	int ANALYSIS_11AC_PHASE_CORR;
	int ANALYSIS_11AC_SYM_TIMING_CORR;
	int ANALYSIS_11AG_AMPL_TRACK;
	int ANALYSIS_11AG_CH_ESTIMATE;
	int ANALYSIS_11AG_FREQ_SYNC;
	int ANALYSIS_11AG_PH_CORR_MODE;
	int ANALYSIS_11AG_SYM_TIM_CORR;
	int ANALYSIS_11B_DC_REMOVE_FLAG;
	int ANALYSIS_11B_EQ_TAPS;
	int ANALYSIS_11B_FIXED_01_DATA_SEQUENCE;
	int ANALYSIS_11B_METHOD_11B;
	int ANALYSIS_11N_AMPLITUDE_TRACKING;
	int ANALYSIS_11N_DECODE_PSDU;
	int ANALYSIS_11N_FREQUENCY_CORRELATION;
	int ANALYSIS_11N_FULL_PACKET_CHANNEL_EST;
	int ANALYSIS_11N_PHASE_CORR;
	int ANALYSIS_11N_SYM_TIMING_CORR;
	int AUTO_READING_LIMIT;
	int DUT_KEEP_TRANSMIT;
	int DUT_RX_SETTLE_TIME_MS;
	int DUT_TX_SETTLE_TIME_MS;
	int EVM_AVERAGE;
	int EVM_CAPTURE_TIME_11AC_HT_GF_US;
	int EVM_CAPTURE_TIME_11AC_HT_MF_US;
	int EVM_CAPTURE_TIME_11AC_NON_HT_US;
	int EVM_CAPTURE_TIME_11AC_VHT_US;
	int EVM_CAPTURE_TIME_11AG_US;
	int EVM_CAPTURE_TIME_11B_L_US;
	int EVM_CAPTURE_TIME_11B_S_US;
	int EVM_CAPTURE_TIME_11N_GREENFIELD_US;
	int EVM_CAPTURE_TIME_11N_MIXED_US;
	int EVM_PRE_TRIG_TIME_US;
	int EVM_SYMBOL_NUM;
	int IQ_P_TO_A_11AG_12M;
	int IQ_P_TO_A_11AG_18M;
	int IQ_P_TO_A_11AG_24M;
	int IQ_P_TO_A_11AG_36M;
	int IQ_P_TO_A_11AG_48M;
	int IQ_P_TO_A_11AG_54M;
	int IQ_P_TO_A_11AG_6M;
	int IQ_P_TO_A_11AG_9M;
	int IQ_P_TO_A_11B_11M;
	int IQ_P_TO_A_11B_1M;
	int IQ_P_TO_A_11B_2M;
	int IQ_P_TO_A_11B_5_5M;
	int IQ_P_TO_A_11N_MCS0;
	int IQ_P_TO_A_11N_MCS1;
	int IQ_P_TO_A_11N_MCS2;
	int IQ_P_TO_A_11N_MCS3;
	int IQ_P_TO_A_11N_MCS4;
	int IQ_P_TO_A_11N_MCS5;
	int IQ_P_TO_A_11N_MCS6;
	int IQ_P_TO_A_11N_MCS7;
	int MASK_DSSS_SAMPLE_INTERVAL_US;
	int MASK_OFDM_SAMPLE_INTERVAL_US;
	int PER_FRAME_COUNT_11AC;
	int PER_FRAME_COUNT_11AG;
	int PER_FRAME_COUNT_11B;
	int PER_FRAME_COUNT_11N;
	int PER_VSG_TIMEOUT_SEC;
	int PM_AVERAGE;
	int PM_DSSS_SAMPLE_INTERVAL_US;
	int PM_IF_FREQ_SHIFT_MHZ;
	int PM_OFDM_SAMPLE_INTERVAL_US;
	int RESET_TEST_ITEM_DURING_RETRY;
	int RETRY_ERROR_ITEMS;
	int RETRY_TEST_ITEM;
	int SPECTRUM_DSSS_SAMPLE_INTERVAL_US;
	int SPECTRUM_OFDM_SAMPLE_INTERVAL_US;
	int VSA_PORT;
	int VSA_SAVE_CAPTURE_ALWAYS;
	int VSA_SAVE_CAPTURE_ON_FAILED;
	int VSA_TRIGGER_TYPE;
	int VSG_PORT;
	double VSA_AMPLITUDE_TOLERANCE_DB;
	double VSA_PRE_TRIGGER_TIME_US;
	double VSA_TRIGGER_LEVEL_DB;
	double VSG_MAX_POWER_11AC;
	double VSG_MAX_POWER_11B;
	double VSG_MAX_POWER_11G;
	double VSG_MAX_POWER_11N;
	TCHAR PER_WAVEFORM_DESTINATION_MAC [BUFFER_SIZE];
	TCHAR PER_WAVEFORM_PATH [BUFFER_SIZE];
	TCHAR PER_WAVEFORM_PREFIX_11AC [BUFFER_SIZE];
	TCHAR PER_WAVEFORM_PREFIX_11AG [BUFFER_SIZE];
	TCHAR PER_WAVEFORM_PREFIX_11B [BUFFER_SIZE];
	TCHAR PER_WAVEFORM_PREFIX_11N [BUFFER_SIZE];


	TS_TEST_CTRL TestCtrl;
	TS_WIFI_LIMIT WifiLimit;
}TS_Global_Setting;

enum err_prepare
{
	err_RegisterTechnologyDll=90,
	err_GLOBAL_SETTINGS,
	err_LOAD_PATH_LOSS_TABLE,
	err_CONNECT_IQ_TESTER,
	err_INSERT_DUT,
	err_INITIALIZE_DUT
};
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
void run_TX_VERIFY_EVM(TX_PARAM_IN *txParam, TX_PARAM_RETURN *txReturn);

// Step 8: TX Verify Power Test     
void run_TX_VERIFY_POWER(TX_PARAM_IN *txParam, TX_PARAM_RETURN *txReturn);

// Step 9: TX Verify Mask Test     
void run_TX_VERIFY_MASK(TX_PARAM_IN *txParam, TX_PARAM_RETURN *txReturn);

// Step 10: TX Verify Spectrum Test     
void run_TX_VERIFY_SPECTRUM(void);

// Step 11: RX Verify PER Test   
void run_RX_VERIFY_PER(RX_PARAM_IN *rxParam, RX_PARAM_RETURN *rxReturn);   

// Step 12: Remove DUT
void run_REMOVE_DUT(void);

// Step 13: Disconnect IQTester
void run_DISCONNECT_IQ_TESTER(void); 
#if 0
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


#endif