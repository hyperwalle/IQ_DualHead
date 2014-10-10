// vDUT.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
//Move to stdafx.h
//#include "lp_windows.h"
#include "vDUT.h"
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
#include <stdlib.h>
#include <string>
#include <map>
#include <vector>

typedef struct vDUT_CallBack
{
    int (*pointerToFunction) (void);
} vDUT_CALLBACK;

typedef int (*dutRegister) (void);

//#define MAX_TECHNOLOGIES_COUNT  5
/*
* Each DUT needs to belong to one technology.
* One technology can only have one DUT, or none.
* The following technologies exist:
*    - WIFI
*    - BT
*    - WIMAX
*    - GPS
*    - FM
*/
typedef struct vDUT_Technology
{
    vDUT_STR technology;
} vDUT_TECHNOLOGY;

typedef struct vDUT_Description
{
    vDUT_ID  dutID;
    int      dutFunctionIndex;
    vDUT_STR dutName;
    vDUT_STR dutDescription;
    vDUT_STR dutDriverVersion;
    vDUT_STR dutFirmwareVersion;
    vDUT_DUT_TYPE dutType;
} vDUT_INFO;

// Global variables
vDUT_ID      g_lastDutId = 0;
int		     g_byPassDut = 1;
//const char  *g_vDUT_Version = "1.6.0 (2010-09-17)\n";
const char  *g_vDUT_Version = VDUT_VERSION;  // version defined in import\Version.h 

using namespace std;


map <string, vDUT_CALLBACK > dutFunctions[MAX_TECHNOLOGIES_COUNT];
typedef pair<string , vDUT_CALLBACK> functionPair;

map <string, vDUT_INFO> technologies;
map <string, vDUT_INFO>::iterator technology_Iter;
typedef pair<string , vDUT_INFO> technologyPair;

map <string, int> intParams[MAX_TECHNOLOGIES_COUNT];
typedef pair<string , int> intParamPair;

map <string, double> doubleParams[MAX_TECHNOLOGIES_COUNT];
typedef pair<string , double> doubleParamPair;

map <string, string> stringParams[MAX_TECHNOLOGIES_COUNT];
typedef pair<string , string> stringParamPair;

map <string, int> intReturns[MAX_TECHNOLOGIES_COUNT];
typedef pair<string , int> intReturnPair;

map <string, double> doubleReturns[MAX_TECHNOLOGIES_COUNT];
typedef pair<string , double> doubleReturnPair;

map <string, string> stringReturns[MAX_TECHNOLOGIES_COUNT];
typedef pair<string , string> stringReturnPair;

map <int, int> wifiChannels;
map <string, vDUT_DATA_RATE> wifiDataRates;
map <string, double> wifiDataRatesMbps;

map <string, vector<double> > g_arrayDoubleReturns[MAX_TECHNOLOGIES_COUNT];
typedef pair<string , vector<double> > arrayDoubleReturnPair;
map <string, vector<double> >::iterator g_arrayDoubleReturn_Iter[MAX_TECHNOLOGIES_COUNT];

vector<SEQ_TX_CONFIG> sequenceTxParams[MAX_TECHNOLOGIES_COUNT];
vector<SEQ_RX_CONFIG> sequenceRxParams[MAX_TECHNOLOGIES_COUNT];
vector<SEQ_TX_ONE_DATARATE_CONFIG> sequenceTxOneDataRateParams[MAX_TECHNOLOGIES_COUNT];

/*
 * Each technology will have to have corresponding DUT control DLL to be loaded.
 * This variable needs to be global, because vDUT_RegisterDutDll() load the DLL,
 * and vDUT_UnregisterDutDll() frees the handle.
 */
HMODULE g_dutDllHandle[MAX_TECHNOLOGIES_COUNT];

vDUT_ID	g_vDutTimerID [MAX_TECHNOLOGIES_COUNT];
vDUT_ID	g_vDutLoggerID[MAX_TECHNOLOGIES_COUNT];

//Saved Dut obj pointer
int DutObjConnected = FALSE;
void *DutObj = NULL;
map <string, string> miscStringValues;
map <string, int> miscIntValues;

int Initialize_vDUT();
//void Initialize_vDUT();
void Free_vDut_Memory();

#ifdef WIN32
	BOOL APIENTRY DllMain( HMODULE hModule,
						  DWORD  ul_reason_for_call,
						  LPVOID lpReserved
						  )
	{
		switch (ul_reason_for_call)
		{
		case DLL_PROCESS_ATTACH:
			g_lastDutId = 0;
			Initialize_vDUT();
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			Free_vDut_Memory();
			break;
		}
		return TRUE;
	}

#else
	int vdut_init = Initialize_vDUT();
#endif


#ifdef _MANAGED
#pragma managed(pop)
#endif

vDUT_API char* vDUT_GetErrorString(vDUT_RETURN errorNum)
{
    char *ret="";

    switch(errorNum)
    {
    case vDUT_ERR_OK: ret="No errors";
    case vDUT_ERR_TECHNOLOGY_DOES_NOT_EXIST: 
        ret="Specified technology has not been defined yet";
        break;
    case vDUT_ERR_TECHNOLOGY_HAS_BEEN_REGISTERED: 
        ret="The specified technology Another has been registgered by another DUT";
        break;
    case vDUT_ERR_FUNCTION_NOT_SUPPORTED:       
        ret="DUT does not support the specified function";
        break;
    case vDUT_ERR_FUNCTION_NOT_DEFINED:
        ret="The specified function has not been defined yet";
        break;
    case vDUT_ERR_PARAM_DOES_NOT_EXIST:
        ret="The specified parameter does not exist";
        break;
    case vDUT_ERR_INVALID_DUT_ID:
        ret="Invalid DUT ID";
        break;
    case vDUT_ERR_DUT_FUNCTION_ERROR:
        ret = "DUT function had errors";
        break;
	case vDUT_ERR_WIFI_FREQ_DOES_NOT_EXIST:
		ret = "The specified WiFi frequency does not exist";
		break;
    }

    return ret;
}

void Free_vDut_Memory()
{
	technologies.clear();
	wifiChannels.clear();
	wifiDataRates.clear();

	for (int i=0;i<MAX_TECHNOLOGIES_COUNT;i++)
	{
		dutFunctions[i].clear();
		intParams[i].clear();
		doubleParams[i].clear();
		stringParams[i].clear();
		intReturns[i].clear();
		doubleReturns[i].clear();
		stringReturns[i].clear();
	}

}

/*! @defgroup wifi_function_names Pre-defined WIFI Function Names
 *
 * The following function names have been defined for WIFI technology:
 *      - INSERT_DUT 
 *      - REMOVE_DUT
 *      - INITIALIZE_DUT
 *      - WRITE_MAC_ADDRESS
 *      - READ_MAC_ADDRESS
 *      - READ_EEPROM
 *      - WRITE_EEPROM
 *      - RF_SET_FREQ
 *      - TX_PRE_TX
 *      - TX_SET_BAND
 *      - TX_SET_DATA_RATE
 *      - TX_SET_ANTENNA
 *      - TX_SET_MISC1
 *      - TX_SET_MISC2
 *      - TX_SET_MISC3
 *      - TX_START
 *      - TX_STOP
 *      - RX_PRE_RX
 *      - RX_SET_BAND
 *      - RX_SET_FREQ
 *      - RX_SET_DATA_RATE
 *      - RX_SET_ANTENNA
 *      - RX_SET_FILTER
 *      - RX_GET_STATS
 *      - RX_SET_MISC1
 *      - RX_SET_MISC2
 *      - RX_SET_MISC3
 */

