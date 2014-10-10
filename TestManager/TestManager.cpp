// TestManager.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
//Move to stdafx.h
//#include "lp_windows.h"
//#include "lp_loadLibrary.h"
#include <stdlib.h>
#include <string>
#include <vector>
#include <limits>
#include <map>
#include "TestManager.h"
#include "StringUtil.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"
//Move to stdafx.h
//#include "lp_loadlibrary.h"
//#include "lp_string.h"
//#include "lp_stdio.h"
#include "Version.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

// TM stands for TestManager

using namespace std;
//using namespace stdext;

//! Define the prototype for callback functions
/*!
 * All test functions take no parameter, and return either zero (no errors), or non-zero (errors occurred)
 */
typedef struct TM_CallBack
{
    int     (*pointerToFunction) (void);
} TM_CALLBACK;


typedef struct TM_Description
{
    TM_ID                   technologyID;    // =-1 before a wireless test is registered; =tmFunctionIndex after a wireless test is register
    enum tagTechnology      tmFunctionIndex; // Index to access to g_testFunctions
    string                  tmName;
    string                  tmVersion;
} TM_INFO;

//const char  *g_TestManager_Version = "1.6.0 (2010-09-17)\n";
const char  *g_TestManager_Version = TESTMANAGER_VERSION; // version defined in import\Version.h 

typedef int (*technologyRegister) (void);
typedef int (*technologyFunction) (void);

map <string, TM_CALLBACK > g_testFunctions[MAX_TECHNOLOGY_NUM];
map <string, TM_CALLBACK>::iterator g_function_Iter[MAX_TECHNOLOGY_NUM];
typedef pair<string , TM_CALLBACK> functionPair;

map <string, TM_INFO> g_technologies;
map <string, TM_INFO>::iterator g_technology_Iter;
typedef pair<string , TM_INFO> technologyPair;

map <string, int> g_intParams[MAX_TECHNOLOGY_NUM];
typedef pair<string , int> intParamPair;
map <string, int>::iterator g_intParams_Iter[MAX_TECHNOLOGY_NUM];

map <string, double> g_doubleParams[MAX_TECHNOLOGY_NUM];
typedef pair<string , double> doubleParamPair;
map <string, double>::iterator g_doubleParam_Iter[MAX_TECHNOLOGY_NUM];

map <string, string> g_stringParams[MAX_TECHNOLOGY_NUM];
typedef pair<string , string> stringParamPair;
map <string, string>::iterator g_stringParam_Iter[MAX_TECHNOLOGY_NUM];

map <string, int> g_intReturns[MAX_TECHNOLOGY_NUM];
typedef pair<string , int> intReturnPair;
// The iterator needs to be declared globally for TM_GetIntegerReturnPair
map <string, int>::iterator g_intReturn_Iter[MAX_TECHNOLOGY_NUM];

map <string, double> g_doubleReturns[MAX_TECHNOLOGY_NUM];
typedef pair<string , double> doubleReturnPair;
// The iterator needs to be declared globally for TM_GetDoubleReturnPair
map <string, double>::iterator g_doubleReturn_Iter[MAX_TECHNOLOGY_NUM];

map <string, string> g_stringReturns[MAX_TECHNOLOGY_NUM];
typedef pair<string , string> stringReturnPair;
// The iterator needs to be declared globally for TM_GetStringReturnPair
map <string, string>::iterator g_stringReturn_Iter[MAX_TECHNOLOGY_NUM];

map <string, vector<double> > g_arrayDoubleReturns[MAX_TECHNOLOGY_NUM];
typedef pair<string , vector<double> > arrayDoubleReturnPair;
map <string, vector<double> >::iterator g_arrayDoubleReturn_Iter[MAX_TECHNOLOGY_NUM];

// map<dataRate, measureMaps>, measureMap<string, resultMap>, resultMap<string, doubleResult>
//Sequence result structures
typedef struct tagSeqReturnValue
{
    vector <double>  values;
    string  unit;
} SEQ_PARAM_RETURN_VALUES;

//map<paramName, SEQ_RETURN_VALUES> SEQ_MEAS_RESULTS
typedef map<string, SEQ_PARAM_RETURN_VALUES> SEQ_MEAS_PARAM_RETURN;
typedef pair<string, SEQ_PARAM_RETURN_VALUES> seqMeasParamReturnPair;
//map<measType, SEQ_MEAS_RESULTS> SEQ_MEAS_TYPE_RESULTS
typedef map<int, SEQ_MEAS_PARAM_RETURN> SEQ_MEAS_TYPE_RESULTS;
typedef pair<int, SEQ_MEAS_PARAM_RETURN> seqMeasTypeResultsPair;
//map<dataRate, SEQ_MEAS_TYPE_RESULTS> SEQ_DATA_RATE_RESULTS;
map<string, SEQ_MEAS_TYPE_RESULTS> g_seqDataRateResults[MAX_TECHNOLOGY_NUM];

// itemUnits works as companion container with three return value containers
// to indicate the unit of items in each return value container
map <string, string> g_itemUnits[MAX_TECHNOLOGY_NUM];
map <string, string>::iterator g_itemUnit_Iter[MAX_TECHNOLOGY_NUM];

// helpText works as companion container with three return value containers
// to the help text for in each return value container
map <string, string> g_helpText[MAX_TECHNOLOGY_NUM];
map <string, string>::iterator g_helpText_Iter[MAX_TECHNOLOGY_NUM];

map <int, int>       wifiChannels;
map <string, int>    wifiDataRates;
map <string, double> wifiDataRatesMbps;
map <string, int>	 dataRateIQ2010Ext;

// Global Multi-Segment Waveform map
map <string, int> g_multiWaveformIndexMap;

HMODULE g_technologyDllHandle[MAX_TECHNOLOGY_NUM];

TM_ID	g_tmTimerID [MAX_TECHNOLOGY_NUM];
TM_ID	g_tmLoggerID[MAX_TECHNOLOGY_NUM];

// Tx and Rx Path Loss Table, Tx is [TECHNOLOGY_NUM][0], Rx is [TECHNOLOGY_NUM][1].
map <int, vector<double> > g_pathLossMap[MAX_TECHNOLOGY_NUM][MAX_PATHLOSS_TABLE_NUM];


typedef struct
{
    std::string sSerialNumber;
    std::string sManufacturer; 
    std::string sModel;
    std::string sRevision;
    std::string sDescription;
} DEVICE_INFO;
DEVICE_INFO g_dutInfo;

int Initialize_TM();
void Free_TM_Memory();

void ProcessExtensions(TM_ID technologyID, TM_STR technologyDllFileName);
void LogTestInputParameters(TM_ID technologyID, const TM_STR functionKeyword);
void LogTestResults(TM_ID technologyID, const TM_STR functionKeyword);

                                           
typedef struct tagPosition
{
    string::size_type startPos;
    string::size_type stopPos;
} POSITION;

//static int init_tm = Initialize_TM();	// initialization at dll load
BOOL APIENTRY DllMain( HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                      )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		Initialize_TM();
		break;
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		Free_TM_Memory();
		break;
    }
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

TM_API char* __stdcall TM_GetErrorString(TM_RETURN errorNum)
{
    char *ret="";

    switch(errorNum)
    {
    case TM_ERR_OK: ret="No errors";
    case TM_ERR_TECHNOLOGY_DOES_NOT_EXIST: 
        ret="Specified technology has not been defined yet";
        break;
    case TM_ERR_TECHNOLOGY_HAS_BEEN_REGISTERED: 
        ret="The specified technology Another has been registgered by another DUT";
        break;
    case TM_ERR_FUNCTION_NOT_SUPPORTED:       
        ret="DUT does not support the specified function";
        break;
    case TM_ERR_FUNCTION_NOT_DEFINED:
        ret="The specified function has not been defined yet";
        break;
    case TM_ERR_PARAM_DOES_NOT_EXIST:
        ret="The specified parameter does not exist";
        break;
    case TM_ERR_INVALID_TECHNOLOGY_ID:
        ret="Invalid DUT ID";
        break;
    case TM_ERR_FUNCTION_ERROR:
        ret = "DUT function had errors";
        break;
	case TM_ERR_WIFI_FREQ_DOES_NOT_EXIST:
		ret = "The specified WiFi frequency does not exist";
		break;
    }

    return ret;
}

void Free_TM_Memory()
{
	g_technologies.clear();
	wifiChannels.clear();
	wifiDataRates.clear();
	dataRateIQ2010Ext.clear();
	g_multiWaveformIndexMap.clear();

	for (int i=0;i<MAX_TECHNOLOGY_NUM;i++)
	{
		g_testFunctions[i].clear();
		g_intParams[i].clear();
		g_doubleParams[i].clear();
		g_stringParams[i].clear();
		g_intReturns[i].clear();
		g_doubleReturns[i].clear();
		g_stringReturns[i].clear();
		g_arrayDoubleReturns[i].clear();
		g_itemUnits[i].clear();
		g_helpText[i].clear();

		for (int j=0;j<MAX_PATHLOSS_TABLE_NUM;j++)
		{
			g_pathLossMap[i][j].clear();
		}
	}
}

/*! @defgroup wifi_test_function_keywords Function Keywords for WiFi Test
 *
 * The following function keywords have been defined for WiFi Test:
 *      - CONNECT_IQ_TESTER
 *          - Connect to LitePoint wireless tester, such as IQview/IQflex, IQmax, and IQ2010.  This function should be 
 *          executed at the very beginning of a test program, but not included in any iterations.
 *      - DISCONNECT_IQ_TESTER
 *          - Disconnect from LitePoint wireless tester.  This function should be executed at the very end of a test program, 
 *          before exiting.
 *      - INSERT_DUT       
 *          - Insert a Device Under Test (DUT).  This function will register a specific DUT implementation with the 
 *          Virtual DUT (vDUT), and all DUT functions will be managed by vDUT.  This function should be executed at the 
 *          beginning of a test iteration.
 *      - REMOVE_DUT        
 *          - Remove a DUT.  This function should be executed as the last step of a test iteration.
 *      - TX_CALIBRATION    
 *      - TX_VERIFY_EVM     
 *      - TX_VERIFY_POWER   
 *      - TX_VERIFY_MASK    
 *      - RX_RSSI_CAL       
 *      - RX_VERIFY_PER     
 *      - RX_VERIFY_PER_MAX 
 *      - RX_SWEEP_PER      
 *      - READ_EEPROM       
 *      - WRITE_EEPROM
 *      - TX_CURRENT
 *      - RX_CURRENT
 */

/*! @defgroup wimax_test_function_keywords Function Keywords for WiMAX Test
 *
 * Yet to be defined
 */

/*! @defgroup bt_test_function_keywords Function Keywords for Bluetooth Test
 *
 * Yet to be defined
 */

/*! @defgroup gps_test_function_keywords Function Keywords for GPS Test
 *
 * Yet to be defined
 */

/*! @defgroup fm_test_function_keywords Function Keywords for FM Test
 *
 * Yet to be defined
 */