int Initialize_vDUT()
{
    vDUT_CALLBACK callBack;

    for(int i=0; i<MAX_TECHNOLOGIES_COUNT; i++)
    {
        dutFunctions[i].clear();
		g_vDutTimerID[i]	=  -1;
		g_vDutLoggerID[i]	=  -1;
    }

    callBack.pointerToFunction = NULL;

    technologies.clear();
    vDUT_INFO dutInfo;
    dutInfo.dutID              = -1;
    dutInfo.dutName            = "";
    dutInfo.dutDescription     = "";
    dutInfo.dutDriverVersion   = "";
    dutInfo.dutFirmwareVersion = "";

    int dfIndex = 0;
    //WIFI
    dutInfo.dutFunctionIndex   = dfIndex;
    g_dutDllHandle[dfIndex]    = NULL;
    technologies.insert( technologyPair("WIFI", dutInfo) );
    dutFunctions[dfIndex].clear();
    dutFunctions[dfIndex].insert( functionPair("INSERT_DUT",          callBack) );
    dutFunctions[dfIndex].insert( functionPair("INITIALIZE_DUT",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("REMOVE_DUT",          callBack) );
	dutFunctions[dfIndex].insert( functionPair("CLEAR_DUT_STATUS",    callBack) );

	dutFunctions[dfIndex].insert( functionPair("GET_SERIAL_NUMBER",   callBack) );
	dutFunctions[dfIndex].insert( functionPair("READ_SERIAL_NUMBER",  callBack) );
	dutFunctions[dfIndex].insert( functionPair("WRITE_SERIAL_NUMBER", callBack) );

	dutFunctions[dfIndex].insert( functionPair("READ_EEPROM",         callBack) );
    dutFunctions[dfIndex].insert( functionPair("WRITE_EEPROM",        callBack) );

    dutFunctions[dfIndex].insert( functionPair("FINALIZE_EEPROM",     callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUMP_EEPROM",		  callBack) );


    dutFunctions[dfIndex].insert( functionPair("WRITE_MAC_ADDRESS",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("READ_MAC_ADDRESS",    callBack) );
	dutFunctions[dfIndex].insert( functionPair("READ_OTP_MAC_ADDRESS",callBack) );

	dutFunctions[dfIndex].insert( functionPair("WRITE_BB_REGISTER",   callBack) );
	dutFunctions[dfIndex].insert( functionPair("READ_BB_REGISTER",    callBack) );

	dutFunctions[dfIndex].insert( functionPair("WRITE_RF_REGISTER",   callBack) );
	dutFunctions[dfIndex].insert( functionPair("READ_RF_REGISTER",    callBack) );

	dutFunctions[dfIndex].insert( functionPair("WRITE_MAC_REGISTER",  callBack) );
	dutFunctions[dfIndex].insert( functionPair("READ_MAC_REGISTER",   callBack) );

	dutFunctions[dfIndex].insert( functionPair("WRITE_SOC_REGISTER",  callBack) );
	dutFunctions[dfIndex].insert( functionPair("READ_SOC_REGISTER",   callBack) );    

    dutFunctions[dfIndex].insert( functionPair("WRITE_MEMORY",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("READ_MEMORY",         callBack) );

    dutFunctions[dfIndex].insert( functionPair("GET_SLEEP_MODE",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("SET_SLEEP_MODE",      callBack) );

    dutFunctions[dfIndex].insert( functionPair("RF_SET_FREQ",         callBack) );

    dutFunctions[dfIndex].insert( functionPair("TX_PRE_TX",           callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_BW",           callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_DATA_RATE",    callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_ANTENNA",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_CONTINUOUS",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_START",            callBack) ); // Used for DUT to transmit as target power  
    dutFunctions[dfIndex].insert( functionPair("TX_START_GAIN",       callBack) ); // Used for DUT to transmit as gainIndex
    dutFunctions[dfIndex].insert( functionPair("TX_STOP",             callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC1",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC2",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC3",        callBack) );
	dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC4",        callBack) );
	dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC5",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION1",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION2",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION3",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION4",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION5",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION6",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION7",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION8",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION9",   callBack) );

    dutFunctions[dfIndex].insert( functionPair("RX_PRE_RX",           callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_BW",           callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_DATA_RATE",    callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_ANTENNA",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_FILTER",       callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_CLEAR_STATS",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_GET_STATS",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_START",            callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_STOP",             callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_MISC1",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_MISC2",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_MISC3",        callBack) );
	dutFunctions[dfIndex].insert( functionPair("RX_CALIBRATION",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_NOISE",            callBack) ); // used for RX noise measurment

    dutFunctions[dfIndex].insert( functionPair("TX_CALIBRATION",     callBack) );     
    dutFunctions[dfIndex].insert( functionPair("START_TX_SEQ",   	 callBack) );
 
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION1",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION2",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION3",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION4",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION5",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION6",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION7",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION8",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION9",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("GET_CAL_SETTING",     callBack) );    
    dutFunctions[dfIndex].insert( functionPair("SET_CAL_MEASUREMENT", callBack) );         
	dutFunctions[dfIndex].insert( functionPair("GET_TX_TARGET_POWER", callBack) ); //used for asking tx target power from DUT based on channel and data rate
	dutFunctions[dfIndex].insert( functionPair("GET_TX_TARGET_POWER_MP", callBack) ); //used for asking tx multi rate target power from DUT
    dutFunctions[dfIndex].insert( functionPair("GET_RX_CAL_SETTING",  callBack) );    
    //dutFunctions[dfIndex].insert( functionPair("SET_RX_CAL_MEASUREMENT", callBack) );   
	dutFunctions[dfIndex].insert( functionPair("FUNCTION_FAILED",	  callBack) );
	dutFunctions[dfIndex].insert( functionPair("CALIBRATION_FAILED",  callBack) );

	//Modified from BD IQlite
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION1",  callBack) ); //used for DUT temperature reading
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION2",  callBack) ); //used for DUT misc commands
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION3",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION4",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION5",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION6",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION7",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION8",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION9",  callBack) );

    // Create the timer ID 
	TIMER_CreateTimer("WIFI_DUT", &g_vDutTimerID[dfIndex]);
    // Create the logger ID 
	LOGGER_CreateLogger("WIFI_DUT", &g_vDutLoggerID[dfIndex]);

    dfIndex++;
    if(dfIndex>MAX_TECHNOLOGIES_COUNT)
    {
        printf("Too many technologies! MAX_TECHNOLOGIES_COUNT needs to be increased!\n");
        exit(1);
    }

	//WIFI_11AC
    dutInfo.dutFunctionIndex   = dfIndex;
    g_dutDllHandle[dfIndex]    = NULL;
    technologies.insert( technologyPair("WIFI_11AC", dutInfo) );
    dutFunctions[dfIndex].clear();
    dutFunctions[dfIndex].insert( functionPair("INSERT_DUT",          callBack) );
    dutFunctions[dfIndex].insert( functionPair("INITIALIZE_DUT",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("REMOVE_DUT",          callBack) );

	dutFunctions[dfIndex].insert( functionPair("GET_SERIAL_NUMBER",   callBack) );
	dutFunctions[dfIndex].insert( functionPair("READ_SERIAL_NUMBER",  callBack) );
	dutFunctions[dfIndex].insert( functionPair("WRITE_SERIAL_NUMBER", callBack) );

	dutFunctions[dfIndex].insert( functionPair("READ_EEPROM",         callBack) );
    dutFunctions[dfIndex].insert( functionPair("WRITE_EEPROM",        callBack) );

    dutFunctions[dfIndex].insert( functionPair("FINALIZE_EEPROM",     callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUMP_EEPROM",		  callBack) );


    dutFunctions[dfIndex].insert( functionPair("WRITE_MAC_ADDRESS",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("READ_MAC_ADDRESS",    callBack) );

	dutFunctions[dfIndex].insert( functionPair("WRITE_BB_REGISTER",   callBack) );
	dutFunctions[dfIndex].insert( functionPair("READ_BB_REGISTER",    callBack) );

	dutFunctions[dfIndex].insert( functionPair("WRITE_RF_REGISTER",   callBack) );
	dutFunctions[dfIndex].insert( functionPair("READ_RF_REGISTER",    callBack) );

	dutFunctions[dfIndex].insert( functionPair("WRITE_MAC_REGISTER",  callBack) );
	dutFunctions[dfIndex].insert( functionPair("READ_MAC_REGISTER",   callBack) );

	dutFunctions[dfIndex].insert( functionPair("WRITE_SOC_REGISTER",  callBack) );
	dutFunctions[dfIndex].insert( functionPair("READ_SOC_REGISTER",   callBack) );    

    dutFunctions[dfIndex].insert( functionPair("WRITE_MEMORY",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("READ_MEMORY",         callBack) );

    dutFunctions[dfIndex].insert( functionPair("GET_SLEEP_MODE",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("SET_SLEEP_MODE",      callBack) );

    dutFunctions[dfIndex].insert( functionPair("RF_SET_FREQ",         callBack) );

    dutFunctions[dfIndex].insert( functionPair("TX_PRE_TX",           callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_BW",           callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_DATA_RATE",    callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_ANTENNA",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_CONTINUOUS",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_START",            callBack) );
	dutFunctions[dfIndex].insert( functionPair("TX_START_GAIN",       callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_STOP",             callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC1",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC2",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC3",        callBack) );
	dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC4",        callBack) );
	dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC5",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION1",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION2",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION3",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION4",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION5",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION6",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION7",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION8",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION9",   callBack) );

    dutFunctions[dfIndex].insert( functionPair("RX_PRE_RX",           callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_BW",           callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_DATA_RATE",    callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_ANTENNA",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_FILTER",       callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_CLEAR_STATS",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_GET_STATS",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_START",            callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_STOP",             callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_MISC1",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_MISC2",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_MISC3",        callBack) );
	dutFunctions[dfIndex].insert( functionPair("RX_CALIBRATION",      callBack) );

    dutFunctions[dfIndex].insert( functionPair("TX_CALIBRATION",     callBack) );     
    dutFunctions[dfIndex].insert( functionPair("START_TX_SEQ",   	 callBack) );
 
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION1",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION2",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION3",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION4",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION5",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION6",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION7",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION8",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION9",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("GET_CAL_SETTING",     callBack) );    
    dutFunctions[dfIndex].insert( functionPair("SET_CAL_MEASUREMENT", callBack) );         
 
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION1",  callBack) ); //used for DUT temperature reading
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION2",  callBack) ); //used for DUT misc commands
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION3",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION4",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION5",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION6",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION7",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION8",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION9",  callBack) );

    // Create the timer ID 
	TIMER_CreateTimer("WIFI_11AC_DUT", &g_vDutTimerID[dfIndex]);
    // Create the logger ID 
	LOGGER_CreateLogger("WIFI_11AC_DUT", &g_vDutLoggerID[dfIndex]);

    dfIndex++;
    if(dfIndex>MAX_TECHNOLOGIES_COUNT)
    {
        printf("Too many technologies! MAX_TECHNOLOGIES_COUNT needs to be increased!\n");
        exit(1);
    }

	//WIFI_11AC_MIMO
    dutInfo.dutFunctionIndex   = dfIndex;
    g_dutDllHandle[dfIndex]    = NULL;
    technologies.insert( technologyPair("WIFI_11AC_MIMO", dutInfo) );
    dutFunctions[dfIndex].clear();
    dutFunctions[dfIndex].insert( functionPair("INSERT_DUT",          callBack) );
    dutFunctions[dfIndex].insert( functionPair("INITIALIZE_DUT",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("REMOVE_DUT",          callBack) );

	dutFunctions[dfIndex].insert( functionPair("GET_SERIAL_NUMBER",   callBack) );
	dutFunctions[dfIndex].insert( functionPair("READ_SERIAL_NUMBER",  callBack) );
	dutFunctions[dfIndex].insert( functionPair("WRITE_SERIAL_NUMBER", callBack) );

	dutFunctions[dfIndex].insert( functionPair("READ_EEPROM",         callBack) );
    dutFunctions[dfIndex].insert( functionPair("WRITE_EEPROM",        callBack) );

    dutFunctions[dfIndex].insert( functionPair("FINALIZE_EEPROM",     callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUMP_EEPROM",		  callBack) );


    dutFunctions[dfIndex].insert( functionPair("WRITE_MAC_ADDRESS",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("READ_MAC_ADDRESS",    callBack) );

	dutFunctions[dfIndex].insert( functionPair("WRITE_BB_REGISTER",   callBack) );
	dutFunctions[dfIndex].insert( functionPair("READ_BB_REGISTER",    callBack) );

	dutFunctions[dfIndex].insert( functionPair("WRITE_RF_REGISTER",   callBack) );
	dutFunctions[dfIndex].insert( functionPair("READ_RF_REGISTER",    callBack) );

	dutFunctions[dfIndex].insert( functionPair("WRITE_MAC_REGISTER",  callBack) );
	dutFunctions[dfIndex].insert( functionPair("READ_MAC_REGISTER",   callBack) );

	dutFunctions[dfIndex].insert( functionPair("WRITE_SOC_REGISTER",  callBack) );
	dutFunctions[dfIndex].insert( functionPair("READ_SOC_REGISTER",   callBack) );    

    dutFunctions[dfIndex].insert( functionPair("WRITE_MEMORY",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("READ_MEMORY",         callBack) );

    dutFunctions[dfIndex].insert( functionPair("GET_SLEEP_MODE",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("SET_SLEEP_MODE",      callBack) );

    dutFunctions[dfIndex].insert( functionPair("RF_SET_FREQ",         callBack) );

    dutFunctions[dfIndex].insert( functionPair("TX_PRE_TX",           callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_BW",           callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_DATA_RATE",    callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_ANTENNA",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_CONTINUOUS",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_START",            callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_STOP",             callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC1",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC2",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC3",        callBack) );
	dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC4",        callBack) );
	dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC5",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION1",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION2",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION3",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION4",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION5",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION6",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION7",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION8",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_MISC_FUNCTION9",   callBack) );

    dutFunctions[dfIndex].insert( functionPair("RX_PRE_RX",           callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_BW",           callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_DATA_RATE",    callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_ANTENNA",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_FILTER",       callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_CLEAR_STATS",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_GET_STATS",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_START",            callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_STOP",             callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_MISC1",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_MISC2",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_MISC3",        callBack) );
	dutFunctions[dfIndex].insert( functionPair("RX_CALIBRATION",      callBack) );

    dutFunctions[dfIndex].insert( functionPair("TX_CALIBRATION",     callBack) );     
    dutFunctions[dfIndex].insert( functionPair("START_TX_SEQ",   	 callBack) );
 
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION1",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION2",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION3",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION4",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION5",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION6",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION7",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION8",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_MISC_FUNCTION9",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("GET_CAL_SETTING",     callBack) );    
    dutFunctions[dfIndex].insert( functionPair("SET_CAL_MEASUREMENT", callBack) );         
 
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION1",  callBack) ); //used for DUT temperature reading
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION2",  callBack) ); //used for DUT misc commands
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION3",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION4",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION5",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION6",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION7",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION8",  callBack) );
    dutFunctions[dfIndex].insert( functionPair("DUT_MISC_FUNCTION9",  callBack) );

    // Create the timer ID 
	TIMER_CreateTimer("WIFI_11AC_MIMO_DUT", &g_vDutTimerID[dfIndex]);
    // Create the logger ID 
	LOGGER_CreateLogger("WIFI_11AC_MIMO_DUT", &g_vDutLoggerID[dfIndex]);

    dfIndex++;
    if(dfIndex>MAX_TECHNOLOGIES_COUNT)
    {
        printf("Too many technologies! MAX_TECHNOLOGIES_COUNT needs to be increased!\n");
        exit(1);
    }

    //BT
    dutInfo.dutFunctionIndex   = dfIndex;
    g_dutDllHandle[dfIndex]    = NULL;
    technologies.insert( technologyPair("BT", dutInfo) );
    dutFunctions[dfIndex].clear();
    dutFunctions[dfIndex].insert( functionPair("INSERT_DUT",          callBack) );    
    dutFunctions[dfIndex].insert( functionPair("REMOVE_DUT",          callBack) );
	dutFunctions[dfIndex].insert( functionPair("INITIALIZE_DUT",      callBack) );
	dutFunctions[dfIndex].insert( functionPair("RESET_DUT",           callBack) );

    dutFunctions[dfIndex].insert( functionPair("WRITE_BD_ADDRESS",    callBack) );
    dutFunctions[dfIndex].insert( functionPair("READ_BD_ADDRESS",     callBack) );
	dutFunctions[dfIndex].insert( functionPair("WRITE_MAC_ADDRESS",    callBack) );
    dutFunctions[dfIndex].insert( functionPair("READ_EEPROM",         callBack) );
    dutFunctions[dfIndex].insert( functionPair("WRITE_EEPROM",        callBack) );
	dutFunctions[dfIndex].insert( functionPair("READ_REGISTER",		  callBack) );
	dutFunctions[dfIndex].insert( functionPair("WRITE_REGISTER",      callBack) );
	dutFunctions[dfIndex].insert( functionPair("INITIALIZE_EEPROM",   callBack) );

    dutFunctions[dfIndex].insert( functionPair("TX_START",            callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_STOP",             callBack) );

    dutFunctions[dfIndex].insert( functionPair("RX_CLEAR_STATS",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_GET_STATS",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_START",            callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_STOP",             callBack) );
	dutFunctions[dfIndex].insert( functionPair("GET_CAL_SETTING",     callBack) );    
	dutFunctions[dfIndex].insert( functionPair("SET_CAL_MEASUREMENT", callBack) );    
	dutFunctions[dfIndex].insert( functionPair("QUERY_POWER_DBM",     callBack) );
	dutFunctions[dfIndex].insert( functionPair("WRITE_DUT_INFO",      callBack) );
	dutFunctions[dfIndex].insert( functionPair("DEVICE_TYPE_UART",      callBack) );
	dutFunctions[dfIndex].insert( functionPair("DEVICE_TYPE_SOCKET",      callBack) );
	dutFunctions[dfIndex].insert( functionPair("FUNCTION_FAILED",	  callBack) );
	dutFunctions[dfIndex].insert( functionPair("CALIBRATION_FAILED",  callBack) );

	dutFunctions[dfIndex].insert( functionPair("BTLE_INITIALIZE",     callBack) );	
    dutFunctions[dfIndex].insert( functionPair("TX_BTLE_START",       callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_BTLE_STOP",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_BTLE_START",       callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_BTLE_GET_STATS",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_BTLE_CLEAR_STATS", callBack) );

    // Create the timer ID 
	TIMER_CreateTimer("BT_DUT", &g_vDutTimerID[dfIndex]);
    // Create the logger ID 
	LOGGER_CreateLogger("BT_DUT", &g_vDutLoggerID[dfIndex]);

    dfIndex++;
    if(dfIndex>MAX_TECHNOLOGIES_COUNT)
    {
        printf("Too many technologies! MAX_TECHNOLOGIES_COUNT needs to be increased!\n");
        exit(1);
    }

    //WIMAX
    dutInfo.dutFunctionIndex   = dfIndex;
    g_dutDllHandle[dfIndex]    = NULL;
    technologies.insert( technologyPair("WIMAX", dutInfo) );

    dutFunctions[dfIndex].clear();
    dutFunctions[dfIndex].insert( functionPair("INSERT_DUT",          callBack) );
    dutFunctions[dfIndex].insert( functionPair("INITIALIZE_DUT",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("REMOVE_DUT",          callBack) );

	dutFunctions[dfIndex].insert( functionPair("GET_SERIAL_NUMBER",   callBack) );

	dutFunctions[dfIndex].insert( functionPair("READ_EEPROM",         callBack) );
    dutFunctions[dfIndex].insert( functionPair("WRITE_EEPROM",        callBack) );

	dutFunctions[dfIndex].insert( functionPair("WRITE_MAC_ADDRESS",   callBack) );
    dutFunctions[dfIndex].insert( functionPair("READ_MAC_ADDRESS",    callBack) );

 
    dutFunctions[dfIndex].insert( functionPair("RF_SET_FREQ",         callBack) );

    dutFunctions[dfIndex].insert( functionPair("TX_PRE_TX",           callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_BW",           callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_DATA_RATE",    callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_ANTENNA",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_START",            callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_STOP",             callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC1",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC2",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_SET_MISC3",        callBack) );

    dutFunctions[dfIndex].insert( functionPair("RX_PRE_RX",           callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_BW",           callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_DATA_RATE",    callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_ANTENNA",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_FILTER",       callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_CLEAR_STATS",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_GET_STATS",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_START",            callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_STOP",             callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_MISC1",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_MISC2",        callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_SET_MISC3",        callBack) );

    dutFunctions[dfIndex].insert( functionPair("TX_RX_CALIBRATION",   callBack) );      
	dutFunctions[dfIndex].insert( functionPair("RESET_DUT",           callBack) );    

	dutFunctions[dfIndex].insert( functionPair("FUNCTION_FAILED",	  callBack) );
	dutFunctions[dfIndex].insert( functionPair("CALIBRATION_FAILED",  callBack) );

    // Create the timer ID 
	TIMER_CreateTimer("WIMAX_DUT", &g_vDutTimerID[dfIndex]);
    // Create the logger ID 
	LOGGER_CreateLogger("WIMAX_DUT", &g_vDutLoggerID[dfIndex]);

    dfIndex++;
    if(dfIndex>MAX_TECHNOLOGIES_COUNT)
    {
        printf("Too many technologies! MAX_TECHNOLOGIES_COUNT needs to be increased!\n");
        exit(1);
    }

    //GPS
    dutInfo.dutFunctionIndex   = dfIndex;
    g_dutDllHandle[dfIndex]    = NULL;
    technologies.insert( technologyPair("GPS", dutInfo) );

	//////////////////////////////////////////////////////////////////
	// [Jarir Fadlullah] Start: Added GPS functions for Brcm 2076 GPS
    dutFunctions[dfIndex].clear();
    dutFunctions[dfIndex].insert( functionPair("INSERT_DUT",          callBack) );
    dutFunctions[dfIndex].insert( functionPair("INITIALIZE_DUT",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("REMOVE_DUT",          callBack) );

    dutFunctions[dfIndex].insert( functionPair("RX_CLEAR_STATS",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_START",            callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_STOP",             callBack) );
	dutFunctions[dfIndex].insert( functionPair("RX_GET_MEASUREMENTS",      callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_GET_CLOCK",        callBack) );
	dutFunctions[dfIndex].insert( functionPair("RX_GET_POSITION",      callBack) );
	dutFunctions[dfIndex].insert( functionPair("FUNCTION_FAILED",      callBack) );
	// [Jarir Fadlullah] End: Added GPS functions for Brcm 2076 GPS
	//////////////////////////////////////////////////////////////////
    // Create the timer ID 
	TIMER_CreateTimer("GPS_DUT", &g_vDutTimerID[dfIndex]);
    // Create the logger ID 
	LOGGER_CreateLogger("GPS_DUT", &g_vDutLoggerID[dfIndex]);


    dfIndex++;
    if(dfIndex>MAX_TECHNOLOGIES_COUNT)
    {
        printf("Too many technologies! MAX_TECHNOLOGIES_COUNT needs to be increased!\n");
        exit(1);
    }

    //FM
    dutInfo.dutFunctionIndex   = dfIndex;
    g_dutDllHandle[dfIndex]    = NULL;
    technologies.insert( technologyPair("FM", dutInfo) );
    dutFunctions[dfIndex].clear();
    dutFunctions[dfIndex].insert( functionPair("INSERT_DUT",          callBack) );    
    dutFunctions[dfIndex].insert( functionPair("REMOVE_DUT",          callBack) );
	dutFunctions[dfIndex].insert( functionPair("INITIALIZE_DUT",      callBack) );

    dutFunctions[dfIndex].insert( functionPair("TX_START",            callBack) );
	dutFunctions[dfIndex].insert( functionPair("TX_STATUS",             callBack) );
    dutFunctions[dfIndex].insert( functionPair("TX_STOP",             callBack) );

    dutFunctions[dfIndex].insert( functionPair("RX_START",            callBack) );
    dutFunctions[dfIndex].insert( functionPair("RX_STOP",             callBack) );
	dutFunctions[dfIndex].insert( functionPair("RX_STATUS",			  callBack) );    
	dutFunctions[dfIndex].insert( functionPair("FUNCTION_FAILED",	  callBack) );
	dutFunctions[dfIndex].insert( functionPair("CALIBRATION_FAILED",  callBack) );


    // Create the timer ID 
	TIMER_CreateTimer("FM_DUT", &g_vDutTimerID[dfIndex]);
    // Create the logger ID 
	LOGGER_CreateLogger("FM_DUT", &g_vDutLoggerID[dfIndex]);

    dfIndex++;
    if(dfIndex>MAX_TECHNOLOGIES_COUNT)
    {
        printf("Too many technologies! MAX_TECHNOLOGIES_COUNT needs to be increased!\n");
        exit(1);
    }

    // Initialize the WiFi Frequency-Channel map, The formula for converting between freq in MHz is cneter freq(MHz)=5000+5xN, N=0,1,2...199
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
	wifiChannels.insert( pair<int,int>( 5250, 50  )); /// Add this channel number, for Brcm_43562, non BW_20_MHz channel. 
	wifiChannels.insert( pair<int,int>( 5260, 52  ));
    wifiChannels.insert( pair<int,int>( 5270, 54  ));
	wifiChannels.insert( pair<int,int>( 5280, 56  ));
	wifiChannels.insert( pair<int,int>( 5290, 58  ));  //11ac channel
	wifiChannels.insert( pair<int,int>( 5300, 60  ));
    wifiChannels.insert( pair<int,int>( 5310, 62  ));
	wifiChannels.insert( pair<int,int>( 5320, 64  ));
	wifiChannels.insert( pair<int,int>( 5330, 66  )); /// Add this channel number, for Brcm_43562, non BW_20_MHz channel. 
	wifiChannels.insert( pair<int,int>( 5400, 80  ));  //Add this channel number, which is not defined in 11a spec, for Atheros calibration. 
	wifiChannels.insert( pair<int,int>( 5500, 100 ));
    wifiChannels.insert( pair<int,int>( 5510, 102 ));
	wifiChannels.insert( pair<int,int>( 5520, 104 ));
	wifiChannels.insert( pair<int,int>( 5530, 106 )); //11ac channel
	wifiChannels.insert( pair<int,int>( 5540, 108 ));
    wifiChannels.insert( pair<int,int>( 5550, 110 ));
	wifiChannels.insert( pair<int,int>( 5560, 112 ));
	wifiChannels.insert( pair<int,int>( 5570, 114 )); /// Add this channel number, for Brcm_43562, non BW_20_MHz channel. 
	wifiChannels.insert( pair<int,int>( 5580, 116 ));
    wifiChannels.insert( pair<int,int>( 5590, 118 ));
	wifiChannels.insert( pair<int,int>( 5600, 120 ));
	wifiChannels.insert( pair<int,int>( 5610, 122 )); //11ac channel
	wifiChannels.insert( pair<int,int>( 5620, 124 ));
    wifiChannels.insert( pair<int,int>( 5630, 126 ));
	wifiChannels.insert( pair<int,int>( 5640, 128 ));
	wifiChannels.insert( pair<int,int>( 5660, 132 ));
    wifiChannels.insert( pair<int,int>( 5670, 134 ));
	wifiChannels.insert( pair<int,int>( 5680, 136 ));
	wifiChannels.insert( pair<int,int>( 5690, 138 )); //11ac channel
	wifiChannels.insert( pair<int,int>( 5700, 140 ));
	wifiChannels.insert( pair<int,int>( 5710, 142 ));
	wifiChannels.insert( pair<int,int>( 5720, 144 ));  //Add this channel number, which is not defined in 11a spec, for Atheros calibration. 
	wifiChannels.insert( pair<int,int>( 5745, 149 ));
	wifiChannels.insert( pair<int,int>( 5755, 151 ));
    wifiChannels.insert( pair<int,int>( 5765, 153 ));
	wifiChannels.insert( pair<int,int>( 5775, 155 )); //11ac channel
	wifiChannels.insert( pair<int,int>( 5785, 157 ));
    wifiChannels.insert( pair<int,int>( 5795, 159 ));
	wifiChannels.insert( pair<int,int>( 5805, 161 ));
	wifiChannels.insert( pair<int,int>( 5815, 163  )); /// Add this channel number, for Brcm_43562, non BW_20_MHz channel. 
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
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "DSSS-1",  DSSS1 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "DSSS-2",  DSSS2 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "CCK-5_5", CCK5_5));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "CCK-11",  CCK11 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "PBCC-22", PBCC22 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "OFDM-6",  OFDM6 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "OFDM-9",  OFDM9 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "OFDM-12", OFDM12 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "OFDM-18", OFDM18 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "OFDM-24", OFDM24 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "OFDM-36", OFDM36 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "OFDM-48", OFDM48 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "OFDM-54", OFDM54 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS0",    MCS0 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS1",    MCS1 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS2",    MCS2 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS3",    MCS3 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS4",    MCS4 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS5",    MCS5 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS6",    MCS6 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS7",    MCS7 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS8",    MCS8 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS9",    MCS9 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS10",   MCS10 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS11",   MCS11 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS12",   MCS12 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS13",   MCS13 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS14",   MCS14 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS15",   MCS15 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS16",   MCS16 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS17",   MCS17 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS18",   MCS18 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS19",   MCS19 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS20",   MCS20 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS21",   MCS21 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS22",   MCS22 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS23",   MCS23 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS24",   MCS24 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS25",   MCS25 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS26",   MCS26 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS27",   MCS27 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS28",   MCS28 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS29",   MCS29 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS30",   MCS30 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "MCS31",   MCS31 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "1DH1",   BT_1DH1 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "1DH3",   BT_1DH3 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "1DH5",   BT_1DH5 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "2DH1",   BT_2DH1 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "2DH3",   BT_2DH3 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "2DH5",   BT_2DH5 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "3DH1",   BT_3DH1 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "3DH3",   BT_3DH3 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "3DH5",   BT_3DH5 ));
    wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "1LE",   BT_1LE ));
	wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "HALF-3",	HALF3 ));
	wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "HALF-4_5",	HALF4_5 ));
	wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "HALF-6",	HALF6 ));
	wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "HALF-9",	HALF9 ));
	wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "HALF-12",	HALF12 ));
	wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "HALF-18",	HALF18 ));
	wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "HALF-24",	HALF24 ));
	wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "HALF-27",	HALF27 ));
	wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "QUAR-1_5",	QUAR1_5 ));
	wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "QUAR-2_25",QUAR2_25 ));
	wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "QUAR-3",	QUAR3 ));
	wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "QUAR-4_5",	QUAR4_5 ));
	wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "QUAR-6",	QUAR6 ));
	wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "QUAR-9",	QUAR9 ));
	wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "QUAR-12",	QUAR12 ));
	wifiDataRates.insert( pair<string, vDUT_DATA_RATE>( "QUAR-13_5",QUAR13_5 ));

    // Initializes the WiFi data rate Mbps map
    wifiDataRatesMbps.clear();
    wifiDataRatesMbps.insert( pair<string, double>( "DSSS-1",   1 ));
    wifiDataRatesMbps.insert( pair<string, double>( "DSSS-2",   2 ));
    wifiDataRatesMbps.insert( pair<string, double>( "CCK-5_5",  5.5 ));
    wifiDataRatesMbps.insert( pair<string, double>( "CCK-11",   11 ));
    wifiDataRatesMbps.insert( pair<string, double>( "PBCC-22",  22 ));
    wifiDataRatesMbps.insert( pair<string, double>( "OFDM-6",   6 ));
    wifiDataRatesMbps.insert( pair<string, double>( "OFDM-9",   9 ));
    wifiDataRatesMbps.insert( pair<string, double>( "OFDM-12",  12 ));
    wifiDataRatesMbps.insert( pair<string, double>( "OFDM-18",  18 ));
    wifiDataRatesMbps.insert( pair<string, double>( "OFDM-24",  24 ));
    wifiDataRatesMbps.insert( pair<string, double>( "OFDM-36",  36 ));
    wifiDataRatesMbps.insert( pair<string, double>( "OFDM-48",  48 ));
    wifiDataRatesMbps.insert( pair<string, double>( "OFDM-54",  54 ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS0_HT20",    6.5     ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS1_HT20",    13      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS2_HT20",    19.5    ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS3_HT20",    26      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS4_HT20",    39      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS5_HT20",    52      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS6_HT20",    58.5    ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS7_HT20",    65      ));

    wifiDataRatesMbps.insert( pair<string, double>( "MCS0_HT40",    13.5    ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS1_HT40",    27      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS2_HT40",    40.5    ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS3_HT40",    54      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS4_HT40",    81      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS5_HT40",    108     ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS6_HT40",    121.5   ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS7_HT40",    135     ));

    wifiDataRatesMbps.insert( pair<string, double>( "MCS8_HT20",    13      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS9_HT20",    26      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS10_HT20",   39      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS11_HT20",   52      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS12_HT20",   78      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS13_HT20",   104     ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS14_HT20",   117     ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS15_HT20",   130     ));

    wifiDataRatesMbps.insert( pair<string, double>( "MCS8_HT40",    27      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS9_HT40",    54      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS10_HT40",   81      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS11_HT40",   108     ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS12_HT40",   162     ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS13_HT40",   216     ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS14_HT40",   243     ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS15_HT40",   270     ));

    wifiDataRatesMbps.insert( pair<string, double>( "MCS16_HT20",    19.5     ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS17_HT20",    39      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS18_HT20",    58.5    ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS19_HT20",    78      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS20_HT20",    117      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS21_HT20",    156      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS22_HT20",    175.5    ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS23_HT20",    195      ));

    wifiDataRatesMbps.insert( pair<string, double>( "MCS16_HT40",    40.5    ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS17_HT40",    81      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS18_HT40",    121.5    ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS19_HT40",    162      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS20_HT40",    243      ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS21_HT40",    324     ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS22_HT40",    364.5   ));
    wifiDataRatesMbps.insert( pair<string, double>( "MCS23_HT40",    405     ));
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

    return 0;												 
}

vDUT_API vDUT_RETURN vDUT_InstallCallbackFunction(vDUT_ID dutID, const vDUT_STR functionName, int (*pointerToFunction)(void) )
{
    vDUT_RETURN ret = vDUT_ERR_OK;
    
    map <string, vDUT_CALLBACK>::iterator function_Iter;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {

        function_Iter = dutFunctions[dutID].find( functionName );

        if( function_Iter!=dutFunctions[dutID].end() )
        {
            function_Iter->second.pointerToFunction = pointerToFunction;
        }
        else
        {
            ret = vDUT_ERR_FUNCTION_NOT_DEFINED;
        }
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_ClearParameters(vDUT_ID dutID)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        intParams[dutID].clear();
        doubleParams[dutID].clear();
        stringParams[dutID].clear();
        sequenceTxParams[dutID].clear();
        sequenceRxParams[dutID].clear();
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_ClearReturns(vDUT_ID dutID)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        intReturns[dutID].clear();
        doubleReturns[dutID].clear();
        stringReturns[dutID].clear();
        g_arrayDoubleReturns[dutID].clear();
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_GetDoubleReturnPairs(vDUT_ID dutID,const int maxPairs, int *numDoublePairs, char doublePairName[][64], double doublePairValue[])
{
	vDUT_RETURN ret = vDUT_ERR_OK;
	int count = 0;
	
	if (g_byPassDut==0)		// 0: means bypass Dut control
	{
		return ret;
	}
	else
	{
		// do nothing
	}	

	if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
	{

		*numDoublePairs = (int)doubleReturns[dutID].size();
		map <string, double>::iterator doubleReturn_Iter;

		//write double params
		for ( doubleReturn_Iter = doubleReturns[dutID].begin( ) ; doubleReturn_Iter != doubleReturns[dutID].end( ) && count < maxPairs; doubleReturn_Iter++ )
		{
			strcpy_s(doublePairName[count], 64, (char*)doubleReturn_Iter->first.c_str());
			doublePairValue[count] = doubleReturn_Iter->second;
			count++;
		}
	}
	else
	{
		ret = vDUT_ERR_INVALID_DUT_ID;;
	}

	return ret;

}

vDUT_API vDUT_RETURN vDUT_GetIntegerReturnPairs(vDUT_ID dutID,const int maxPairs, int *numIntegerPairs, char integerPairName[][64], int integerPairValue[])
{
	vDUT_RETURN ret = vDUT_ERR_OK;
	int count = 0;


	if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
	{

		*numIntegerPairs = (int)intReturns[dutID].size();
		map <string, int>::iterator intergerReturn_Iter;

		//write double params
		for ( intergerReturn_Iter = intReturns[dutID].begin( ) ; intergerReturn_Iter != intReturns[dutID].end( ) && count < maxPairs; intergerReturn_Iter++ )
		{
			strcpy_s(integerPairName[count], 64, (char*)intergerReturn_Iter->first.c_str());
			integerPairValue[count] = intergerReturn_Iter->second;
			count++;
		}
	}
	else
	{
		ret = vDUT_ERR_INVALID_DUT_ID;
	}

	return ret;

}


vDUT_API vDUT_RETURN vDUT_AddIntegerParameter(vDUT_ID dutID, const vDUT_STR paramName, const int paramValue)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        intParams[dutID].insert( intParamPair(paramName, paramValue) );
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }
    return ret;
}

vDUT_API vDUT_RETURN vDUT_AddIntegerReturn(vDUT_ID dutID, const vDUT_STR paramName, const int paramValue)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        intReturns[dutID].insert( intReturnPair(paramName, paramValue) );
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }
    return ret;
}

vDUT_API vDUT_RETURN vDUT_GetIntegerParameter(vDUT_ID dutID, const vDUT_STR paramName, int* paramValue)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

	if (g_byPassDut==0)		// 0: means bypass Dut control
	{
		return ret;
	}
	else
	{
		// do nothing
	}
    map <string, int>::iterator intParam_Iter;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        intParam_Iter = intParams[dutID].find(paramName);

        if( intParam_Iter!=intParams[dutID].end() )
        {
            *paramValue = intParam_Iter->second;
        }
        else
        {
            ret = vDUT_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_GetIntegerReturn(vDUT_ID dutID, const vDUT_STR paramName, int* paramValue)
{
    vDUT_RETURN ret = vDUT_ERR_OK;
	if (g_byPassDut==0)		// 0: means bypass Dut control
	{
		return ret;
	}
	else
	{
		// do nothing
	}

    map <string, int>::iterator intParam_Iter;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        intParam_Iter = intReturns[dutID].find(paramName);
        if( intParam_Iter!=intReturns[dutID].end() )
        {
            *paramValue = intParam_Iter->second;
        }
        else
        {
            ret = vDUT_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_AddDoubleParameter(vDUT_ID dutID, const vDUT_STR paramName, const double paramValue)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        doubleParams[dutID].insert( doubleParamPair(paramName, paramValue) );
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }
    return ret;
}

vDUT_API vDUT_RETURN vDUT_AddDoubleReturn(vDUT_ID dutID, const vDUT_STR paramName, const double paramValue)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        doubleReturns[dutID].insert( doubleReturnPair(paramName, paramValue) );
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }
    return ret;
}

vDUT_API vDUT_RETURN vDUT_GetDoubleParameter(vDUT_ID dutID, const vDUT_STR paramName, double* paramValue)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

	if (g_byPassDut==0)		// 0: means bypass Dut control
	{
		return ret;
	}
	else
	{
		// do nothing
	}
    map <string, double>::iterator doubleParam_Iter;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        doubleParam_Iter = doubleParams[dutID].find(paramName);

        if( doubleParam_Iter!=doubleParams[dutID].end() )
        {
            *paramValue = doubleParam_Iter->second;
        }
        else
        {
            ret = vDUT_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_GetDoubleReturn(vDUT_ID dutID, const vDUT_STR paramName, double* paramValue)
{
    vDUT_RETURN ret = vDUT_ERR_OK;
	if (g_byPassDut==0)		// 0: means bypass Dut control
	{
		return ret;
	}
	else
	{
		// do nothing
	}

    map <string, double>::iterator doubleReturn_Iter;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        doubleReturn_Iter = doubleReturns[dutID].find(paramName);
        if( doubleReturn_Iter!=doubleReturns[dutID].end() )
        {
            *paramValue = doubleReturn_Iter->second;
        }
        else
        {
            ret = vDUT_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_AddStringParameter(vDUT_ID dutID, const vDUT_STR paramName, const vDUT_STR paramValue)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        stringParams[dutID].insert( stringParamPair(paramName, paramValue) );
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }
    return ret;
}

vDUT_API vDUT_RETURN vDUT_AddStringReturn(vDUT_ID dutID, const vDUT_STR paramName, const vDUT_STR paramValue)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        stringReturns[dutID].insert( stringReturnPair(paramName, paramValue) );
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }
    return ret;
}

vDUT_API vDUT_RETURN vDUT_GetStringParameter(vDUT_ID dutID, const vDUT_STR paramName, vDUT_STR paramValue, int bufferSize)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    map <string, string>::iterator stringParam_Iter;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        stringParam_Iter = stringParams[dutID].find(paramName);
        if( stringParam_Iter!=stringParams[dutID].end() )
        {
            strcpy_s((char*)paramValue, bufferSize, (char*)stringParam_Iter->second.c_str());
        }
        else
        {
            ret = vDUT_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_GetStringReturn(vDUT_ID dutID, const vDUT_STR paramName, vDUT_STR paramValue, int bufferSize)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    map <string, string>::iterator stringReturn_Iter;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        stringReturn_Iter = stringReturns[dutID].find(paramName);
        if( stringReturn_Iter!=stringReturns[dutID].end() )
        {
            strcpy_s((char*)paramValue, bufferSize, (char*)stringReturn_Iter->second.c_str());
        }
        else
        {
            ret = vDUT_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }

    return ret;
}


vDUT_API vDUT_RETURN vDUT_AddSequenceParameters(vDUT_ID dutID, SEQ_TX_CONFIG *sequenceParameters, int seqStepCounter)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        for (int i = 0; i < seqStepCounter; i++)
        {
            sequenceTxParams[dutID].push_back(*sequenceParameters);
            sequenceParameters ++;
        }
   }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }
    return ret;
}

 vDUT_API vDUT_RETURN vDUT_AddTxOneDataRateSequenceParameters(vDUT_ID dutID, vector<SEQ_TX_ONE_DATARATE_CONFIG>::iterator &sequenceIter, int seqStepCounter)
 {
	 vDUT_RETURN ret = vDUT_ERR_OK;
	 sequenceTxOneDataRateParams[dutID].clear();

	 if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
	 {
		 for (int i = 0; i < seqStepCounter; i++)
		 {
			 sequenceTxOneDataRateParams[dutID].push_back(*sequenceIter);
			 sequenceIter ++;
		 }
	 }
	 else
	 {
		 ret = vDUT_ERR_INVALID_DUT_ID;;
	 }
	 return ret;
 }

vDUT_API vDUT_RETURN vDUT_AddRxSequenceParameters(vDUT_ID dutID, SEQ_RX_CONFIG *sequenceParameters, int seqStepCounter)
{
	vDUT_RETURN ret = vDUT_ERR_OK;

	if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
	{
		for (int i = 0; i < seqStepCounter; i++)
		{
			sequenceRxParams[dutID].push_back(*sequenceParameters);
			sequenceParameters ++;
		}
	}
	else
	{
		ret = vDUT_ERR_INVALID_DUT_ID;;
	}
	return ret;
}

vDUT_API vDUT_RETURN vDUT_GetSequenceParameters(vDUT_ID dutID, SEQ_TX_CONFIG *sequenceParameters)
{
    vDUT_RETURN ret = vDUT_ERR_OK;
    vector<SEQ_TX_CONFIG>::iterator seqParam_Iter;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        seqParam_Iter = sequenceTxParams[dutID].begin();
        int i = 0;

        while(seqParam_Iter != sequenceTxParams[dutID].end())
        {
            *sequenceParameters = seqParam_Iter[0];
            sequenceParameters++;
            seqParam_Iter++;
            i++;
        }
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }

    return ret;
}

 vDUT_API vDUT_RETURN vDUT_GetTxOneDataRateSequenceParameters(vDUT_ID dutID, vector<SEQ_TX_ONE_DATARATE_CONFIG> &sequenceTxOneDataRate)
 {
	 vDUT_RETURN ret = vDUT_ERR_OK;
	 vector<SEQ_TX_ONE_DATARATE_CONFIG>::iterator seqParam_Iter;	
	 if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
	 {
		 seqParam_Iter = sequenceTxOneDataRateParams[dutID].begin();	

			 while (seqParam_Iter != sequenceTxOneDataRateParams[dutID].end())
			 {
				 sequenceTxOneDataRate.push_back(*seqParam_Iter);
				 seqParam_Iter++;
			 }
	 }
	 else
	 {
		 ret = vDUT_ERR_INVALID_DUT_ID;;
	 }
	 return ret;
 }

vDUT_API vDUT_RETURN vDUT_GetRxSequenceParameters(vDUT_ID dutID, SEQ_RX_CONFIG *sequenceParameters)
{
	vDUT_RETURN ret = vDUT_ERR_OK;
	vector<SEQ_RX_CONFIG>::iterator seqParam_Iter;

	if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
	{
		seqParam_Iter = sequenceRxParams[dutID].begin();
		int i = 0;

		while(seqParam_Iter != sequenceRxParams[dutID].end())
		{
			*sequenceParameters = seqParam_Iter[0];
			sequenceParameters++;
			seqParam_Iter++;
			i++;
		}
	}
	else
	{
		ret = vDUT_ERR_INVALID_DUT_ID;;
	}

	return ret;
}

vDUT_API vDUT_RETURN vDUT_ByPassDutControl(int vDutControlFlag)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    g_byPassDut = vDutControlFlag;

    return ret;
}



vDUT_API vDUT_RETURN vDUT_Run(vDUT_ID dutID, const vDUT_STR functionName)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

	if (g_byPassDut==0)		// 0: means bypass Dut control
	{
		return ret;
	}
	else
	{
		// do nothing
	}

	double durationInMiniSec = 0;

    map <string, vDUT_CALLBACK>::iterator function_Iter;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        function_Iter = dutFunctions[dutID].find( functionName );

        if( function_Iter!=dutFunctions[dutID].end() )
        {
            if( NULL!=function_Iter->second.pointerToFunction )
            {
                // Start Timer
				::TIMER_StartTimer(g_vDutTimerID[dutID], functionName);
				::LOGGER_Write_Ext(LOG_IQLITE_VDUT, g_vDutLoggerID[dutID], LOGGER_INFORMATION, "[vDut]=>vDUT_Run[%s]\n", functionName);

                if( 0==function_Iter->second.pointerToFunction() )
                {
                    // DUT function ran OK
                }
                else
                {
                    // DUT function had an error
                    // TODO: how do we handle the errors from DUT functions?
                    // Option 1: to insert a string message before returing
                    ret = vDUT_ERR_DUT_FUNCTION_ERROR;
                }

				// Stop Timer
				::TIMER_StopTimer(g_vDutTimerID[dutID], functionName, &durationInMiniSec);
				// Save to log
				::LOGGER_Write_Ext(LOG_IQLITE_VDUT, g_vDutLoggerID[dutID], LOGGER_INFORMATION, "[vDut]=>[%s],%.2f,ms\n", functionName, durationInMiniSec);
            }
            else
            {
                //printf("DUT does not support %s\n", functionName);
                ret = vDUT_ERR_FUNCTION_NOT_SUPPORTED;
            }
        }
        else
        {
            //printf("%s cannot be found\n", functionName);
            ret = vDUT_ERR_FUNCTION_NOT_DEFINED;
        }
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_RegisterTechnology(const vDUT_STR technology, vDUT_ID *dutID)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    // We need to check if the specified technology is one of the pre-defined, or
    // if another DUT has registered the same technology already
    technology_Iter = technologies.find( technology );
    if( technology_Iter == technologies.end() )
    {
        ret = vDUT_ERR_TECHNOLOGY_DOES_NOT_EXIST;
    }
    else
    {
        if( technology_Iter->second.dutID != -1 )
        {
            ret = vDUT_ERR_TECHNOLOGY_HAS_BEEN_REGISTERED;
        }
        else
        {
            // Register the technology with the DUT
            g_lastDutId++;
            // Since we only allow one DUT to register one technology, we can use
            // dutFunctionIndex as the DUT ID to make it easier to call the DUT's functions;
            // otherwise, if the DUT ID is different from the function index, we have to
            // a search each time we call a DUT function in map "technologies"
            technology_Iter->second.dutID = technology_Iter->second.dutFunctionIndex;
            *dutID = technology_Iter->second.dutFunctionIndex;
        }
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_UnregisterTechnology(const vDUT_STR technology)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    // We need to check if the specified technology is one of the pre-defined, or
    // if another DUT has registered the same technology already
    technology_Iter = technologies.find( technology );
    if( technology_Iter == technologies.end() )
    {
        ret = vDUT_ERR_TECHNOLOGY_DOES_NOT_EXIST;
    }
    else
    {
        if( technology_Iter->second.dutID != -1 )
        {
            // Unregister
            int dfIndex = technology_Iter->second.dutID;
            technology_Iter->second.dutID = -1;
            // Set all callback functions to NULL

            map <string, vDUT_CALLBACK>::iterator function_Iter;
            for(function_Iter=dutFunctions[dfIndex].begin(); 
                function_Iter!=dutFunctions[dfIndex].end();
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

vDUT_API vDUT_RETURN vDUT_RegisterDutDll(const vDUT_STR technology, vDUT_STR dutDllFileName, vDUT_ID *dutID)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    // We need to check if the specified technology is one of the pre-defined, or
    // if the specified technology is taken
    technology_Iter = technologies.find( technology );
    try
    {
        if( technology_Iter == technologies.end() )
        {
            throw vDUT_ERR_TECHNOLOGY_DOES_NOT_EXIST;
        }

        if( technology_Iter->second.dutID != -1 )
        {
            throw vDUT_ERR_TECHNOLOGY_HAS_BEEN_REGISTERED;
        }

        // Load the DUT control DLL
        HMODULE dutDllHandle = LoadLibraryA( dutDllFileName );
        if( NULL==dutDllHandle )
        {
            throw vDUT_ERR_FAILED_TO_LOAD_DUT_DLL;
        }

        string registerFunctionName(technology);
        registerFunctionName += "_DutRegister";

        dutRegister dutRegisterFunction;

        // Get the Dut register function address
        dutRegisterFunction = (dutRegister)GetProcAddress( dutDllHandle, registerFunctionName.c_str() );

        if( NULL==dutRegisterFunction )
        {
            throw vDUT_ERR_NO_DUT_REGISTER_FUNCTION_FOUND;
            FreeLibrary( dutDllHandle );
        }

        // So far, so good.  We are going to execute the function
        // dutRegisterFunction() does not follow the general rule in terms of return value
        // If it returns a value >=0, registration succeeded.
        // If it returns -1, failed.
        *dutID = dutRegisterFunction();
        if( -1==*dutID )
        {
            // The DUT register function failed
            throw vDUT_ERR_DUT_REGISTER_FUNCTION_FAILED;
            FreeLibrary( dutDllHandle );
        }

        // Save the handle for FreeLibrary()
        g_dutDllHandle[*dutID] = dutDllHandle;

        //RW-20090506: the code below has been executed by "dutRegisterFunction" calling vDUT_RegisterTechnology()
        //// Register the technology with the DUT
        //g_lastDutId++;
        //// Since we only allow one DUT to register for one technology, we can use
        //// dutFunctionIndex as the DUT ID to make it easier to call the DUT's functions;
        //// otherwise, if the DUT ID is different from the function index, we have to
        //// search each time we call a DUT function in map "technologies"
        //technology_Iter->second.dutID = technology_Iter->second.dutFunctionIndex;

    }
    catch(vDUT_RETURN errCode)
    {
        ret = errCode;
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_UnregisterDutDll(const vDUT_STR technology, vDUT_ID *dutID)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    // We need to check if the specified technology is one of the pre-defined, or
    // if another DUT has registered the same technology already
    technology_Iter = technologies.find( technology );
    if( technology_Iter == technologies.end() )
    {
        ret = vDUT_ERR_TECHNOLOGY_DOES_NOT_EXIST;
    }
    else
    {
        if( technology_Iter->second.dutID != -1 )
        {
            // unload the DLL
            if( NULL!=g_dutDllHandle[technology_Iter->second.dutID] )
            {
                FreeLibrary( g_dutDllHandle[technology_Iter->second.dutID] );
                g_dutDllHandle[technology_Iter->second.dutID] = NULL;
            }
            else
            {
                // do nothing
            }
            
            // Unregister
            int dfIndex = technology_Iter->second.dutID;
            technology_Iter->second.dutID = -1;
            // Set all callback functions to NULL

            map <string, vDUT_CALLBACK>::iterator function_Iter;
            for(function_Iter=dutFunctions[dfIndex].begin(); 
                function_Iter!=dutFunctions[dfIndex].end();
                function_Iter++)
            {
                function_Iter->second.pointerToFunction = NULL;
            }

            // Change the DUT ID to -1
            *dutID = -1;
        }
        else
        {
            // No DUT has been registered.  Do nothing.
        }
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_WiFiConvertFrequencyToChannel(int freq, int* channel)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    map <int, int>::iterator channel_Iter;

    channel_Iter = wifiChannels.find(freq);
    if( channel_Iter!=wifiChannels.end() )
    {
        *channel = channel_Iter->second;
    }
    else
    {
        ret = vDUT_ERR_WIFI_FREQ_DOES_NOT_EXIST;
    }

    return ret;
}

/* #LPTW# cfy,-2011/07/7- */
vDUT_API vDUT_RETURN vDUT_WiFiConvertChannelToFrequency(int channel, int *freq)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    map <int, int>::iterator freq_Iter;

    freq_Iter = wifiChannels.begin();
    if( freq_Iter!=wifiChannels.end() )
    {
        while( freq_Iter!=wifiChannels.end() )
        {
            if ( channel==freq_Iter->second )
            {
                *freq = freq_Iter->first;
                ret = vDUT_ERR_OK;
                break;
            }
            else
            {                
                // keep searching...
                freq_Iter++;
                ret = vDUT_ERR_WIFI_FREQ_DOES_NOT_EXIST;
            }
        }
    }
    else
    {
        ret = vDUT_ERR_WIFI_FREQ_DOES_NOT_EXIST;
    }
    return ret;
}

vDUT_API vDUT_RETURN vDUT_WiFiConvertDataRateNameToIndex(vDUT_STR name, vDUT_DATA_RATE* index)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    map <string, vDUT_DATA_RATE>::iterator dataRate_Iter;

    dataRate_Iter = wifiDataRates.find(name);
    if( dataRate_Iter!=wifiDataRates.end() )
    {
        *index = dataRate_Iter->second;
    }
    else
    {
        ret = vDUT_ERR_WIFI_FREQ_DOES_NOT_EXIST;
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_GetVersion(vDUT_STR paramValue, int bufferSize)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    if( 0!=strlen(g_vDUT_Version) )	// vDUT Version not empty
    {
		strcpy_s((char*)paramValue, bufferSize, (char*)g_vDUT_Version);
    }
    else
    {
        ret = vDUT_ERR_PARAM_DOES_NOT_EXIST;;
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_GetDutObjectPointer(void **pDutObj, int *DutConnected)
{
	*pDutObj = DutObj;
	*DutConnected = DutObjConnected;
	return vDUT_ERR_OK;
}

vDUT_API vDUT_RETURN vDUT_SetDutObjectPointer(void *pDutObj, int DutConnected)
{
	DutObj = pDutObj;
	DutObjConnected = DutConnected;
	return vDUT_ERR_OK;
}

vDUT_API vDUT_RETURN vDUT_RemoveDoubleParameter(vDUT_ID dutID, const vDUT_STR paramName)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    map <string, double>::iterator doubleParam_Iter;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        doubleParam_Iter = doubleParams[dutID].find(paramName);

        if( doubleParam_Iter!=doubleParams[dutID].end() )
        {
            doubleParams[dutID].erase(doubleParam_Iter);
        }
        else
        {
            ret = vDUT_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_RemoveIntegerParameter(vDUT_ID dutID, const vDUT_STR paramName)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    map <string, int>::iterator intParam_Iter;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        intParam_Iter = intParams[dutID].find(paramName);

        if( intParam_Iter!=intParams[dutID].end() )
        {
            intParams[dutID].erase(intParam_Iter);
        }
        else
        {
            ret = vDUT_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_RemoveStringParameter(vDUT_ID dutID, const vDUT_STR paramName)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    map <string, string>::iterator stringParam_Iter;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        stringParam_Iter = stringParams[dutID].find(paramName);

        if( stringParam_Iter!=stringParams[dutID].end() )
        {
            stringParams[dutID].erase(stringParam_Iter);
        }
        else
        {
            ret = vDUT_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_WiFiConvertDataRateNameToMbps(vDUT_STR name, double* dataRateMbps)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    map <string, double>::iterator dataRateMbps_Iter;

    dataRateMbps_Iter = wifiDataRatesMbps.find(name);
    if( dataRateMbps_Iter!=wifiDataRatesMbps.end() )
    {
        *dataRateMbps = dataRateMbps_Iter->second;
    }
    else
    {
        ret = vDUT_ERR_PARAM_DOES_NOT_EXIST;
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_WiFiConvertPacketLengthUsToNumberOfBytes(int packetLengthUs, char *dataRate, vDUT_BANDWIDTH_MODE bandwidth, vDUT_PREAMBLE_TYPE preamble, vDUT_PACKET_FORMAT_11N packetType11n, int* numberOfBytes)
{
    vDUT_RETURN     ret = vDUT_ERR_OK;
    double          dataRateMbps = 0;
    vDUT_DATA_RATE  dataRateIndex = OFDM54;
    char            dataRateString[MAX_BUFFER_SIZE] = {0};
    const int       minMpduInBits = 34*8; //By running some testing looks like there is 28 Bytes in Broadcom CRC ????   34*8     //There are at least 34 bytes data in MPDU for 802.11 frame
    const int       longPreambleLengthUs = 192; 
    const int       shortPreambleLengthUs = 96; 
    int             mpduTimeUs = 0;
    const int       dsss1MinLengthUs = 420;
    const int       dsss2MinLengthUs = 310;
    
    ret = vDUT_WiFiConvertDataRateNameToIndex(dataRate, &dataRateIndex);
    if (ret == vDUT_ERR_OK)
    {
        if (dataRateIndex <= OFDM54)
        {
            sprintf_s(dataRateString, "%s", dataRate);
        }
        else if (bandwidth == BANDWIDTH_MODE_HT20)
        {
            sprintf_s(dataRateString, "%s_HT20", dataRate);
        }
        else if (bandwidth == BANDWIDTH_MODE_HT40)
            sprintf_s(dataRateString, "%s_HT40", dataRate);

        ret = vDUT_WiFiConvertDataRateNameToMbps(dataRateString, &dataRateMbps);
        if(ret == vDUT_ERR_OK)
        {
            if (dataRateIndex <= CCK11)
            {
                // Minimum length of DSSS is 33 bytes, 430us for DSSS2 and 460 for DSSS1
                if (dataRateIndex == DSSS2)
                {
                    packetLengthUs = packetLengthUs - dsss2MinLengthUs;
                }
                else if (dataRateIndex == DSSS1)
                {
                    packetLengthUs = packetLengthUs - dsss1MinLengthUs;
                }
                else
                {
                    /*!< Capture time for measuring 11b (long preamble) EVM.  =192+1000/11+(34*8) mpdu */
                    /*!< Capture time for measuring 11b (short preamble) EVM. =96+1000/11+(34*8) mpdu  */
                    mpduTimeUs = (int)((double)minMpduInBits/dataRateMbps);

                    //When use the function for DUT transmit packetSize, we need to subtract minMpdu data length from the 
                    //transmitted data, otherwise, we get more data than what we want
                    if (preamble == PREAMBLE_TYPE_SHORT)
                        packetLengthUs = packetLengthUs - shortPreambleLengthUs - mpduTimeUs;
                    else
                        packetLengthUs = packetLengthUs - longPreambleLengthUs - mpduTimeUs;
                    //Transmit at least 500 chips when packet legth is 0 
                    if (packetLengthUs < 0)
                        packetLengthUs = 500/11;
                }
            }
            else if (dataRateIndex <= OFDM54)
            {
                /*!< Capture time for measuring 11ag EVM. =20+4*18 */
                packetLengthUs = packetLengthUs - 20 - 8;
            }
            else if (dataRateIndex <= MCS31)
            {
                /*!< Capture time for measuring 11n mixed format EVM. =20+12+4*4+4*18 */
                /*!< Capture time for measuring 11n greenfield EVM. =24+4*4+4*18 */
                if (packetType11n == PACKET_FORMAT_11N_MIXED)
                    packetLengthUs = packetLengthUs - 20 - 4 - 16;
                else
                    packetLengthUs = packetLengthUs - 24 - 16;
            }

            if (packetLengthUs < 0)
                packetLengthUs = 0;

            *numberOfBytes = (int)(((double)packetLengthUs*dataRateMbps)/8.0);
        }
        else
        {
            ret = vDUT_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = vDUT_ERR_PARAM_DOES_NOT_EXIST;
    }
    return ret;
}

vDUT_API vDUT_RETURN vDUT_AddArrayDoubleReturn(vDUT_ID dutID, const vDUT_STR paramName, const double *paramValue, const int arraySize)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        if( arraySize>0 )
        {
            vector<double> arrayDouble;
            for( int i=0; i<arraySize; i++)
            {
                arrayDouble.push_back( paramValue[i] );
            }
            g_arrayDoubleReturns[dutID].insert( arrayDoubleReturnPair(paramName, arrayDouble) );
        }
        else
        {
            vector<double> arrayDouble;
            g_arrayDoubleReturns[dutID].insert( arrayDoubleReturnPair(paramName, arrayDouble) );

        }
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }
    return ret;
}

vDUT_API vDUT_RETURN vDUT_GetArrayDoubleReturn(vDUT_ID dutID, const vDUT_STR paramName, double *paramValue, const int arraySize)
{
    vDUT_RETURN ret = vDUT_ERR_OK;

    map <string, vector<double> >::iterator arrayDoubleReturn_Iter;

    if( dutID>-1 && dutID<MAX_TECHNOLOGIES_COUNT )
    {
        arrayDoubleReturn_Iter = g_arrayDoubleReturns[dutID].find(paramName);
        if( arrayDoubleReturn_Iter!=g_arrayDoubleReturns[dutID].end() )
        {
            for(int i=0; i<min(arraySize,(int)arrayDoubleReturn_Iter->second.size()); i++)
            {
                paramValue[i] = arrayDoubleReturn_Iter->second[i];
            }
        }
        else
        {
            ret = vDUT_ERR_PARAM_DOES_NOT_EXIST;
        }
    }
    else
    {
        ret = vDUT_ERR_INVALID_DUT_ID;;
    }

    return ret;
}

vDUT_API vDUT_RETURN vDUT_ClearMiscValues(void)
{
	vDUT_RETURN ret = vDUT_ERR_OK;

	miscIntValues.clear();
	miscStringValues.clear();
	
	return ret;
}

vDUT_API vDUT_RETURN vDUT_SetMiscIntVal(const vDUT_STR paramName, const int mIntVal)
{

	vDUT_RETURN ret = vDUT_ERR_OK;

	map <string, int>::iterator intParam_Iter;

	intParam_Iter = miscIntValues.find(paramName);
	if( intParam_Iter!=miscIntValues.end() )
	{
		miscIntValues.erase(intParam_Iter);
	}
	
	miscIntValues.insert( intParamPair(paramName, mIntVal) );
	
	return ret;
}

vDUT_API vDUT_RETURN vDUT_GetMiscIntVal(const vDUT_STR paramName, int* pIntVal)
{
	vDUT_RETURN ret = vDUT_ERR_OK;

	map <string, int>::iterator intParam_Iter;

	intParam_Iter = miscIntValues.find(paramName);
	if( intParam_Iter!=miscIntValues.end() )
	{
		*pIntVal = intParam_Iter->second;
	}
	else
	{
		ret = vDUT_ERR_PARAM_DOES_NOT_EXIST;
	}

	return ret;
}

vDUT_API vDUT_RETURN vDUT_SetMiscStrVal(const vDUT_STR paramName, const vDUT_STR paramValue)
{

	vDUT_RETURN ret = vDUT_ERR_OK;

	map <string, string>::iterator stringReturn_Iter;

	stringReturn_Iter = miscStringValues.find(paramName);
	if( stringReturn_Iter!=miscStringValues.end() )
	{
		miscStringValues.erase(stringReturn_Iter);
	}
	
	miscStringValues.insert( stringReturnPair(paramName, paramValue) );
	
	return ret;
}


vDUT_API vDUT_RETURN vDUT_GetMiscStrVal(const vDUT_STR paramName, vDUT_STR paramValue, int bufferSize)
{
	vDUT_RETURN ret = vDUT_ERR_OK;

	map <string, string>::iterator stringReturn_Iter;

	stringReturn_Iter = miscStringValues.find(paramName);
	if( stringReturn_Iter!=miscStringValues.end() )
	{
		strcpy_s((char*)paramValue, bufferSize, (char*)stringReturn_Iter->second.c_str());
	}
	else
	{
		ret = vDUT_ERR_PARAM_DOES_NOT_EXIST;
	}
	
	return ret;
}