int Initialize_TM()
{
    TM_CALLBACK callBack;

    for(int i=0; i<MAX_TECHNOLOGY_NUM; i++)
    {
        g_testFunctions[i].clear();
		g_tmTimerID[i]	=  -1;
		g_tmLoggerID[i]	=  -1;

		// Initialize path loss table
		for (int j=0;j<MAX_PATHLOSS_TABLE_NUM;j++)
		{
			g_pathLossMap[i][j].clear();
		}
    }

    callBack.pointerToFunction = NULL;

    g_technologies.clear();
    TM_INFO tmInfo;
    tmInfo.technologyID    = -1;
    tmInfo.tmName          = "";
    tmInfo.tmVersion       = "";

	//------------//
    //    WIFI    //
	//------------//
    tmInfo.tmFunctionIndex   = WIFI;
    g_technologies.insert( technologyPair("WIFI", tmInfo) );
    g_testFunctions[WIFI].clear();
    g_testFunctions[WIFI].insert( functionPair("GLOBAL_SETTINGS",      callBack) );

    // Tester and DUT control functions
    g_testFunctions[WIFI].insert( functionPair("CONNECT_IQ_TESTER",    callBack) );
    g_testFunctions[WIFI].insert( functionPair("DISCONNECT_IQ_TESTER", callBack) );    
    g_testFunctions[WIFI].insert( functionPair("INSERT_DUT",           callBack) );
    g_testFunctions[WIFI].insert( functionPair("INITIALIZE_DUT",       callBack) );
    g_testFunctions[WIFI].insert( functionPair("REMOVE_DUT",           callBack) );
	g_testFunctions[WIFI].insert( functionPair("RUN_DUT_COMMAND",      callBack) );
    g_testFunctions[WIFI].insert( functionPair("RESET_DUT",            callBack) );
    g_testFunctions[WIFI].insert( functionPair("POWER_MODE_DUT",       callBack) );

    // Calibration functions
    g_testFunctions[WIFI].insert( functionPair("TX_CALIBRATION",    callBack) );
    g_testFunctions[WIFI].insert( functionPair("RX_CALIBRATION",    callBack) );
    g_testFunctions[WIFI].insert( functionPair("XTAL_CALIBRATION",  callBack) );

    // Verification functions
    g_testFunctions[WIFI].insert( functionPair("TX_MULTI_VERIFICATION", callBack) );
    g_testFunctions[WIFI].insert( functionPair("TX_VERIFY_EVM",     callBack) );
    g_testFunctions[WIFI].insert( functionPair("TX_VERIFY_POWER",   callBack) );
    g_testFunctions[WIFI].insert( functionPair("TX_VERIFY_MASK",    callBack) );
    g_testFunctions[WIFI].insert( functionPair("TX_VERIFY_SPECTRUM",callBack) );
    g_testFunctions[WIFI].insert( functionPair("TX_RAMP_TIMING",    callBack) );
    g_testFunctions[WIFI].insert( functionPair("RX_VERIFY_PER",     callBack) );
    g_testFunctions[WIFI].insert( functionPair("RX_SWEEP_PER",      callBack) );
    g_testFunctions[WIFI].insert( functionPair("APPEND_EEPROM",     callBack) );
    g_testFunctions[WIFI].insert( functionPair("FINALIZE_EEPROM",   callBack) );
    g_testFunctions[WIFI].insert( functionPair("VERIFY_EEPROM",     callBack) );
    g_testFunctions[WIFI].insert( functionPair("CLEAR_EEPROM",      callBack) );

    g_testFunctions[WIFI].insert( functionPair("GET_SERIAL_NUMBER", callBack) );
	g_testFunctions[WIFI].insert( functionPair("READ_SERIAL_NUMBER", callBack) );
	g_testFunctions[WIFI].insert( functionPair("WRITE_SERIAL_NUMBER", callBack) );

	// EEPROM handling (place holders)
	g_testFunctions[WIFI].insert( functionPair("READ_EEPROM",           callBack) );
	g_testFunctions[WIFI].insert( functionPair("WRITE_EEPROM",          callBack) );
	g_testFunctions[WIFI].insert( functionPair("FINALIZE_EEPROM",       callBack) );

    g_testFunctions[WIFI].insert( functionPair("READ_MAC_ADDRESS",		callBack) );
    g_testFunctions[WIFI].insert( functionPair("WRITE_MAC_ADDRESS",		callBack) );

	g_testFunctions[WIFI].insert( functionPair("WRITE_BB_REGISTER",     callBack) );
	g_testFunctions[WIFI].insert( functionPair("READ_BB_REGISTER",      callBack) );

	g_testFunctions[WIFI].insert( functionPair("WRITE_RF_REGISTER",     callBack) );
	g_testFunctions[WIFI].insert( functionPair("READ_RF_REGISTER",      callBack) );

	g_testFunctions[WIFI].insert( functionPair("WRITE_MAC_REGISTER",    callBack) );
	g_testFunctions[WIFI].insert( functionPair("READ_MAC_REGISTER",     callBack) );

	g_testFunctions[WIFI].insert( functionPair("WRITE_SOC_REGISTER",    callBack) );
	g_testFunctions[WIFI].insert( functionPair("READ_SOC_REGISTER",     callBack) );

    // Run External program
    g_testFunctions[WIFI].insert( functionPair("RUN_EXTERNAL_PROGRAM",	callBack) );
	g_testFunctions[WIFI].insert( functionPair("CURRENT_TEST",			callBack) );

	g_testFunctions[WIFI].insert( functionPair("LOAD_PATH_LOSS_TABLE",			callBack) );
	g_testFunctions[WIFI].insert( functionPair("LOAD_MULTI_SEGMENT_WAVEFORM",	callBack) );

	g_testFunctions[WIFI].insert( functionPair("VDUT_DISABLED",			callBack) );
	g_testFunctions[WIFI].insert( functionPair("VDUT_ENABLED",			callBack) );
    // Create the timer ID 
	TIMER_CreateTimer("WIFI_TM", &g_tmTimerID[WIFI]);
    // Create the logger ID 
	LOGGER_CreateLogger("WIFI_TM", &g_tmLoggerID[WIFI]);

	//-----------------//
    //    WIFI_MIMO    //
	//-----------------//
    tmInfo.tmFunctionIndex   = WIFI_MIMO;
    g_technologies.insert( technologyPair("WIFI_MIMO", tmInfo) );
    g_testFunctions[WIFI_MIMO].clear();
    g_testFunctions[WIFI_MIMO].insert( functionPair("GLOBAL_SETTINGS",      callBack) );

    // Tester and DUT control functions
    g_testFunctions[WIFI_MIMO].insert( functionPair("CONNECT_IQ_TESTER",    callBack) );
    g_testFunctions[WIFI_MIMO].insert( functionPair("DISCONNECT_IQ_TESTER", callBack) );    
    g_testFunctions[WIFI_MIMO].insert( functionPair("INSERT_DUT",           callBack) );
    g_testFunctions[WIFI_MIMO].insert( functionPair("INITIALIZE_DUT",       callBack) );
    g_testFunctions[WIFI_MIMO].insert( functionPair("REMOVE_DUT",           callBack) );
	g_testFunctions[WIFI_MIMO].insert( functionPair("RUN_DUT_COMMAND",      callBack) );
    g_testFunctions[WIFI_MIMO].insert( functionPair("RESET_DUT",            callBack) );
    g_testFunctions[WIFI_MIMO].insert( functionPair("POWER_MODE_DUT",       callBack) );

    // Calibration functions
    g_testFunctions[WIFI_MIMO].insert( functionPair("TX_CALIBRATION",    callBack) );
    g_testFunctions[WIFI_MIMO].insert( functionPair("RX_CALIBRATION",    callBack) );
    g_testFunctions[WIFI_MIMO].insert( functionPair("XTAL_CALIBRATION",  callBack) );

    // Verification functions
    g_testFunctions[WIFI_MIMO].insert( functionPair("TX_VERIFY_EVM",     callBack) );
    g_testFunctions[WIFI_MIMO].insert( functionPair("TX_VERIFY_POWER",   callBack) );
    g_testFunctions[WIFI_MIMO].insert( functionPair("TX_VERIFY_MASK",    callBack) );
    g_testFunctions[WIFI_MIMO].insert( functionPair("TX_VERIFY_SPECTRUM",callBack) );
    g_testFunctions[WIFI_MIMO].insert( functionPair("TX_RAMP_TIMING",    callBack) );
    g_testFunctions[WIFI_MIMO].insert( functionPair("RX_VERIFY_PER",     callBack) );
    //g_testFunctions[WIFI_MIMO].insert( functionPair("RX_SWEEP_PER",      callBack) );
    g_testFunctions[WIFI_MIMO].insert( functionPair("APPEND_EEPROM",     callBack) );
    g_testFunctions[WIFI_MIMO].insert( functionPair("FINALIZE_EEPROM",   callBack) );
    g_testFunctions[WIFI_MIMO].insert( functionPair("VERIFY_EEPROM",     callBack) );
    g_testFunctions[WIFI_MIMO].insert( functionPair("CLEAR_EEPROM",      callBack) );

    g_testFunctions[WIFI_MIMO].insert( functionPair("GET_SERIAL_NUMBER", callBack) );

	// EEPROM handling (place holders)
	g_testFunctions[WIFI_MIMO].insert( functionPair("READ_EEPROM",           callBack) );
	g_testFunctions[WIFI_MIMO].insert( functionPair("WRITE_EEPROM",          callBack) );

    g_testFunctions[WIFI_MIMO].insert( functionPair("READ_MAC_ADDRESS",  callBack) );
    g_testFunctions[WIFI_MIMO].insert( functionPair("WRITE_MAC_ADDRESS", callBack) );

	g_testFunctions[WIFI_MIMO].insert( functionPair("WRITE_BB_REGISTER",     callBack) );
	g_testFunctions[WIFI_MIMO].insert( functionPair("READ_BB_REGISTER",      callBack) );

	g_testFunctions[WIFI_MIMO].insert( functionPair("WRITE_RF_REGISTER",     callBack) );
	g_testFunctions[WIFI_MIMO].insert( functionPair("READ_RF_REGISTER",      callBack) );

	g_testFunctions[WIFI_MIMO].insert( functionPair("WRITE_MAC_REGISTER",    callBack) );
	g_testFunctions[WIFI_MIMO].insert( functionPair("READ_MAC_REGISTER",     callBack) );

	g_testFunctions[WIFI_MIMO].insert( functionPair("WRITE_SOC_REGISTER",    callBack) );
	g_testFunctions[WIFI_MIMO].insert( functionPair("READ_SOC_REGISTER",     callBack) );

    // Run External program
    g_testFunctions[WIFI_MIMO].insert( functionPair("RUN_EXTERNAL_PROGRAM",callBack) );
	g_testFunctions[WIFI_MIMO].insert( functionPair("CURRENT_TEST",callBack) );

	g_testFunctions[WIFI_MIMO].insert( functionPair("LOAD_PATH_LOSS_TABLE",  callBack) );

    // Create the timer ID 
	TIMER_CreateTimer("WIFI_MIMO_TM", &g_tmTimerID[WIFI_MIMO]);
    // Create the logger ID 
	LOGGER_CreateLogger("WIFI_MIMO_TM", &g_tmLoggerID[WIFI_MIMO]);

	//-----------------//
	//    WIFI_11AC    //
	//-----------------//
	tmInfo.tmFunctionIndex   = WIFI_11AC;
	g_technologies.insert( technologyPair("WIFI_11AC", tmInfo) );
	g_testFunctions[WIFI_11AC].clear();
	g_testFunctions[WIFI_11AC].insert( functionPair("GLOBAL_SETTINGS",      callBack) );

	// Tester and DUT control functions
	g_testFunctions[WIFI_11AC].insert( functionPair("CONNECT_IQ_TESTER",    callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("DISCONNECT_IQ_TESTER", callBack) );    
	g_testFunctions[WIFI_11AC].insert( functionPair("INSERT_DUT",           callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("INITIALIZE_DUT",       callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("REMOVE_DUT",           callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("RUN_DUT_COMMAND",      callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("RESET_DUT",            callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("POWER_MODE_DUT",       callBack) );

	// Calibration functions
	g_testFunctions[WIFI_11AC].insert( functionPair("TX_CALIBRATION",    callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("RX_CALIBRATION",    callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("XTAL_CALIBRATION",  callBack) );

	// Verification functions
	g_testFunctions[WIFI_11AC].insert( functionPair("TX_MULTI_VERIFICATION", callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("TX_VERIFY_EVM",     callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("TX_VERIFY_POWER",   callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("TX_VERIFY_MASK",    callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("TX_VERIFY_SPECTRUM",callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("TX_RAMP_TIMING",    callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("RX_VERIFY_PER",     callBack) );
	//g_testFunctions[WIFI_11AC].insert( functionPair("RX_SWEEP_PER",      callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("APPEND_EEPROM",     callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("FINALIZE_EEPROM",   callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("VERIFY_EEPROM",     callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("CLEAR_EEPROM",      callBack) );

	g_testFunctions[WIFI_11AC].insert( functionPair("GET_SERIAL_NUMBER", callBack) );

	// EEPROM handling (place holders)
	g_testFunctions[WIFI_11AC].insert( functionPair("READ_EEPROM",           callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("WRITE_EEPROM",          callBack) );

	g_testFunctions[WIFI_11AC].insert( functionPair("READ_MAC_ADDRESS",  callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("WRITE_MAC_ADDRESS", callBack) );

	g_testFunctions[WIFI_11AC].insert( functionPair("WRITE_BB_REGISTER",     callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("READ_BB_REGISTER",      callBack) );

	g_testFunctions[WIFI_11AC].insert( functionPair("WRITE_RF_REGISTER",     callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("READ_RF_REGISTER",      callBack) );

	g_testFunctions[WIFI_11AC].insert( functionPair("WRITE_MAC_REGISTER",    callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("READ_MAC_REGISTER",     callBack) );

	g_testFunctions[WIFI_11AC].insert( functionPair("WRITE_SOC_REGISTER",    callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("READ_SOC_REGISTER",     callBack) );

	// Run External program
	g_testFunctions[WIFI_11AC].insert( functionPair("RUN_EXTERNAL_PROGRAM",callBack) );
	g_testFunctions[WIFI_11AC].insert( functionPair("CURRENT_TEST",callBack) );

	g_testFunctions[WIFI_11AC].insert( functionPair("LOAD_PATH_LOSS_TABLE",  callBack) );

	// Create the timer ID 
	TIMER_CreateTimer("WIFI_11AC_TM", &g_tmTimerID[WIFI_11AC]);
	// Create the logger ID 
	LOGGER_CreateLogger("WIFI_11AC_TM", &g_tmLoggerID[WIFI_11AC]);

	//-----------------//
	// WIFI_11AC_MIMO  //
	//-----------------//
	tmInfo.tmFunctionIndex   = WIFI_11AC_MIMO;
	g_technologies.insert( technologyPair("WIFI_11AC_MIMO", tmInfo) );
	g_testFunctions[WIFI_11AC_MIMO].clear();
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("GLOBAL_SETTINGS",      callBack) );

	// Tester and DUT control functions
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("CONNECT_IQ_TESTER",    callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("DISCONNECT_IQ_TESTER", callBack) );    
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("INSERT_DUT",           callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("INITIALIZE_DUT",       callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("REMOVE_DUT",           callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("RUN_DUT_COMMAND",      callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("RESET_DUT",            callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("POWER_MODE_DUT",       callBack) );

	// Calibration functions
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("TX_CALIBRATION",    callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("RX_CALIBRATION",    callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("XTAL_CALIBRATION",  callBack) );

	// Verification functions
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("TX_VERIFY_EVM",     callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("TX_VERIFY_POWER",   callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("TX_VERIFY_MASK",    callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("TX_VERIFY_SPECTRUM",callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("TX_RAMP_TIMING",    callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("RX_VERIFY_PER",     callBack) );
	//g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("RX_SWEEP_PER",      callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("APPEND_EEPROM",     callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("FINALIZE_EEPROM",   callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("VERIFY_EEPROM",     callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("CLEAR_EEPROM",      callBack) );

	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("GET_SERIAL_NUMBER", callBack) );

	// EEPROM handling (place holders)
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("READ_EEPROM",           callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("WRITE_EEPROM",          callBack) );

	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("READ_MAC_ADDRESS",  callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("WRITE_MAC_ADDRESS", callBack) );

	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("WRITE_BB_REGISTER",     callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("READ_BB_REGISTER",      callBack) );

	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("WRITE_RF_REGISTER",     callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("READ_RF_REGISTER",      callBack) );

	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("WRITE_MAC_REGISTER",    callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("READ_MAC_REGISTER",     callBack) );

	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("WRITE_SOC_REGISTER",    callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("READ_SOC_REGISTER",     callBack) );

	// Run External program
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("RUN_EXTERNAL_PROGRAM",callBack) );
	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("CURRENT_TEST",callBack) );

	g_testFunctions[WIFI_11AC_MIMO].insert( functionPair("LOAD_PATH_LOSS_TABLE",  callBack) );

	// Create the timer ID 
	TIMER_CreateTimer("WIFI_11AC_MIMO_TM", &g_tmTimerID[WIFI_11AC_MIMO]);
	// Create the logger ID 
	LOGGER_CreateLogger("WIFI_11AC_MIMO_TM", &g_tmLoggerID[WIFI_11AC_MIMO]);

	//------------//
	// WIFI_MPS   //
	//------------//
	tmInfo.tmFunctionIndex   = WIFI_MPS;
	g_technologies.insert( technologyPair("WIFI_MPS", tmInfo) );
	g_testFunctions[WIFI_MPS].clear();
	g_testFunctions[WIFI_MPS].insert( functionPair("GLOBAL_SETTINGS",      callBack) );

	// Tester and DUT control functions
	g_testFunctions[WIFI_MPS].insert( functionPair("CONNECT_IQ_TESTER",    callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("DISCONNECT_IQ_TESTER", callBack) );    
	g_testFunctions[WIFI_MPS].insert( functionPair("INSERT_DUT",           callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("INITIALIZE_DUT",       callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("REMOVE_DUT",           callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("RESET_DUT",            callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("POWER_MODE_DUT",       callBack) );

	// Calibration functions
	g_testFunctions[WIFI_MPS].insert( functionPair("TX_CALIBRATION",    callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("RX_CALIBRATION",    callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("XTAL_CALIBRATION",  callBack) );

	// Verification functions
	g_testFunctions[WIFI_MPS].insert( functionPair("TX_VERIFY_EVM",     callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("TX_VERIFY_POWER",   callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("TX_VERIFY_MASK",    callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("TX_VERIFY_SPECTRUM",callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("TX_RAMP_TIMING",    callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("RX_VERIFY_PER",     callBack) );

    g_testFunctions[WIFI_MPS].insert( functionPair("TEST_PREPARE",      callBack) );
    g_testFunctions[WIFI_MPS].insert( functionPair("TEST_RUN",          callBack) );
    g_testFunctions[WIFI_MPS].insert( functionPair("TX_TEST_ADD",       callBack) );
    g_testFunctions[WIFI_MPS].insert( functionPair("RX_TEST_ADD",       callBack) );
    g_testFunctions[WIFI_MPS].insert( functionPair("RUN_DUT_COMMAND",   callBack) );

	//g_testFunctions[WIFI_MPS].insert( functionPair("RX_SWEEP_PER",      callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("APPEND_EEPROM",     callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("FINALIZE_EEPROM",   callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("VERIFY_EEPROM",     callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("CLEAR_EEPROM",      callBack) );

	g_testFunctions[WIFI_MPS].insert( functionPair("GET_SERIAL_NUMBER", callBack) );

	// EEPROM handling (place holders)
	g_testFunctions[WIFI_MPS].insert( functionPair("READ_EEPROM",           callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("WRITE_EEPROM",          callBack) );

	g_testFunctions[WIFI_MPS].insert( functionPair("READ_MAC_ADDRESS",		callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("WRITE_MAC_ADDRESS",		callBack) );

	g_testFunctions[WIFI_MPS].insert( functionPair("WRITE_BB_REGISTER",     callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("READ_BB_REGISTER",      callBack) );

	g_testFunctions[WIFI_MPS].insert( functionPair("WRITE_RF_REGISTER",     callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("READ_RF_REGISTER",      callBack) );

	g_testFunctions[WIFI_MPS].insert( functionPair("WRITE_MAC_REGISTER",    callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("READ_MAC_REGISTER",     callBack) );

	g_testFunctions[WIFI_MPS].insert( functionPair("WRITE_SOC_REGISTER",    callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("READ_SOC_REGISTER",     callBack) );

	// Run External program
	g_testFunctions[WIFI_MPS].insert( functionPair("RUN_EXTERNAL_PROGRAM",	callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("CURRENT_TEST",			callBack) );

	g_testFunctions[WIFI_MPS].insert( functionPair("LOAD_PATH_LOSS_TABLE",			callBack) );
	g_testFunctions[WIFI_MPS].insert( functionPair("LOAD_MULTI_SEGMENT_WAVEFORM",	callBack) );

	// Create the timer ID 
	TIMER_CreateTimer("WIFI_MPS_TM", &g_tmTimerID[WIFI_MPS]);
	// Create the logger ID 
	LOGGER_CreateLogger("WIFI_MPS_TM", &g_tmLoggerID[WIFI_MPS]);


	//------------//
    //     BT     //
	//------------//
    tmInfo.tmFunctionIndex   = BT;
    g_technologies.insert( technologyPair("BT", tmInfo) );
    g_testFunctions[BT].clear();
    g_testFunctions[BT].insert( functionPair("GLOBAL_SETTINGS",      callBack) );

    // Tester and DUT control functions
    g_testFunctions[BT].insert( functionPair("CONNECT_IQ_TESTER",    callBack) );
    g_testFunctions[BT].insert( functionPair("DISCONNECT_IQ_TESTER", callBack) );    
    g_testFunctions[BT].insert( functionPair("INSERT_DUT",           callBack) );
    g_testFunctions[BT].insert( functionPair("INITIALIZE_DUT",       callBack) );
    g_testFunctions[BT].insert( functionPair("REMOVE_DUT",           callBack) );
    g_testFunctions[BT].insert( functionPair("RESET_DUT",            callBack) );

    // Verification functions
	g_testFunctions[BT].insert( functionPair("TX_BDR",            callBack) );
	g_testFunctions[BT].insert( functionPair("TX_EDR",            callBack) );
 	g_testFunctions[BT].insert( functionPair("TX_LE",             callBack) );
	g_testFunctions[BT].insert( functionPair("RX_BDR",            callBack) );
    g_testFunctions[BT].insert( functionPair("RX_EDR",            callBack) );
    g_testFunctions[BT].insert( functionPair("RX_LE",             callBack) );
	g_testFunctions[BT].insert( functionPair("TX_POWER_CONTROL",  callBack) );
	g_testFunctions[BT].insert( functionPair("TX_CALIBRATION",    callBack) );

	g_testFunctions[BT].insert( functionPair("WRITE_BD_ADDRESS",  callBack) );
	g_testFunctions[BT].insert( functionPair("READ_BD_ADDRESS",   callBack) );
	g_testFunctions[BT].insert( functionPair("WRITE_DUT_INFO",    callBack) );

	g_testFunctions[BT].insert( functionPair("LOAD_PATH_LOSS_TABLE",  callBack) );
	g_testFunctions[BT].insert( functionPair("POWER_MODE_DUT",        callBack) );
	g_testFunctions[BT].insert( functionPair("RUN_EXTERNAL_PROGRAM",  callBack) );
	g_testFunctions[BT].insert( functionPair("BTLE_INITIALIZE",  callBack) );
    // Create the timer ID 
	TIMER_CreateTimer("BT_TM", &g_tmTimerID[BT]);
    // Create the logger ID 
	LOGGER_CreateLogger("BT_TM", &g_tmLoggerID[BT]);


    //------------//
    //    FM      //
	//------------//
    tmInfo.tmFunctionIndex   = FM;
    g_technologies.insert( technologyPair("FM", tmInfo) );
    g_testFunctions[FM].clear();
    g_testFunctions[FM].insert( functionPair("GLOBAL_SETTINGS",				callBack) );

    // Tester and DUT control functions
    g_testFunctions[FM].insert( functionPair("CONNECT_IQ_TESTER",			callBack) );
    g_testFunctions[FM].insert( functionPair("DISCONNECT_IQ_TESTER",		callBack) );    
    g_testFunctions[FM].insert( functionPair("INSERT_DUT",					callBack) );
    g_testFunctions[FM].insert( functionPair("INITIALIZE_DUT",				callBack) );
    g_testFunctions[FM].insert( functionPair("REMOVE_DUT",					callBack) );
    g_testFunctions[FM].insert( functionPair("RESET_DUT",					callBack) );
	//Run External program and path loss table
	g_testFunctions[FM].insert( functionPair("FM_LOAD_PATH_LOSS_TABLE",		callBack) );
	g_testFunctions[FM].insert( functionPair("RUN_EXTERNAL_PROGRAM",		callBack) );

    // Verification functions
	g_testFunctions[FM].insert( functionPair("TX_VERIFY_RF_MEASUREMENTS",   callBack) );
	g_testFunctions[FM].insert( functionPair("TX_VERIFY_AUDIO",				callBack) );

	g_testFunctions[FM].insert( functionPair("RX_VERIFY_AUDIO",				callBack) );
	g_testFunctions[FM].insert( functionPair("RX_VERIFY_RSSI",				callBack) );
	g_testFunctions[FM].insert( functionPair("RX_VERIFY_RDS",				callBack) );

	g_testFunctions[FM].insert( functionPair("LOAD_PATH_LOSS_TABLE",  callBack) );

    // Create the timer ID 
	TIMER_CreateTimer("FM_TM", &g_tmTimerID[FM]);
    // Create the logger ID 
	LOGGER_CreateLogger("FM_TM", &g_tmLoggerID[FM]);


    //------------//
    //   WIMAX    //
	//------------//
    tmInfo.tmFunctionIndex   = WIMAX;
    g_technologies.insert( technologyPair("WIMAX", tmInfo) );
    g_testFunctions[WIMAX].insert( functionPair("GLOBAL_SETTINGS",      callBack) );

    // Tester and DUT control functions
    g_testFunctions[WIMAX].insert( functionPair("CONNECT_IQ_TESTER",    callBack) );
    g_testFunctions[WIMAX].insert( functionPair("DISCONNECT_IQ_TESTER", callBack) );    

    g_testFunctions[WIMAX].insert( functionPair("INSERT_DUT",           callBack) );
    g_testFunctions[WIMAX].insert( functionPair("INITIALIZE_DUT",       callBack) );
    g_testFunctions[WIMAX].insert( functionPair("REMOVE_DUT",           callBack) );

	g_testFunctions[WIMAX].insert( functionPair("READ_EEPROM",          callBack) );
	g_testFunctions[WIMAX].insert( functionPair("WRITE_EEPROM",         callBack) );
    g_testFunctions[WIMAX].insert( functionPair("READ_MAC_ADDRESS",		callBack) );
    g_testFunctions[WIMAX].insert( functionPair("WRITE_MAC_ADDRESS",	callBack) );

    // Calibration functions
    g_testFunctions[WIMAX].insert( functionPair("TXRX_CALIBRATION",     callBack) );
	g_testFunctions[WIMAX].insert( functionPair("RESET_DUT",			callBack) );

    // Verification functions
    g_testFunctions[WIMAX].insert( functionPair("TXRX_VERIFICATION",    callBack) );
    g_testFunctions[WIMAX].insert( functionPair("TX_VERIFY_EVM",        callBack) );
	g_testFunctions[WIMAX].insert( functionPair("TX_VERIFY_POWER_STEP", callBack) );	
	g_testFunctions[WIMAX].insert( functionPair("LOAD_PATH_LOSS_TABLE", callBack) );
	g_testFunctions[WIMAX].insert( functionPair("TX_VERIFY_MASK",		callBack) );
	g_testFunctions[WIMAX].insert( functionPair("RX_VERIFY_PER",		callBack) );

    // Create the timer ID 
	TIMER_CreateTimer("WIMAX_TM", &g_tmTimerID[WIMAX]);
    // Create the logger ID 
	LOGGER_CreateLogger("WIMAX_TM", &g_tmLoggerID[WIMAX]);


	//------------//
    //    GPS     //
	//------------//
    tmInfo.tmFunctionIndex   = GPS;
    g_technologies.insert( technologyPair("GPS", tmInfo) );
    g_testFunctions[GPS].insert( functionPair("GLOBAL_SETTINGS",      callBack) );
    g_testFunctions[GPS].insert( functionPair("CONNECT_IQ_TESTER",	  callBack) );
    g_testFunctions[GPS].insert( functionPair("DISCONNECT_IQ_TESTER", callBack) );
    g_testFunctions[GPS].insert( functionPair("LOAD_PATH_LOSS_TABLE", callBack) );
    g_testFunctions[GPS].insert( functionPair("INSERT_DUT",           callBack) );
    g_testFunctions[GPS].insert( functionPair("INITIALIZE_DUT",       callBack) );
    g_testFunctions[GPS].insert( functionPair("REMOVE_DUT",           callBack) );
    g_testFunctions[GPS].insert( functionPair("TRANSMIT_SCENARIO",    callBack) );
    g_testFunctions[GPS].insert( functionPair("MODULATED_MODE",       callBack) );
    g_testFunctions[GPS].insert( functionPair("CONTINUE_WAVE",        callBack) );

	// Create the timer ID 
	TIMER_CreateTimer("GPS_TM", &g_tmTimerID[GPS]);
    // Create the logger ID 
	LOGGER_CreateLogger("GPS_TM", &g_tmLoggerID[GPS]);

	//-----------------//
    //    IQReport     //
	//-----------------//
    tmInfo.tmFunctionIndex   = IQREPORT;
    g_technologies.insert( technologyPair("IQREPORT", tmInfo) );

    g_testFunctions[IQREPORT].clear();
    g_testFunctions[IQREPORT].insert( functionPair("START_REPORT",  callBack) );
    g_testFunctions[IQREPORT].insert( functionPair("END_REPORT",    callBack) );
    g_testFunctions[IQREPORT].insert( functionPair("REPORT_INPUT",  callBack) );
    g_testFunctions[IQREPORT].insert( functionPair("REPORT_RESULT", callBack) );

    // Create the timer ID 
	TIMER_CreateTimer("IQREPORT_TM", &g_tmTimerID[IQREPORT]);
    // Create the logger ID 
	LOGGER_CreateLogger("IQREPORT_TM", &g_tmLoggerID[IQREPORT]);

    // Initialize the WiFi Frequency-Channel map, The formula for converting between freq in MHz is cneter freq(MHz)=5000+5xN, N=0,1,2...199
    // Parts of this map have not been verified and may not be valid.
	wifiChannels.clear();
	wifiChannels.insert( pair<int,int>( 2412, 1   ));
	wifiChannels.insert( pair<int,int>( 2417, 2   ));
	wifiChannels.insert( pair<int,int>( 2422, 3   ));
	wifiChannels.insert( pair<int,int>( 2427, 4   ));
	wifiChannels.insert( pair<int,int>( 2432, 5   ));
	wifiChannels.insert( pair<int,int>( 2437, 6   ));
	wifiChannels.insert( pair<int,int>( 2442, 7   ));
	wifiChannels.insert( pair<int,int>( 2447, 8   ));
	wifiChannels.insert( pair<int,int>( 2452, 9   ));
	wifiChannels.insert( pair<int,int>( 2457, 10  ));
	wifiChannels.insert( pair<int,int>( 2462, 11  ));
	wifiChannels.insert( pair<int,int>( 2467, 12  ));
	wifiChannels.insert( pair<int,int>( 2472, 13  ));
	wifiChannels.insert( pair<int,int>( 2484, 14  ));
	wifiChannels.insert( pair<int,int>( 4920, 184 ));
	wifiChannels.insert( pair<int,int>( 4940, 188 ));
	wifiChannels.insert( pair<int,int>( 4960, 192 ));
	wifiChannels.insert( pair<int,int>( 4980, 196 ));
	wifiChannels.insert( pair<int,int>( 5040, 8   ));
	wifiChannels.insert( pair<int,int>( 5060, 12  ));
	wifiChannels.insert( pair<int,int>( 5080, 16  ));
	wifiChannels.insert( pair<int,int>( 5170, 34  ));
	wifiChannels.insert( pair<int,int>( 5180, 36  ));
	wifiChannels.insert( pair<int,int>( 5190, 38  ));
	wifiChannels.insert( pair<int,int>( 5200, 40  ));
	wifiChannels.insert( pair<int,int>( 5210, 42  ));
	wifiChannels.insert( pair<int,int>( 5220, 44  ));
	wifiChannels.insert( pair<int,int>( 5230, 46  ));
	wifiChannels.insert( pair<int,int>( 5240, 48  ));
	wifiChannels.insert( pair<int,int>( 5260, 52  ));
    wifiChannels.insert( pair<int,int>( 5270, 54  ));
	wifiChannels.insert( pair<int,int>( 5280, 56  ));
	wifiChannels.insert( pair<int,int>( 5300, 60  ));
    wifiChannels.insert( pair<int,int>( 5310, 62  ));
	wifiChannels.insert( pair<int,int>( 5320, 64  ));
	wifiChannels.insert( pair<int,int>( 5400, 80  ));  //Add this channel number, which is not defined in 11a spec, for Atheros calibration. 
	wifiChannels.insert( pair<int,int>( 5500, 100 ));
    wifiChannels.insert( pair<int,int>( 5510, 102 ));
	wifiChannels.insert( pair<int,int>( 5520, 104 ));
	wifiChannels.insert( pair<int,int>( 5540, 108 ));
    wifiChannels.insert( pair<int,int>( 5550, 110 ));
	wifiChannels.insert( pair<int,int>( 5560, 112 ));
	wifiChannels.insert( pair<int,int>( 5580, 116 ));
    wifiChannels.insert( pair<int,int>( 5590, 118 ));
	wifiChannels.insert( pair<int,int>( 5600, 120 ));
	wifiChannels.insert( pair<int,int>( 5620, 124 ));
    wifiChannels.insert( pair<int,int>( 5630, 126 ));
	wifiChannels.insert( pair<int,int>( 5640, 128 ));
	wifiChannels.insert( pair<int,int>( 5660, 132 ));
    wifiChannels.insert( pair<int,int>( 5670, 134 ));
	wifiChannels.insert( pair<int,int>( 5680, 136 ));
	wifiChannels.insert( pair<int,int>( 5700, 140 ));
	wifiChannels.insert( pair<int,int>( 5720, 144 ));  //Add this channel number, which is not defined in 11a spec, for Atheros calibration. 
	wifiChannels.insert( pair<int,int>( 5745, 149 ));
	wifiChannels.insert( pair<int,int>( 5755, 151 ));
    wifiChannels.insert( pair<int,int>( 5765, 153 ));
	wifiChannels.insert( pair<int,int>( 5785, 157 ));
    wifiChannels.insert( pair<int,int>( 5795, 159 ));
	wifiChannels.insert( pair<int,int>( 5805, 161 ));
	wifiChannels.insert( pair<int,int>( 5820, 164 ));  //Add this channel number, which is not defined in 11a spec, for Atheros calibration. 
	wifiChannels.insert( pair<int,int>( 5825, 165 ));
	wifiChannels.insert( pair<int,int>( 5845, 169 ));
	wifiChannels.insert( pair<int,int>( 5865, 173 ));
	wifiChannels.insert( pair<int,int>( 5885, 177 ));
	wifiChannels.insert( pair<int,int>( 5905, 181 ));
	wifiChannels.insert( pair<int,int>( 5925, 185 ));
	wifiChannels.insert( pair<int,int>( 5945, 189 ));

	// Initializes the WiFi data rate index map
	wifiDataRates.clear();
	wifiDataRates.insert( pair<string, int>( "DSSS-1",   0 ));
	wifiDataRates.insert( pair<string, int>( "DSSS-2",   1 ));
	wifiDataRates.insert( pair<string, int>( "CCK-5_5",  2 ));
	wifiDataRates.insert( pair<string, int>( "CCK-11",   3 ));
	wifiDataRates.insert( pair<string, int>( "PBCC-22",  4 ));
	wifiDataRates.insert( pair<string, int>( "OFDM-6",   5 ));
	wifiDataRates.insert( pair<string, int>( "OFDM-9",   6 ));
	wifiDataRates.insert( pair<string, int>( "OFDM-12",  7 ));
	wifiDataRates.insert( pair<string, int>( "OFDM-18",  8 ));
	wifiDataRates.insert( pair<string, int>( "OFDM-24",  9 ));
	wifiDataRates.insert( pair<string, int>( "OFDM-36", 10 ));
	wifiDataRates.insert( pair<string, int>( "OFDM-48", 11 ));
	wifiDataRates.insert( pair<string, int>( "OFDM-54", 12 ));
	wifiDataRates.insert( pair<string, int>( "MCS0",    14 ));
	wifiDataRates.insert( pair<string, int>( "MCS1",    15 ));
	wifiDataRates.insert( pair<string, int>( "MCS2",    16 ));
	wifiDataRates.insert( pair<string, int>( "MCS3",    17 ));
	wifiDataRates.insert( pair<string, int>( "MCS4",    18 ));
	wifiDataRates.insert( pair<string, int>( "MCS5",    19 ));
	wifiDataRates.insert( pair<string, int>( "MCS6",    20 ));
	wifiDataRates.insert( pair<string, int>( "MCS7",    21 ));
	wifiDataRates.insert( pair<string, int>( "MCS8",    22 ));
	wifiDataRates.insert( pair<string, int>( "MCS9",    23 ));
	wifiDataRates.insert( pair<string, int>( "MCS10",   24 ));
	wifiDataRates.insert( pair<string, int>( "MCS11",   25 ));
	wifiDataRates.insert( pair<string, int>( "MCS12",   26 ));
	wifiDataRates.insert( pair<string, int>( "MCS13",   27 ));
	wifiDataRates.insert( pair<string, int>( "MCS14",   28 ));
	wifiDataRates.insert( pair<string, int>( "MCS15",   29 ));
	wifiDataRates.insert( pair<string, int>( "MCS16",   30 ));
	wifiDataRates.insert( pair<string, int>( "MCS17",   31 ));
	wifiDataRates.insert( pair<string, int>( "MCS18",   32 ));
	wifiDataRates.insert( pair<string, int>( "MCS19",   33 ));
	wifiDataRates.insert( pair<string, int>( "MCS20",   34 ));
	wifiDataRates.insert( pair<string, int>( "MCS21",   35 ));
	wifiDataRates.insert( pair<string, int>( "MCS22",   36 ));
	wifiDataRates.insert( pair<string, int>( "MCS23",   37 ));
	wifiDataRates.insert( pair<string, int>( "MCS24",   38 ));
	wifiDataRates.insert( pair<string, int>( "MCS25",   39 ));
	wifiDataRates.insert( pair<string, int>( "MCS26",   40 ));
	wifiDataRates.insert( pair<string, int>( "MCS27",   41 ));
	wifiDataRates.insert( pair<string, int>( "MCS28",   42 ));
	wifiDataRates.insert( pair<string, int>( "MCS29",   43 ));
	wifiDataRates.insert( pair<string, int>( "MCS30",   44 ));
	wifiDataRates.insert( pair<string, int>( "MCS31",   45 ));
	wifiDataRates.insert( pair<string, int>( "HALF-3",		200 ));
	wifiDataRates.insert( pair<string, int>( "HALF-4_5",	201 ));
	wifiDataRates.insert( pair<string, int>( "HALF-6",		202 ));
	wifiDataRates.insert( pair<string, int>( "HALF-9",		203 ));
	wifiDataRates.insert( pair<string, int>( "HALF-12",		204 ));
	wifiDataRates.insert( pair<string, int>( "HALF-18",		205 ));
	wifiDataRates.insert( pair<string, int>( "HALF-24",		206 ));
	wifiDataRates.insert( pair<string, int>( "HALF-27",		207 ));
	wifiDataRates.insert( pair<string, int>( "QUAR-1_5",	208 ));
	wifiDataRates.insert( pair<string, int>( "QUAR-2_25",	209 ));
	wifiDataRates.insert( pair<string, int>( "QUAR-3",		210 ));
	wifiDataRates.insert( pair<string, int>( "QUAR-4_5",	211 ));
	wifiDataRates.insert( pair<string, int>( "QUAR-6",		212 ));
	wifiDataRates.insert( pair<string, int>( "QUAR-9",		213 ));
	wifiDataRates.insert( pair<string, int>( "QUAR-12",		214 ));
	wifiDataRates.insert( pair<string, int>( "QUAR-13_5",	215 ));

	// Initializes the WiFi data rate Mbps map
	wifiDataRatesMbps.clear();
	wifiDataRatesMbps.insert( pair<string, double>( "DSSS-1",   1 ));
	wifiDataRatesMbps.insert( pair<string, double>( "DSSS-2",   2 ));
	wifiDataRatesMbps.insert( pair<string, double>( "CCK-5_5",  5.5 ));
	wifiDataRatesMbps.insert( pair<string, double>( "CCK-11",   11 ));
	wifiDataRatesMbps.insert( pair<string, double>( "PBCC-22",  22 ));
	wifiDataRatesMbps.insert( pair<string, double>( "OFDM-6",   5 ));
	wifiDataRatesMbps.insert( pair<string, double>( "OFDM-9",   9 ));
	wifiDataRatesMbps.insert( pair<string, double>( "OFDM-12",  12 ));
	wifiDataRatesMbps.insert( pair<string, double>( "OFDM-18",  18 ));
	wifiDataRatesMbps.insert( pair<string, double>( "OFDM-24",  24 ));
	wifiDataRatesMbps.insert( pair<string, double>( "OFDM-36",  36 ));
	wifiDataRatesMbps.insert( pair<string, double>( "OFDM-48",  48 ));
	wifiDataRatesMbps.insert( pair<string, double>( "OFDM-54",  54 ));

	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS0", 6.5 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS1", 13 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS2", 19.5 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS3", 26 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS4", 39 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS5", 52 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS6", 58.5 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS7", 65 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS8", 13 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS9", 26 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS10", 39 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS11", 52 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS12", 78 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS13", 104 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS14", 117 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS15", 130 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS16", 19.5 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS17", 39 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS18", 58.5 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS19", 78 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS20", 117 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS21", 156 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS22", 175.5 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS23", 195 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS24", 26 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS25", 52 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS26", 78 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS27", 104 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS28", 156 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS29", 208 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS30", 234 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT20_MCS31", 260 ));

	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS0", 13.5 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS1", 27 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS2", 40.5 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS3", 54 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS4", 81 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS5", 108 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS6", 121.5 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS7", 135 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS8", 27 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS9", 54 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS10", 81 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS11", 108 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS12", 162 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS13", 216 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS14", 243 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS15", 270 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS16", 40.5 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS17", 81 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS18", 121.5 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS19", 162 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS20", 243 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS21", 324 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS22", 364.5 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS23", 405 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS24", 54 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS25", 108 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS26", 162 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS27", 216 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS28", 324 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS29", 432 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS30", 486 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HT40_MCS31", 540 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HALF-3",	3 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HALF-4_5",	4.5 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HALF-6",	6 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HALF-9",	9 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HALF-12",	12 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HALF-18",	18 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HALF-24",	24 ));
	wifiDataRatesMbps.insert( pair<string, double>( "HALF-27",	27 ));
	wifiDataRatesMbps.insert( pair<string, double>( "QUAR-1_5",	1.5 ));
	wifiDataRatesMbps.insert( pair<string, double>( "QUAR-2_25",2.25 ));
	wifiDataRatesMbps.insert( pair<string, double>( "QUAR-3",	3 ));
	wifiDataRatesMbps.insert( pair<string, double>( "QUAR-4_5",	4.5 ));
	wifiDataRatesMbps.insert( pair<string, double>( "QUAR-6",	6 ));
	wifiDataRatesMbps.insert( pair<string, double>( "QUAR-9",	9 ));
	wifiDataRatesMbps.insert( pair<string, double>( "QUAR-12",	12 ));
	wifiDataRatesMbps.insert( pair<string, double>( "QUAR-13_5",13.5 ));

	// Initializes the WiFi data rate IQ2010Ext map
	dataRateIQ2010Ext.clear();
	dataRateIQ2010Ext.insert( pair<string, int>( "DSSS-1",   0 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "DSSS-2",   1 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "CCK-5_5",  2 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "CCK-11",   3 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "OFDM-6",   4 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "OFDM-9",   5 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "OFDM-12",  6 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "OFDM-18",  7 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "OFDM-24",  8 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "OFDM-36",  9 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "OFDM-48",  10 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "OFDM-54",  11 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS0",	  12 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS1",	  13 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS2",	  14 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS3",	  15 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS4",	  16 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS5",	  17 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS6",	  18 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS7",	  19 ));

    //-----------------//
    //   Device Info   //
	//-----------------//
    g_dutInfo.sSerialNumber = "";
    g_dutInfo.sManufacturer = "";
    g_dutInfo.sModel = "";
    g_dutInfo.sRevision = "";
    g_dutInfo.sDescription = "";

    return 1;												 
}

TM_API TM_RETURN __stdcall TM_InstallCallbackFunction(TM_ID technologyID, const TM_STR functionKeyword, int (*pointerToFunction)(void) )
{
    TM_RETURN ret = TM_ERR_OK;
    
    map <string, TM_CALLBACK>::iterator function_Iter;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {

        function_Iter = g_testFunctions[technologyID].find( functionKeyword );

        if( function_Iter!=g_testFunctions[technologyID].end() )
        {
            function_Iter->second.pointerToFunction = pointerToFunction;
        }
        else
        {
            ret = TM_ERR_FUNCTION_NOT_DEFINED;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_ClearParameters(TM_ID technologyID)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        g_intParams[technologyID].clear();
        g_doubleParams[technologyID].clear();
        g_stringParams[technologyID].clear();
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_ClearReturns(TM_ID technologyID)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        g_intReturns[technologyID].clear();
        g_doubleReturns[technologyID].clear();
        g_stringReturns[technologyID].clear();
        g_itemUnits[technologyID].clear();
        g_helpText[technologyID].clear();
        g_seqDataRateResults[technologyID].clear();

        g_arrayDoubleReturn_Iter[technologyID] = g_arrayDoubleReturns[technologyID].begin();
        while( g_arrayDoubleReturn_Iter[technologyID] != g_arrayDoubleReturns[technologyID].end() )
        {
            g_arrayDoubleReturn_Iter[technologyID]->second.clear();
            g_arrayDoubleReturn_Iter[technologyID]++;
        }
        g_arrayDoubleReturns[technologyID].clear();
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_AddIntegerParameter(TM_ID technologyID, const TM_STR paramName, const int paramValue)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        g_intParams[technologyID].insert( intParamPair(paramName, paramValue) );
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }
    return ret;
}

TM_API TM_RETURN __stdcall TM_AddIntegerReturn(TM_ID technologyID, const TM_STR paramName, const int paramValue)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        g_intReturns[technologyID].insert( intReturnPair(paramName, paramValue) );
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }
    return ret;
}

TM_API TM_RETURN __stdcall TM_GetIntegerParameter(TM_ID technologyID, const TM_STR paramName, int* paramValue)
{
    TM_RETURN ret = TM_ERR_OK;

    map <string, int>::iterator intParam_Iter;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        intParam_Iter = g_intParams[technologyID].find(paramName);
        if( intParam_Iter!=g_intParams[technologyID].end() )
        {
            *paramValue = intParam_Iter->second;
        }
        else
        {
            ret = TM_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_GetIntegerReturn(TM_ID technologyID, const TM_STR paramName, int* paramValue)
{
    TM_RETURN ret = TM_ERR_OK;

    map <string, int>::iterator intParam_Iter;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        intParam_Iter = g_intReturns[technologyID].find(paramName);
        if( intParam_Iter!=g_intReturns[technologyID].end() )
        {
            *paramValue = intParam_Iter->second;
        }
        else
        {
            ret = TM_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_GetIntegerReturnPair(TM_ID technologyID, 
                                         TM_STR paramName, 
                                         int bufferSize, 
                                         TM_STR unit, 
                                         int unitSize, 
                                         TM_STR helpText, 
                                         int helpTextSize, 
                                         int* paramValue, 
                                         int order)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        if( FIRST==order )
        {
            // The first time to retrieve the info
            g_intReturn_Iter[technologyID] = g_intReturns[technologyID].begin();
        }
        else
        {
            // Next
            g_intReturn_Iter[technologyID]++;
        }
        if( g_intReturn_Iter[technologyID]!=g_intReturns[technologyID].end() )
        {
            strcpy_s((char*)paramName, bufferSize, g_intReturn_Iter[technologyID]->first.c_str());
            if(NULL!=paramValue)
            {
                *paramValue = g_intReturn_Iter[technologyID]->second;
            }
            if( NULL!=unit  )
            {
                // Get the item unit from the itemUnits container
                g_itemUnit_Iter[technologyID] = g_itemUnits[technologyID].find(g_intReturn_Iter[technologyID]->first);
                if( g_itemUnit_Iter[technologyID]!=g_itemUnits[technologyID].end() )
                {
                    strcpy_s((char*)unit, unitSize, g_itemUnit_Iter[technologyID]->second.c_str());
                }
            }
            if( NULL!=helpText  )
            {
                // Get the help text from the helpText container
                g_helpText_Iter[technologyID] = g_helpText[technologyID].find(g_intReturn_Iter[technologyID]->first);
                if( g_helpText_Iter[technologyID]!=g_helpText[technologyID].end() )
                {
                    strcpy_s((char*)helpText, helpTextSize, g_helpText_Iter[technologyID]->second.c_str());
                }
            }
        }
        else
        {
            ret = TM_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_GetIntegerParamPair(TM_ID technologyID, 
                                                  TM_STR paramName, 
                                                  int bufferSize, 
                                                  TM_STR unit, 
                                                  int unitSize, 
                                                  TM_STR helpText, 
                                                  int helpTextSize, 
                                                  int* paramValue, 
                                                  int order)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        if( FIRST==order )
        {
            // The first time to retrieve the info
            g_intParams_Iter[technologyID] = g_intParams[technologyID].begin();
        }
        else
        {
            // Next
            g_intParams_Iter[technologyID]++;
        }
        if( g_intParams_Iter[technologyID]!=g_intParams[technologyID].end() )
        {
            strcpy_s((char*)paramName, bufferSize, g_intParams_Iter[technologyID]->first.c_str());
            if(NULL!=paramValue)
            {
                *paramValue = g_intParams_Iter[technologyID]->second;
            }
            if( NULL!=unit )
            {
                // Get the item unit from the itemUnits container
                g_itemUnit_Iter[technologyID] = g_itemUnits[technologyID].find(g_intParams_Iter[technologyID]->first);
                if( g_itemUnit_Iter[technologyID]!=g_itemUnits[technologyID].end() )
                {
                    strcpy_s((char*)unit, unitSize, g_itemUnit_Iter[technologyID]->second.c_str());
                }
            }
            if( NULL!=helpText )
            {
                // Get the help text from the helpText container
                g_helpText_Iter[technologyID] = g_helpText[technologyID].find(g_intParams_Iter[technologyID]->first);
                if( g_helpText_Iter[technologyID]!=g_helpText[technologyID].end() )
                {
                    strcpy_s((char*)helpText, helpTextSize, g_helpText_Iter[technologyID]->second.c_str());
                }
            }
        }
        else
        {
            ret = TM_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_AddDoubleParameter(TM_ID technologyID, const TM_STR paramName, const double paramValue)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        g_doubleParams[technologyID].insert( doubleParamPair(paramName, paramValue) );
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }
    return ret;
}

TM_API TM_RETURN __stdcall TM_AddDoubleReturn(TM_ID technologyID, const TM_STR paramName, const double paramValue)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        g_doubleReturns[technologyID].insert( doubleReturnPair(paramName, paramValue) );
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }
    return ret;
}

TM_API TM_RETURN __stdcall TM_SetDoubleParameter(TM_ID technologyID, const TM_STR paramName, double paramValue)
{
	TM_RETURN ret = TM_ERR_OK;

	map <string, double>::iterator doubleParam_Iter;

	if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
	{
		doubleParam_Iter = g_doubleParams[technologyID].find(paramName);
		if( doubleParam_Iter!=g_doubleParams[technologyID].end() )
		{
			doubleParam_Iter->second = paramValue;
		}
		else
		{
			ret = TM_ERR_PARAM_DOES_NOT_EXIST;
		}
	}
	else
	{
		ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
	}

	return ret;
}

TM_API TM_RETURN __stdcall TM_GetDoubleParameter(TM_ID technologyID, const TM_STR paramName, double* paramValue)
{
    TM_RETURN ret = TM_ERR_OK;

    map <string, double>::iterator doubleParam_Iter;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        doubleParam_Iter = g_doubleParams[technologyID].find(paramName);
        if( doubleParam_Iter!=g_doubleParams[technologyID].end() )
        {
            *paramValue = doubleParam_Iter->second;
        }
        else
        {
            ret = TM_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_GetDoubleReturn(TM_ID technologyID, const TM_STR paramName, double* paramValue)
{
    TM_RETURN ret = TM_ERR_OK;

    map <string, double>::iterator doubleReturn_Iter;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        doubleReturn_Iter = g_doubleReturns[technologyID].find(paramName);
        if( doubleReturn_Iter!=g_doubleReturns[technologyID].end() )
        {
            *paramValue = doubleReturn_Iter->second;
        }
        else
        {
            ret = TM_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_GetDoubleParamPair(TM_ID technologyID, 
                                                 TM_STR paramName, 
                                                 int bufferSize, 
                                                 TM_STR unit, 
                                                 int unitSize, 
                                                 TM_STR helpText, 
                                                 int helpTextSize, 
                                                 double* paramValue, 
                                                 int order)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        if( FIRST==order )
        {
            // The first time to retrieve the info
            g_doubleParam_Iter[technologyID] = g_doubleParams[technologyID].begin();
        }
        else
        {
            // Next
            g_doubleParam_Iter[technologyID]++;
        }
        if( g_doubleParam_Iter[technologyID]!=g_doubleParams[technologyID].end() )
        {
            strcpy_s((char*)paramName, bufferSize, g_doubleParam_Iter[technologyID]->first.c_str());
            if(NULL!=paramValue)
            {
                *paramValue = g_doubleParam_Iter[technologyID]->second;
            }
            if( NULL!=unit )
            {
                // Get the item unit from the itemUnits container
                g_itemUnit_Iter[technologyID] = g_itemUnits[technologyID].find(g_doubleParam_Iter[technologyID]->first);
                if( g_itemUnit_Iter[technologyID]!=g_itemUnits[technologyID].end() )
                {
                    strcpy_s((char*)unit, unitSize, g_itemUnit_Iter[technologyID]->second.c_str());
                }
            }
            if( NULL!=helpText )
            {
                // Get the help text from the helpText container
                g_helpText_Iter[technologyID] = g_helpText[technologyID].find(g_doubleParam_Iter[technologyID]->first);
                if( g_helpText_Iter[technologyID]!=g_helpText[technologyID].end() )
                {
                    strcpy_s((char*)helpText, helpTextSize, g_helpText_Iter[technologyID]->second.c_str());
                }
            }
        }
        else
        {
            ret = TM_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_GetDoubleReturnPair(TM_ID technologyID, 
                                         TM_STR paramName, 
                                         int bufferSize, 
                                         TM_STR unit, 
                                         int unitSize, 
                                         TM_STR helpText, 
                                         int helpTextSize, 
                                         double* paramValue, 
                                         int order)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        if( FIRST==order )
        {
            // The first time to retrieve the info
            g_doubleReturn_Iter[technologyID] = g_doubleReturns[technologyID].begin();
        }
        else
        {
            // Next
            g_doubleReturn_Iter[technologyID]++;
        }
        if( g_doubleReturn_Iter[technologyID]!=g_doubleReturns[technologyID].end() )
        {
            strcpy_s((char*)paramName, bufferSize, g_doubleReturn_Iter[technologyID]->first.c_str());
            if(NULL!=paramValue)
            {
                *paramValue = g_doubleReturn_Iter[technologyID]->second;
            }
            if( NULL!=unit  )
            {
                // Get the item unit from the itemUnits container
                g_itemUnit_Iter[technologyID] = g_itemUnits[technologyID].find(g_doubleReturn_Iter[technologyID]->first);
                if( g_itemUnit_Iter[technologyID]!=g_itemUnits[technologyID].end() )
                {
                    strcpy_s((char*)unit, unitSize, g_itemUnit_Iter[technologyID]->second.c_str());
                }
            }
            if( NULL!=helpText  )
            {
                // Get the help text from the helpText container
                g_helpText_Iter[technologyID] = g_helpText[technologyID].find(g_doubleReturn_Iter[technologyID]->first);
                if( g_helpText_Iter[technologyID]!=g_helpText[technologyID].end() )
                {
                    strcpy_s((char*)helpText, helpTextSize, g_helpText_Iter[technologyID]->second.c_str());
                }
            }
        }
        else
        {
            ret = TM_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_AddStringParameter(TM_ID technologyID, const TM_STR paramName, const TM_STR paramValue)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        g_stringParams[technologyID].insert( stringParamPair(paramName, paramValue) );
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }
    return ret;
}

TM_API TM_RETURN __stdcall TM_AddStringReturn(TM_ID technologyID, const TM_STR paramName, const TM_STR paramValue)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        g_stringReturns[technologyID].insert( stringReturnPair(paramName, paramValue) );
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }
    return ret;
}

TM_API TM_RETURN __stdcall TM_AddArrayDoubleReturn(TM_ID technologyID, const TM_STR paramName, const double *paramValue, const int arraySize)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        if( arraySize>0 )
        {
            vector<double> arrayDouble;
            for( int i=0; i<arraySize; i++)
            {
                arrayDouble.push_back( paramValue[i] );
            }
            g_arrayDoubleReturns[technologyID].insert( arrayDoubleReturnPair(paramName, arrayDouble) );
        }
        else
        {
            vector<double> arrayDouble;
            g_arrayDoubleReturns[technologyID].insert( arrayDoubleReturnPair(paramName, arrayDouble) );
        
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }
    return ret;
}

TM_API TM_RETURN __stdcall TM_AddUnitReturn(TM_ID technologyID, const TM_STR paramName, const TM_STR unit)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        g_itemUnits[technologyID].insert( pair<string,string>(paramName, unit) );
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }
    return ret;
}

TM_API TM_RETURN __stdcall TM_GetUnitReturn(TM_ID technologyID, const TM_STR paramName, TM_STR unitValue, int bufferSize)
{
    TM_RETURN ret = TM_ERR_OK;

    map <string, string>::iterator stringParam_Iter;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        stringParam_Iter = g_itemUnits[technologyID].find(paramName);
        if( stringParam_Iter!=g_itemUnits[technologyID].end() )
        {
            strcpy_s(unitValue, bufferSize, stringParam_Iter->second.c_str()); 
        }
        else
        {
            ret = TM_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_AddHelpTextReturn(TM_ID technologyID, const TM_STR paramName, const TM_STR help)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        g_helpText[technologyID].insert( pair<string,string>(paramName, help) );
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }
    return ret;
}

TM_API TM_RETURN __stdcall TM_GetStringParameter(TM_ID technologyID, const TM_STR paramName, TM_STR paramValue, int bufferSize)
{
    TM_RETURN ret = TM_ERR_OK;

    map <string, string>::iterator stringParam_Iter;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        stringParam_Iter = g_stringParams[technologyID].find(paramName);
        if( stringParam_Iter!=g_stringParams[technologyID].end() )
        {
            strcpy_s(paramValue, bufferSize, stringParam_Iter->second.c_str()); 
        }
        else
        {
            ret = TM_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_GetStringReturn(TM_ID technologyID, const TM_STR paramName, TM_STR paramValue, int bufferSize)
{
    TM_RETURN ret = TM_ERR_OK;

    map <string, string>::iterator stringReturn_Iter;
    
    if( NULL!=paramValue )
    {
        memset(paramValue, 0, bufferSize);
    }
    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        stringReturn_Iter = g_stringReturns[technologyID].find(paramName);
        if( stringReturn_Iter!=g_stringReturns[technologyID].end() )
        {
            //paramValue = (TM_STR)reinterpret_cast<const char *>(stringReturn_Iter->second.c_str());
            strcpy_s(paramValue, bufferSize, stringReturn_Iter->second.c_str());
        }
        else
        {
            ret = TM_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_GetArrayDoubleReturnSize(TM_ID technologyID, const TM_STR paramName,int* arraySize)
{

	TM_RETURN ret = TM_ERR_OK;
	map <string, vector<double> >::iterator arrayDoubleReturn_Iter;


    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        arrayDoubleReturn_Iter = g_arrayDoubleReturns[technologyID].find(paramName);
        if( arrayDoubleReturn_Iter!=g_arrayDoubleReturns[technologyID].end() )
        {
			*arraySize = (int) arrayDoubleReturn_Iter->second.size();
        }
        else
        {
            ret = TM_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;


}



TM_API TM_RETURN __stdcall TM_GetArrayDoubleReturn(TM_ID technologyID, const TM_STR paramName, double *paramValue, const int arraySize)
{
    TM_RETURN ret = TM_ERR_OK;

    map <string, vector<double> >::iterator arrayDoubleReturn_Iter;
    
    //if( NULL!=paramValue )
    //{
    //    memset(paramValue, 0, bufferSize);
    //}
    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        arrayDoubleReturn_Iter = g_arrayDoubleReturns[technologyID].find(paramName);
        if( arrayDoubleReturn_Iter!=g_arrayDoubleReturns[technologyID].end() )
        {
            for(int i=0; i<min(arraySize,(int)arrayDoubleReturn_Iter->second.size()); i++)
            {
                paramValue[i] = arrayDoubleReturn_Iter->second[i];
            }
        }
        else
        {
            ret = TM_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_GetStringParamPair(TM_ID technologyID, 
                                                 TM_STR paramName, 
                                                 int bufferSize, 
                                                 TM_STR unit, 
                                                 int unitSize, 
                                                 TM_STR helpText, 
                                                 int helpTextSize, 
                                                 TM_STR paramValue,
                                                 int paramValueBufferSize,
                                                 int order)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        if( FIRST==order )
        {
            // The first time to retrieve the info
            g_stringParam_Iter[technologyID] = g_stringParams[technologyID].begin();
        }
        else
        {
            // Next
            g_stringParam_Iter[technologyID]++;
        }
        if( g_stringParam_Iter[technologyID]!=g_stringParams[technologyID].end() )
        {
            strcpy_s((char*)paramName, bufferSize, g_stringParam_Iter[technologyID]->first.c_str());
            if(NULL!=paramValue)
            {
                strcpy_s((char*)paramValue, paramValueBufferSize, g_stringParam_Iter[technologyID]->second.c_str());
            }
            if( NULL!=unit )
            {
                // Get the item unit from the itemUnits container
                g_itemUnit_Iter[technologyID] = g_itemUnits[technologyID].find(g_stringParam_Iter[technologyID]->first);
                if( g_itemUnit_Iter[technologyID]!=g_itemUnits[technologyID].end() )
                {
                    strcpy_s((char*)unit, unitSize, g_itemUnit_Iter[technologyID]->second.c_str());
                }
            }
            if( NULL!=helpText )
            {
                // Get the help text from the helpText container
                g_helpText_Iter[technologyID] = g_helpText[technologyID].find(g_stringParam_Iter[technologyID]->first);
                if( g_helpText_Iter[technologyID]!=g_helpText[technologyID].end() )
                {
                    strcpy_s((char*)helpText, helpTextSize, g_helpText_Iter[technologyID]->second.c_str());
                }
            }
        }
        else
        {
            ret = TM_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_GetStringReturnPair(TM_ID technologyID, 
                                        TM_STR paramName, 
                                        int bufferSize, 
                                        TM_STR unit, 
                                        int unitSize, 
                                        TM_STR helpText, 
                                        int helpTextSize, 
                                        TM_STR paramValue,
                                        int paramValueBufferSize,
                                        int order)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        if( FIRST==order )
        {
            // The first time to retrieve the info
            g_stringReturn_Iter[technologyID] = g_stringReturns[technologyID].begin();
        }
        else
        {
            // Next
            g_stringReturn_Iter[technologyID]++;
        }
        if( g_stringReturn_Iter[technologyID]!=g_stringReturns[technologyID].end() )
        {
            strcpy_s((char*)paramName, bufferSize, g_stringReturn_Iter[technologyID]->first.c_str());
            if(NULL!=paramValue)
            {
                strcpy_s((char*)paramValue, paramValueBufferSize, g_stringReturn_Iter[technologyID]->second.c_str());
            }
            if( NULL!=unit  )
            {
                // Get the item unit from the itemUnits container
                g_itemUnit_Iter[technologyID] = g_itemUnits[technologyID].find(g_stringReturn_Iter[technologyID]->first);
                if( g_itemUnit_Iter[technologyID]!=g_itemUnits[technologyID].end() )
                {
                    strcpy_s((char*)unit, unitSize, g_itemUnit_Iter[technologyID]->second.c_str());
                }
            }
            if( NULL!=helpText  )
            {
                // Get the help text from the helpText container
                g_helpText_Iter[technologyID] = g_helpText[technologyID].find(g_stringReturn_Iter[technologyID]->first);
                if( g_helpText_Iter[technologyID]!=g_helpText[technologyID].end() )
                {
                    strcpy_s((char*)helpText, helpTextSize, g_helpText_Iter[technologyID]->second.c_str());
                }
            }
        }
        else
        {
            ret = TM_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_GetArrayDoubleReturnPair(TM_ID technologyID, 
                                        TM_STR paramName, 
                                        int bufferSize, 
                                         TM_STR unit, 
                                         int unitSize, 
                                         TM_STR helpText, 
                                         int helpTextSize, 
                                        double* paramValue,
                                        int order)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        if( FIRST==order )
        {
            // The first time to retrieve the info
            g_arrayDoubleReturn_Iter[technologyID] = g_arrayDoubleReturns[technologyID].begin();
        }
        else
        {
            // Next
            g_arrayDoubleReturn_Iter[technologyID]++;
        }
        if( g_arrayDoubleReturn_Iter[technologyID]!=g_arrayDoubleReturns[technologyID].end() )
        {
            strcpy_s((char*)paramName, bufferSize, g_arrayDoubleReturn_Iter[technologyID]->first.c_str());
            if(NULL!=paramValue)
            {
                *paramValue = g_arrayDoubleReturn_Iter[technologyID]->second.size();
            }
            if( NULL!=unit  )
            {
                // Get the item unit from the itemUnits container
                g_itemUnit_Iter[technologyID] = g_itemUnits[technologyID].find(g_arrayDoubleReturn_Iter[technologyID]->first);
                if( g_itemUnit_Iter[technologyID]!=g_itemUnits[technologyID].end() )
                {
                    strcpy_s((char*)unit, unitSize, g_itemUnit_Iter[technologyID]->second.c_str());
                }
            }
            if( NULL!=helpText  )
            {
                // Get the help text from the helpText container
                g_helpText_Iter[technologyID] = g_helpText[technologyID].find(g_arrayDoubleReturn_Iter[technologyID]->first);
                if( g_helpText_Iter[technologyID]!=g_helpText[technologyID].end() )
                {
                    strcpy_s((char*)helpText, helpTextSize, g_helpText_Iter[technologyID]->second.c_str());
                }
            }
        }
        else
        {
            ret = TM_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;

}

TM_API TM_RETURN __stdcall TM_Run(TM_ID technologyID, const TM_STR functionKeyword)
{
    TM_RETURN ret = TM_ERR_OK;

	double durationInMiniSec = 0;

    map <string, TM_CALLBACK>::iterator function_Iter;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        //printf("calling the find method of map...\n");
        function_Iter = g_testFunctions[technologyID].find( functionKeyword );

        if( function_Iter!=g_testFunctions[technologyID].end() )
        {
            if( NULL!=function_Iter->second.pointerToFunction )
            {
                // Start Timer
				::TIMER_StartTimer(g_tmTimerID[technologyID], functionKeyword);
				::LOGGER_Write_Ext(LOG_IQLITE_TM, g_tmLoggerID[technologyID], LOGGER_INFORMATION, "\n[ TM ]=>TM_Run[%s]\n", functionKeyword);

				LogTestInputParameters( technologyID, functionKeyword );

                if( 0==function_Iter->second.pointerToFunction() )
                {
                    // DUT function ran OK
                }
                else
                {
                    // DUT function had an error
                    // TODO: how do we handle the errors from DUT functions?
                    // Option 1: to insert a string message before returing
                    ret = TM_ERR_FUNCTION_ERROR;
                }
                
				LogTestResults( technologyID, functionKeyword );

				// Stop Timer
				::TIMER_StopTimer(g_tmTimerID[technologyID], functionKeyword, &durationInMiniSec);
				// Save to log
				::LOGGER_Write_Ext(LOG_IQLITE_TM, g_tmLoggerID[technologyID], LOGGER_INFORMATION, "[ TM ]=>[%s],%.2f,ms\n", functionKeyword, durationInMiniSec);

            }
            else
            {
                //printf("DUT does not support %s\n", functionKeyword);
                ret = TM_ERR_FUNCTION_NOT_SUPPORTED;
            }
        }
        else
        {
            //printf("%s cannot be found\n", functionKeyword);
            ret = TM_ERR_FUNCTION_NOT_DEFINED;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_RegisterTechnology(const TM_STR technology, TM_ID *technologyID)
{
    TM_RETURN ret = TM_ERR_OK;

    // We need to check if the specified technology is one of the pre-defined, or
    // if another DUT has registered the same technology already
    g_technology_Iter = g_technologies.find( technology );
    if( g_technology_Iter == g_technologies.end() )
    {
        ret = TM_ERR_TECHNOLOGY_DOES_NOT_EXIST;
    }
    else
    {
        if( g_technology_Iter->second.technologyID != -1 )
        {
            ret = TM_ERR_TECHNOLOGY_HAS_BEEN_REGISTERED;
        }
        else
        {
            // Register the technology with the DUT
            // Since we only allow one DUT to register one technology, we can use
            // tmFunctionIndex as the DUT ID to make it easier to call the DUT's functions;
            // otherwise, if the DUT ID is different from the function index, we have to
            // a search each time we call a DUT function in map "g_technologies"
            g_technology_Iter->second.technologyID = g_technology_Iter->second.tmFunctionIndex;
            *technologyID = g_technology_Iter->second.tmFunctionIndex;
        }
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_UnregisterTechnology(const TM_STR technology)
{
    TM_RETURN ret = TM_ERR_OK;

    // We need to check if the specified technology is one of the pre-defined, or
    // if another DUT has registered the same technology already
    g_technology_Iter = g_technologies.find( technology );
    if( g_technology_Iter == g_technologies.end() )
    {
        ret = TM_ERR_TECHNOLOGY_DOES_NOT_EXIST;
    }
    else
    {
        if( g_technology_Iter->second.technologyID != -1 )
        {
            // Unregister
            int tfIndex = g_technology_Iter->second.technologyID;
            g_technology_Iter->second.technologyID = -1;
            // Set all callback functions to NULL

            map <string, TM_CALLBACK>::iterator function_Iter;
            for(function_Iter=g_testFunctions[tfIndex].begin(); 
                function_Iter!=g_testFunctions[tfIndex].end();
                function_Iter++)
            {
                function_Iter->second.pointerToFunction = NULL;
            }
        }
        else
        {
            // No DUT has been registered.  Do nothing.
        }
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_RegisterTechnologyDll(const TM_STR technology, TM_STR technologyDllFileName, TM_ID *technologyID)
{
    TM_RETURN ret = TM_ERR_OK;

    // We need to check if the specified technology is one of the pre-defined, or
    // if the specified technology is taken

    g_technology_Iter = g_technologies.find( technology );
    try
    {
        if( g_technology_Iter == g_technologies.end() )
        {
            throw TM_ERR_TECHNOLOGY_DOES_NOT_EXIST;
        }

		if( g_technology_Iter->second.technologyID != -1 )
        {
            throw TM_ERR_TECHNOLOGY_HAS_BEEN_REGISTERED;
        }

        // Load the DUT control DLL
        HMODULE technologyDllHandle = LoadLibraryA( technologyDllFileName );
        if( NULL==technologyDllHandle )
        {
            //DWORD lastError = GetLastError();
            throw TM_ERR_FAILED_TO_LOAD_TECHNOLOGY_DLL;
        }

        string registerFunctionName(technology);
        registerFunctionName += "_technologyRegister";

        technologyRegister technologyRegisterFunction;

        // Get the Dut register function address
        technologyRegisterFunction = (technologyRegister)::GetProcAddress( technologyDllHandle, registerFunctionName.c_str() );

        if( NULL==technologyRegisterFunction )
        {
            FreeLibrary( technologyDllHandle );
			throw TM_ERR_NO_TECHNOLOGY_REGISTER_FUNCTION_FOUND;
        }

        // So far, so good.  We are going to execute the function
        // technologyRegisterFunction() does not follow the general rule in terms of return value
        // If it returns a value >=0, registration succeeded.
        // If it returns -1, failed.
        *technologyID = technologyRegisterFunction();
        if( -1==*technologyID )
        {
            // The DUT register function failed
            FreeLibrary( technologyDllHandle );
			throw TM_ERR_TECHNOLOGY_REGISTER_FUNCTION_FAILED;
        }

        // Save the handle for FreeLibrary()
        g_technologyDllHandle[*technologyID] = technologyDllHandle;

        // Take care of extensions
        ProcessExtensions(*technologyID, technologyDllFileName);

    }
    catch(TM_RETURN errCode)
    {
        ret = errCode;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_UnregisterTechnologyDll(const TM_STR technology, TM_ID *technologyID)
{
    TM_RETURN ret = TM_ERR_OK;

    // We need to check if the specified technology is one of the pre-defined, or
    // if another Test module has registered the same technology already
    g_technology_Iter = g_technologies.find( technology );
    if( g_technology_Iter == g_technologies.end() )
    {
        ret = TM_ERR_TECHNOLOGY_DOES_NOT_EXIST;
    }
    else
    {
        if( g_technology_Iter->second.technologyID != -1 )
        {
            // unload the DLL
			if( NULL!=g_technologyDllHandle[g_technology_Iter->second.technologyID] )
            {
				FreeLibrary( g_technologyDllHandle[g_technology_Iter->second.technologyID] );
				g_technologyDllHandle[g_technology_Iter->second.technologyID] = NULL;
            }
            else
            {
                // do nothing
            }
            
            // Unregister
            int dfIndex = g_technology_Iter->second.technologyID;
			g_technology_Iter->second.technologyID = -1;
            // Set all callback functions to NULL

            map <string, TM_CALLBACK>::iterator function_Iter;
            for(function_Iter=g_testFunctions[dfIndex].begin(); 
                function_Iter!=g_testFunctions[dfIndex].end();
                function_Iter++)
            {
                function_Iter->second.pointerToFunction = NULL;
            }

            // Change the technology ID to -1
            *technologyID = -1;
        }
        else
        {
            // No technology has been registered.  Do nothing.
        }
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_GetTechnologyInfo(char *technologyName, 
                                      int bufferSize, 
                                      int *technologyIndex, 
                                      int *testFunctionCount, 
                                      int order)
{
    TM_RETURN ret = TM_ERR_OK;

    if ( g_technologies.size() <=0 )
    {
        ret = TM_ERR_TECHNOLOGY_DOES_NOT_EXIST;
    }
    else
    {
        if( FIRST==order )
        {
            // The first time to retrieve the technology info
            g_technology_Iter = g_technologies.begin();
        }
        else
        {
            // Next
            g_technology_Iter++;
        }
        if( g_technology_Iter != g_technologies.end() )
        {
            // Copy the technology name to the return buffer
            strcpy_s( technologyName, bufferSize, g_technology_Iter->first.c_str());
            *technologyIndex = g_technology_Iter->second.tmFunctionIndex;
            // Get the number of pre-defined function keywords for the technology
            *testFunctionCount = (int)g_testFunctions[g_technology_Iter->second.tmFunctionIndex].size();
            ret = TM_ERR_OK;
        }
        else
        {
            *testFunctionCount = 0;
            ret = TM_ERR_TECHNOLOGY_DOES_NOT_EXIST;
        }
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_GetTechnologyIndex(const char* technologyName, 
                                      int *technologyIndex, 
                                      int *testFunctionCount)
{
    TM_RETURN ret = TM_ERR_OK;

    if ( g_technologies.size() <=0 )
    {
        ret = TM_ERR_TECHNOLOGY_DOES_NOT_EXIST;
    }
    else
    {
		g_technology_Iter = g_technologies.find( technologyName );
        if( g_technology_Iter != g_technologies.end() )
        {
            *technologyIndex = g_technology_Iter->second.tmFunctionIndex;
            // Get the number of pre-defined function keywords for the technology
            *testFunctionCount = (int)g_testFunctions[g_technology_Iter->second.tmFunctionIndex].size();
            ret = TM_ERR_OK;
        }
        else
        {
            *testFunctionCount = 0;
            ret = TM_ERR_TECHNOLOGY_DOES_NOT_EXIST;
        }
    }

    return ret;
}


TM_API TM_RETURN __stdcall TM_GetFunctionKeyword(const char* technologyName, 
                                       char *functionKeyword, 
                                       int bufferSize, 
                                       BOOL *callbackInstalled,
                                       int order)
{
    TM_RETURN ret = TM_ERR_OK;

    g_technology_Iter = g_technologies.find( technologyName );
    if( g_technology_Iter == g_technologies.end() )
    {
        ret = TM_ERR_TECHNOLOGY_DOES_NOT_EXIST;
    }
    else
    {
        int index = g_technology_Iter->second.tmFunctionIndex;
        if( FIRST==order )
        {
            // The first time to retrieve the technology info
            g_function_Iter[index] = g_testFunctions[index].begin();
        }
        else
        {
            // Next
            g_function_Iter[index]++;
        }
        if( g_function_Iter[index] != g_testFunctions[index].end() )
        {
            // Copy the function keyword to the return buffer
            strcpy_s( functionKeyword, bufferSize, g_function_Iter[index]->first.c_str());
            // Check the installed callback function
            if( NULL==g_function_Iter[index]->second.pointerToFunction )
            {
                *callbackInstalled = FALSE;
            }
            else
            {
                *callbackInstalled = TRUE;
            }
            ret = TM_ERR_OK;
        }
        else
        {
            ret = TM_ERR_TECHNOLOGY_DOES_NOT_EXIST;
        }
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_WiFiConvertFrequencyToChannel(int freq, int* channel)
{
    TM_RETURN ret = TM_ERR_OK;

    map <int, int>::iterator channel_Iter;

    channel_Iter = wifiChannels.find(freq);
    if( channel_Iter!=wifiChannels.end() )
    {
        *channel = channel_Iter->second;
    }
    else
    {
        *channel = (int)NA_NUMBER;	//ret = TM_ERR_WIFI_FREQ_DOES_NOT_EXIST;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_WiFiConvertChannelToFrequency(int channel, int *freq)
{
    TM_RETURN ret = TM_ERR_OK;

    map <int, int>::iterator freq_Iter;

    freq_Iter = wifiChannels.begin();
    if( freq_Iter!=wifiChannels.end() )
    {
        while( freq_Iter!=wifiChannels.end() )
        {
            if ( channel==freq_Iter->second )
            {
                *freq = freq_Iter->first;
                ret = TM_ERR_OK;
                break;
            }
            else
            {                
                // keep searching...
                freq_Iter++;
                ret = TM_ERR_WIFI_FREQ_DOES_NOT_EXIST;
            }
        }
    }
    else
    {
        ret = TM_ERR_WIFI_FREQ_DOES_NOT_EXIST;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_WiFiConvertDataRateNameToIndex(TM_STR name, int* index)
{
    TM_RETURN ret = TM_ERR_OK;

    map <string, int>::iterator dataRate_Iter;

    dataRate_Iter = wifiDataRates.find(name);
    if( dataRate_Iter!=wifiDataRates.end() )
    {
        *index = dataRate_Iter->second;
    }
    else
    {
        ret = TM_ERR_WIFI_FREQ_DOES_NOT_EXIST;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_WiFiConvertIndexToDataRateName(int index, TM_STR name, int nameTextSize)
{
    TM_RETURN ret = TM_ERR_OK;
    bool keyWordFound = false;

    map <string, int>::iterator dataRate_Iter;

    dataRate_Iter = wifiDataRates.begin();
    while( dataRate_Iter != wifiDataRates.end() )
    {
        if (index == dataRate_Iter->second)
        {
            strcpy_s(name, nameTextSize, dataRate_Iter->first.c_str()); 
            keyWordFound = true;
            break;
        }
        dataRate_Iter++;
    }
    
    if (keyWordFound == false)
    {
        ret = TM_ERR_WIFI_FREQ_DOES_NOT_EXIST;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_WiFiConvertDataRateNameToMbps(TM_STR name, double* dataRateMbps)
{
    TM_RETURN ret = TM_ERR_OK;

    map <string, double>::iterator dataRateMbps_Iter;

    dataRateMbps_Iter = wifiDataRatesMbps.find(name);
    if( dataRateMbps_Iter!=wifiDataRatesMbps.end() )
    {
        *dataRateMbps = dataRateMbps_Iter->second;
    }
    else
    {
        ret = TM_ERR_WIFI_FREQ_DOES_NOT_EXIST;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_WiFiConvertDataRateNameToIQ2010ExtIndex(TM_STR name, int* dataRateIndex)
{
    TM_RETURN ret = TM_ERR_OK;

    map <string, int>::iterator dataRateIQ2010Ext_Iter;

    dataRateIQ2010Ext_Iter = dataRateIQ2010Ext.find(name);
    if( dataRateIQ2010Ext_Iter!=dataRateIQ2010Ext.end() )
    {
        *dataRateIndex = dataRateIQ2010Ext_Iter->second;
    }
    else
    {
        ret = TM_ERR_WIFI_FREQ_DOES_NOT_EXIST;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_GetVersion(TM_STR paramValue, int bufferSize)
{
    TM_RETURN ret = TM_ERR_OK;;

    if( 0!=strlen(g_TestManager_Version) )	// TestManager Version not empty
    {
		strcpy_s((char*)paramValue, bufferSize, (char*)g_TestManager_Version);
    }
    else
    {
        ret = TM_ERR_PARAM_DOES_NOT_EXIST;
    }

    return ret;
}


// Tx and Rx Path Loss Table, Tx is [TECHNOLOGY_NUM][0], Rx is [TECHNOLOGY_NUM][1].
TM_API TM_RETURN __stdcall TM_UpdatePathLossAtFrequency(  TM_ID  technologyID,
														  int	 freqMHz,
														  double *pathLoss,
														  int	 pathLossCount,
														  int    indicatorTxRx)
{
    TM_RETURN ret = TM_ERR_OK;

    map <int, vector<double> >::iterator pathLoss_Iter;

    // Convert the pathLoss array to vector
    vector<double> vectorPathLoss;
    for(int i=0; i<pathLossCount; i++)
    {
        vectorPathLoss.push_back( pathLoss[i] );
    }

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        pathLoss_Iter = g_pathLossMap[technologyID][indicatorTxRx].find(freqMHz);
        if( pathLoss_Iter==g_pathLossMap[technologyID][indicatorTxRx].end() )
        {
            // Frequency does not exist in the table.  Add the frequency entry
            g_pathLossMap[technologyID][indicatorTxRx].insert( pair<int, vector<double> >(freqMHz, vectorPathLoss) );
        }
        else
        {
            // Frequency exists in the table.  Use the current pathLoss_Iter to update path loss
            pathLoss_Iter->second.clear();
            for( int i=0; i<(int)vectorPathLoss.size(); i++ )
            {
                pathLoss_Iter->second.push_back( vectorPathLoss[i] );
            }
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

// Tx and Rx Path Loss Table, Tx is [TECHNOLOGY_NUM][0], Rx is [TECHNOLOGY_NUM][1].
TM_API TM_RETURN __stdcall TM_GetPathLossAtFrequency(  TM_ID   technologyID,
													   int	   freqMHz,
													   double *pathLoss,
													   int     pathLossIndex,
													   int     indicatorTxRx)
{
    TM_RETURN ret = TM_ERR_OK;

    map <int, vector<double> >::iterator pathLoss_Iter;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        if( 0==g_pathLossMap[technologyID][indicatorTxRx].size() )
        {
            // No entries in the path loss table
           *pathLoss = NA_NUMBER;
            ret = TM_ERR_NO_VALUE_DEFINED;
        }
        else if( 1==g_pathLossMap[technologyID][indicatorTxRx].size() )
        {
            // Only one frequency in the path loss table.  Apply it to any specified frequency
            pathLoss_Iter = g_pathLossMap[technologyID][indicatorTxRx].begin();
            if( pathLossIndex>=(int)pathLoss_Iter->second.size() )
            {
                *pathLoss = NA_NUMBER;
                ret = TM_ERR_NO_VALUE_DEFINED;
            }
            else
            {
                *pathLoss = pathLoss_Iter->second[pathLossIndex];
            }
        }
        else
        {
            pathLoss_Iter = g_pathLossMap[technologyID][indicatorTxRx].find(freqMHz);
            if( pathLoss_Iter==g_pathLossMap[technologyID][indicatorTxRx].end() )
            {
                // Frequency does not exist in the table.  Use interpolation
                pair< map<int,vector<double> >::iterator, map<int,vector<double> >::iterator > pr;
                pr = g_pathLossMap[technologyID][indicatorTxRx].equal_range( freqMHz );
                //printf("first=%d, second=%d\n", pr.first->first, pr.second->first);
                if( pr.first==g_pathLossMap[technologyID][indicatorTxRx].end() )
                {
                    // The specified frequency succeeded the last element.  Apply the last item to any specified frequency
                    pathLoss_Iter = g_pathLossMap[technologyID][indicatorTxRx].end();
                    pathLoss_Iter--;
                    if( pathLossIndex>=(int)pathLoss_Iter->second.size() )
                    {
                        *pathLoss = NA_NUMBER;
                        ret = TM_ERR_NO_VALUE_DEFINED;
                    }
                    else
                    {
                        *pathLoss = pathLoss_Iter->second[pathLossIndex];
                    }
                }
                else if( pr.first==g_pathLossMap[technologyID][indicatorTxRx].begin() )
                {
                    // The specified frequency is less than the first item.  Apply the first item to any specified frequency
                    pathLoss_Iter = g_pathLossMap[technologyID][indicatorTxRx].begin();
                    if( pathLossIndex>=(int)pathLoss_Iter->second.size() )
                    {
                        *pathLoss = NA_NUMBER;
                        ret = TM_ERR_NO_VALUE_DEFINED;
                    }
                    else
                    {
                        *pathLoss = pathLoss_Iter->second[pathLossIndex];
                    }
                }
                else
                {
                    // Interpolate
                    pr.second = pr.first;
                    pr.first--;
                    if( (pathLossIndex < (int)pr.first->second.size()) && (pathLossIndex < (int)pr.second->second.size()) )
                    {
                        int x1,x2;
                        double y1,y2;
                        x1 = pr.first->first;
                        x2 = pr.second->first;

                        y1 = pr.first->second[pathLossIndex];
                        y2 = pr.second->second[pathLossIndex];

                        *pathLoss = (y2-y1) * (freqMHz-x1) / (x2-x1) + y1;
                    }
                    else
                    {
                        // Although two frequencies are found for interpolation, pathLossIndex succeeds the path loss count
                        *pathLoss = NA_NUMBER;
                        ret = TM_ERR_NO_VALUE_DEFINED;
                    }
                }
            }
            else
            {
                // Frequency exists in the table.
                if( pathLossIndex>=(int)pathLoss_Iter->second.size() )
                {
                    *pathLoss = NA_NUMBER;
                    ret = TM_ERR_NO_VALUE_DEFINED;
                }
                else
                {
                    *pathLoss = pathLoss_Iter->second[pathLossIndex];
                }
            }
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

typedef struct tagExtension
{
    string dllName;
    map<string, string> keywordPair;
} TEST_EXTENSION;

void ProcessExtensions(TM_ID technologyID, TM_STR technologyDllFileName)
{
    
    TEST_EXTENSION          extensionSingle;
    vector<TEST_EXTENSION>  extensionList;
    extensionList.clear();
    
    
    // Step 1: check if <technologyDllFileName>_Ext.conf exists or not
    string confFileName(technologyDllFileName);

    string::size_type posDot = confFileName.find_last_of(".");
    if( string::npos != posDot )
    {
#ifdef WIN32
        confFileName = confFileName.substr(0, posDot) + "_Ext.conf";
#else
		confFileName = confFileName.substr(3, posDot - 3) + "_Ext.conf";
#endif
    }
    else
    {
        confFileName += "_Ext.conf";
    }

    // Step 2: Parse the .conf file
    FILE *confFile=NULL;
    fopen_s(&confFile, confFileName.c_str(), "rt");

    if( NULL!=confFile )
    {

        string line;
        char buffer[MAX_BUFFER_SIZE] = {'\0'};
        fgets(buffer, MAX_BUFFER_SIZE, confFile);
        line = buffer;
        //The very first line has to contain "<DLL_name> Extension"
        string firstLine(technologyDllFileName);
        string::size_type posBackSlash = firstLine.find_last_of("\\");

        if( (string::npos != posDot) &&
            (string::npos != posBackSlash) &&
            (posDot > posBackSlash) )
        {
            firstLine = firstLine.substr(posBackSlash+1, posDot-posBackSlash-1);
        }
        else if( (string::npos == posBackSlash) &&
                 (string::npos != posDot) )
        {
            firstLine = firstLine.substr(0, posDot);
        }
        else
        {
            // do nothing
        }
        firstLine = " Extension";
        if( string::npos != line.find(firstLine) )
        {
            // valid .conf file
            while( !feof( confFile ) )
            {
                fgets(buffer, MAX_BUFFER_SIZE, confFile);
                line = buffer;
                if( 0==line.size() || string::npos!=line.find("#") || string::npos!=line.find("//"))
                {
                    // skip blank and comment lines
                    continue;
                } 
                else if( string::npos != line.find("[") && string::npos != line.find("]") )
                {
                    // a line specifying DLL
                    extensionSingle.dllName = line.substr(line.find("[")+1, line.find("]")-line.find("[")-5);
#ifdef WIN32
					extensionSingle.dllName = extensionSingle.dllName + ".DLL";
#else
					extensionSingle.dllName = "lib" + extensionSingle.dllName + ".dylib";
#endif
                    extensionSingle.keywordPair.clear();
                    extensionList.push_back( extensionSingle );
                }
                else if( string::npos != line.find("=") )
                {
                    string key, value;
                    key = line.substr(0, line.find("="));
                    value = line.substr(line.find("=")+1, line.length()-line.find("=")-1);    //there is \r at the end

                    Trim(key);
                    Trim(value);

                    extensionList[extensionList.size()-1].keywordPair.insert( pair<string,string>(key, value) );
                    //extensionSingle.keywordPair.insert( pair<string,string>(key, value) );
                }
                else
                {
                    // don't know how to part it. skip it.
                }
            }
        }
        else
        {
            // invalid .conf file
        }
        fclose(confFile);
    }
    else
    {
        // .conf cannot be opened.
    }

    // Step 3: Load the DLL and get function entries
    if( extensionList.size() > 0 )
    {
        map<string,string>::iterator extension_Iter;
        map <string, TM_CALLBACK>::iterator function_Iter;
        for( int i=0; i<(int)extensionList.size(); i++ )
        {
            HMODULE dllHandle = ::LoadLibraryA( extensionList[i].dllName.c_str() );
            if( NULL!=dllHandle )
            {
                technologyFunction function;

                extension_Iter = extensionList[i].keywordPair.begin();
                while( extension_Iter != extensionList[i].keywordPair.end() )
                {	                            
					/* Support "disable" function for each Test Item by config setting 
					   so as to NOT show unsupported items on the IQWizard+ UI.
					   e.g. "TX_MULTI_VERIFICATION = disable" in xxx_Ext.conf */ /* #LPTW# cfy,-2010/05/25- */
					if (!strcmp(extension_Iter->second.c_str(), "disabled"))
					{
						if (g_testFunctions[technologyID].find( extension_Iter->first) != g_testFunctions[technologyID].end())
						{
							// remove the function from list
							g_testFunctions[technologyID].erase(extension_Iter->first);
							LOGGER_Write_Ext(LOG_IQLITE_TM, g_tmLoggerID[technologyID], LOGGER_INFORMATION, "[ TM ] function [%s] is disabled.\n", extension_Iter->first.c_str());
						}
						else
						{
							// could not find the function in the specified DLL.  Skip it
							LOGGER_Write_Ext(LOG_IQLITE_TM, g_tmLoggerID[technologyID], LOGGER_WARNING, "[ TM ] function [%s] is not found.\n", extension_Iter->first.c_str());
						}
					}
					else
					{
                    function = (technologyFunction)::GetProcAddress( dllHandle, extension_Iter->second.c_str() );
                    if( NULL!=function )
                    {
                        function_Iter = g_testFunctions[technologyID].find( extension_Iter->first );
                        if( function_Iter==g_testFunctions[technologyID].end() )
                        {
                            // Keyword not found.  Insert a new one
                            TM_CALLBACK callBack;
                            callBack.pointerToFunction = function;
                            g_testFunctions[technologyID].insert( functionPair(extension_Iter->first, callBack) );
						  }
                        else
                        {
                            // Keyword found.  replace the callback function
                            function_Iter->second.pointerToFunction = function;
                        }
						  LOGGER_Write_Ext(LOG_IQLITE_TM, g_tmLoggerID[technologyID], LOGGER_INFORMATION, "[ TM ] function [%s] is inserted.\n", extension_Iter->first.c_str());
                    }
                    else
                    {
                        // could not find the function in the specified DLL.  Skip it
					  }
                   }
                   extension_Iter++;
                }
            }
            else
            {
                //failed to load the externsion DLL
            }
        }
    }

    return;
}

// Tx and Rx Path Loss Table, Tx is [TECHNOLOGY_NUM][0], Rx is [TECHNOLOGY_NUM][1].
TM_API TM_RETURN __stdcall TM_UpdatePathLossByFile(TM_ID technologyID, TM_STR pathLossFileName, int indicatorTxRx)
{
    TM_RETURN ret = TM_ERR_OK;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
		if ( indicatorTxRx>=MAX_PATHLOSS_TABLE_NUM )
		{
			indicatorTxRx = 0;	// If over the range, (default) set to Tx table.
		}
		else
		{
			// do nothing
		}

        FILE *pathLossFile=NULL;
        fopen_s(&pathLossFile, pathLossFileName, "rt");

        if( NULL!=pathLossFile )
        {
            // clear the current path loss table
            g_pathLossMap[technologyID][indicatorTxRx].clear();

            char buffer[MAX_BUFFER_SIZE] = {'\0'};
            string line;
            vector<string> splits;
            while( !feof( pathLossFile ) )
            {
                fgets(buffer, MAX_BUFFER_SIZE, pathLossFile);
                line = buffer;
                Trim( line );
                if( 0==line.size() || string::npos!=line.find("#") || string::npos!=line.find("//"))
                {
                    // skip blank and comment lines
                    continue;
                } 
                else
                {
                    splits.clear();
                    SplitString(line, splits, ",");
                    if( splits.size()>=2 )
                    {
                        // a valid line at least contains one frequency and one path loss
                        vector<double> pathLoss;
                        pathLoss.clear();
                        for( int i=1; i<(int)splits.size(); i++)
                        {
                            pathLoss.push_back( atof(splits[i].c_str()) );
                        }
                        g_pathLossMap[technologyID][indicatorTxRx].insert( pair<int,vector<double> >( atoi(splits[0].c_str()), pathLoss ) );
                    }
                }
            }
            fclose(pathLossFile);
        }
        else
        {
            // Failed to open the path loss file
            ret = TM_ERR_FAILED_TO_OPEN_FILE;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_GetDutInfo(TM_STR dutSerialNumber, int dutSerialNumberBufferSize,
                                         TM_STR dutManufacturer, int dutManufacturerBufferSize,
                                         TM_STR dutModel, int dutModelBufferSize,
                                         TM_STR dutRevision, int dutRevisionBufferSize,
                                         TM_STR dutDescription, int dutDescriptionBufferSize)
{
    TM_RETURN ret = TM_ERR_OK;

    if( g_dutInfo.sSerialNumber.length() != 0 || 
        g_dutInfo.sManufacturer.length() != 0 ||
        g_dutInfo.sModel.length() != 0 ||
        g_dutInfo.sRevision.length() != 0 ||
        g_dutInfo.sDescription.length() != 0 )	// Dut Info not empty
    {
		strcpy_s((char*)dutSerialNumber, dutSerialNumberBufferSize, g_dutInfo.sSerialNumber.c_str());
		strcpy_s((char*)dutManufacturer, dutManufacturerBufferSize, g_dutInfo.sManufacturer.c_str());
		strcpy_s((char*)dutModel, dutModelBufferSize, g_dutInfo.sModel.c_str());
		strcpy_s((char*)dutRevision, dutRevisionBufferSize, g_dutInfo.sRevision.c_str());
		strcpy_s((char*)dutDescription, dutDescriptionBufferSize, g_dutInfo.sDescription.c_str());
    }
    else
    {
        ret = TM_ERR_PARAM_DOES_NOT_EXIST;
    }

    return ret;
}

TM_API TM_RETURN __stdcall TM_SetDutInfo(TM_STR dutSerialNumber,
                                         TM_STR dutManufacturer,
                                         TM_STR dutModel,
                                         TM_STR dutRevision,
                                         TM_STR dutDescription)
{
    TM_RETURN ret = TM_ERR_OK;

	g_dutInfo.sSerialNumber.assign( dutSerialNumber );
	g_dutInfo.sManufacturer.assign( dutManufacturer );
	g_dutInfo.sModel.assign( dutModel );
	g_dutInfo.sRevision.assign( dutRevision );
	g_dutInfo.sDescription.assign( dutDescription );

    return ret;
}


TM_API TM_RETURN __stdcall TM_CheckMultiWaveformIndexMap(bool *mapIsEmpty)
{
    TM_RETURN ret = TM_ERR_OK;

	if ( 0>=(int)g_multiWaveformIndexMap.size() )
	{
		*mapIsEmpty = true;
	}
	else
	{
		*mapIsEmpty = false;
	}
	
    return ret;
}

TM_API TM_RETURN __stdcall TM_ClearMultiWaveformIndexMap()
{
    TM_RETURN ret = TM_ERR_OK;

	g_multiWaveformIndexMap.clear();

    return ret;
}

TM_API TM_RETURN __stdcall TM_AddMultiWaveformWithIndex(TM_STR waveformName, int index)
{
    TM_RETURN ret = TM_ERR_OK;
						 
	g_multiWaveformIndexMap.insert( pair<string,int>(waveformName, index) );
	
    return ret;
}

TM_API TM_RETURN __stdcall TM_GetMultiWaveformIndex(TM_STR waveformName, int *index)
{
    TM_RETURN ret = TM_ERR_OK;

	map <string, int>::iterator mapIterator;

    // Searching the keyword in g_multiWaveformIndexMap
	mapIterator = g_multiWaveformIndexMap.find( waveformName );
    if( mapIterator!=g_multiWaveformIndexMap.end() )
    {
		*index = mapIterator->second;
	}
	else
    {
        ret = TM_ERR_PARAM_DOES_NOT_EXIST;
    }
	
    return ret;
}

TM_API TM_RETURN __stdcall TM_GetSeqMeasureResults(const TM_STR dataRate, int mpsMeasureType)
{
	TM_RETURN ret = TM_ERR_OK;

	return ret;
}

TM_API TM_RETURN __stdcall TM_ApplyDataRateMeasTypeReturn(TM_ID technologyID, const TM_STR dataRate, TM_SEQ_MEAS_TYPE measType, const TM_STR paramName, const double *paramValue, const int arraySize, const TM_STR unit)
{
    TM_RETURN ret = TM_ERR_OK;
    SEQ_PARAM_RETURN_VALUES seqReturnValues;
    SEQ_MEAS_PARAM_RETURN seqMeasResults;
    SEQ_MEAS_TYPE_RESULTS seqMeasTypeResults;
    map<string, SEQ_MEAS_TYPE_RESULTS>::iterator seqDataRateResults_Iter;
    SEQ_MEAS_TYPE_RESULTS::iterator seqMeasTypeResults_Iter;
    
    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {  
        if( arraySize>0 )
        {
            seqReturnValues.unit = unit;
            for( int i=0; i<arraySize; i++)
            {
                seqReturnValues.values.push_back( paramValue[i] );
            }
        }

        seqDataRateResults_Iter = g_seqDataRateResults[technologyID].find(dataRate);
        if(seqDataRateResults_Iter != g_seqDataRateResults[technologyID].end())
        {
            seqMeasTypeResults_Iter = seqDataRateResults_Iter->second.find(measType);
            if (seqMeasTypeResults_Iter != seqDataRateResults_Iter->second.end())
            {
                seqMeasTypeResults_Iter->second.insert(seqMeasParamReturnPair(paramName, seqReturnValues));
            }
            else
            {
                seqMeasResults.insert(seqMeasParamReturnPair(paramName, seqReturnValues));
                seqDataRateResults_Iter->second.insert(seqMeasTypeResultsPair(measType, seqMeasResults));
            }
        }
        else
        {
            seqMeasResults.insert(seqMeasParamReturnPair(paramName, seqReturnValues));
            seqMeasTypeResults.insert(seqMeasTypeResultsPair(measType, seqMeasResults));
            g_seqDataRateResults[technologyID].insert(pair<string, SEQ_MEAS_TYPE_RESULTS>(dataRate,seqMeasTypeResults));
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }
    return ret;
}


TM_API TM_RETURN __stdcall TM_GetSeqDataRateReturn(TM_ID technologyID, int *numOfDataRate, TM_STR *dataRateList)
{
    TM_RETURN ret = TM_ERR_OK;
    map<string, SEQ_MEAS_TYPE_RESULTS>::iterator seqDataRateResults_Iter = g_seqDataRateResults[technologyID].begin();
    
    *numOfDataRate = (int)g_seqDataRateResults[technologyID].size();

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {
        while(seqDataRateResults_Iter != g_seqDataRateResults[technologyID].end())
        {
            *dataRateList = (TM_STR)seqDataRateResults_Iter->first.c_str();
            dataRateList ++;
            seqDataRateResults_Iter++;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }
    return ret;
}

TM_API TM_RETURN __stdcall TM_GetSeqOneDataRateMeasTypeReturn(TM_ID technologyID, const TM_STR dataRate, int *numOfMeasType, TM_SEQ_MEAS_TYPE *measTypeList)
{
    TM_RETURN ret = TM_ERR_OK;
    map<string, SEQ_MEAS_TYPE_RESULTS>::iterator seqDataRateResults_Iter;
    SEQ_MEAS_TYPE_RESULTS::iterator seqMeasTypeResults_Iter;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {  

        seqDataRateResults_Iter = g_seqDataRateResults[technologyID].find(dataRate);
        if(seqDataRateResults_Iter != g_seqDataRateResults[technologyID].end())
        {
            seqMeasTypeResults_Iter = seqDataRateResults_Iter->second.begin();
            *numOfMeasType = (int)seqDataRateResults_Iter->second.size();

            while(seqMeasTypeResults_Iter != seqDataRateResults_Iter->second.end())
            {
                *measTypeList = (TM_SEQ_MEAS_TYPE)seqMeasTypeResults_Iter->first;
                measTypeList++;
                seqMeasTypeResults_Iter++;
            }
        }
        else
        {
             ret = TM_ERR_DATARATE_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }
    return ret;
}

TM_API TM_RETURN __stdcall TM_GetSeqOneMeasTypeReturn(TM_ID technologyID, const TM_STR dataRate, TM_SEQ_MEAS_TYPE measType, TM_SEQ_ONE_MEAS_TYPE_RESULTS *seqOneMeasTypeResults)
{
    TM_RETURN ret = TM_ERR_OK;
    map<string, SEQ_MEAS_TYPE_RESULTS>::iterator seqDataRateResults_Iter;
    SEQ_MEAS_TYPE_RESULTS::iterator seqMeasTypeResults_Iter;
    SEQ_MEAS_PARAM_RETURN::iterator seqMeasParamReturn_Iter;
	seqOneMeasTypeResults->numOfResults = 0;

    if( technologyID>-1 && technologyID<MAX_TECHNOLOGY_NUM )
    {  
        seqDataRateResults_Iter = g_seqDataRateResults[technologyID].find(dataRate);
        if(seqDataRateResults_Iter != g_seqDataRateResults[technologyID].end())
        {
            seqMeasTypeResults_Iter = seqDataRateResults_Iter->second.find(measType);
            if (seqMeasTypeResults_Iter != seqDataRateResults_Iter->second.end())
            {
                seqMeasParamReturn_Iter = seqMeasTypeResults_Iter->second.begin();
                int n = 0;
                while(seqMeasParamReturn_Iter != seqMeasTypeResults_Iter->second.end())
                {
                    seqOneMeasTypeResults->numOfResults++;
                    seqOneMeasTypeResults->results[n].numOfValues = (int)seqMeasParamReturn_Iter->second.values.size();
                    strcpy_s(seqOneMeasTypeResults->results[n].paramName, MAX_RESULT_NAME, seqMeasParamReturn_Iter->first.c_str());
                    strcpy_s(seqOneMeasTypeResults->results[n].unit, MAX_RESULT_NAME, seqMeasParamReturn_Iter->second.unit.c_str());
                    for (int i = 0; i < seqOneMeasTypeResults->results[n].numOfValues; i++)
                    {
                        seqOneMeasTypeResults->results[n].values[i] = seqMeasParamReturn_Iter->second.values[i];
                    }
                    n++;
                    seqMeasParamReturn_Iter++;
                }
            }
            else
            {
                ret = TM_ERR_MEAS_TYPE_DOES_NOT_EXIST;
            }
        }
        else
        {
            ret = TM_ERR_DATARATE_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = TM_ERR_INVALID_TECHNOLOGY_ID;;
    }
    return ret;
}

void LogTestInputParameters( TM_ID technologyID, const TM_STR functionKeyword)
{
    int dummyValue;

    if( TM_ERR_OK != TM_GetIntegerParameter( technologyID, "QUERY_INPUT", &dummyValue )
        && TM_ERR_OK != TM_GetIntegerParameter( technologyID, "QUERY_RETURN", &dummyValue )
        && technologyID != IQREPORT )
    {
        ::TM_ClearParameters( IQREPORT );
        ::TM_AddIntegerParameter( IQREPORT, "TECHNOLOGY_ID", technologyID );
        ::TM_AddStringParameter( IQREPORT, "FUNCTION_KEYWORD", functionKeyword );
        ::TM_Run( IQREPORT, "REPORT_INPUT" );
    }
}

void LogTestResults( TM_ID technologyID, const TM_STR functionKeyword )
{
    int dummyValue;

    if( TM_ERR_OK != TM_GetIntegerParameter( technologyID, "QUERY_INPUT", &dummyValue )
        && TM_ERR_OK != TM_GetIntegerParameter( technologyID, "QUERY_RETURN", &dummyValue )
        && technologyID != IQREPORT )
    {
        ::TM_ClearParameters( IQREPORT );
        ::TM_AddIntegerParameter( IQREPORT, "TECHNOLOGY_ID", technologyID );
        ::TM_AddStringParameter( IQREPORT, "FUNCTION_KEYWORD", functionKeyword );
        ::TM_Run( IQREPORT, "REPORT_RESULT" );

    }
}
