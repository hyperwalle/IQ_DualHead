#include "stdafx.h"
#include "TestManager.h"
#include "BT_Test.h"
#include "BT_Test_Internal.h"
#include "IQmeasure.h"
#include "StringUtil.h"

// This global variable is declared in BT_Test_Internal.cpp
extern TM_ID    g_BT_Test_ID;
extern int		g_Tester_Type;
extern int      g_Tester_Number;
extern int      g_Tester_Reconnect;
// This global variable is declared in BT_Global_Setting.cpp
extern BT_GLOBAL_SETTING g_BTGlobalSettingParam;
int             g_iTesterName = IQTYPE_2010;

using namespace std;

// Input Parameter Container
map<string, BT_SETTING_STRUCT> l_connectTesterParamMap;

// Return Value Container 
map<string, BT_SETTING_STRUCT> l_connectTesterReturnMap;

struct tagParam
{
    char    IQTESTER_IP[MAX_BUFFER_SIZE];
	int     IQTESTER_RECONNECT;
	int	    IQTESTER_TYPE;								/* < Tester type */
	int 	IQTESTER_CONTROL_METHOD;					/* IQAPI 0, Direct SCPI 1 */
} l_connectTesterParam;

struct tagReturn
{
	char    IQTESTER_INFO[MAX_BUFFER_SIZE];				/*!< IQtester information string, such as HW version, firmware version, etc. */
    char    IQTESTER_SERIAL_NUM[MAX_BUFFER_SIZE];       /*!< Tester's serial number */
    char    IQAPI_VERSION[MAX_BUFFER_SIZE];             /*!< IQapi version number */
    char    IQAPITC_VERSION[MAX_BUFFER_SIZE];           /*!< IQapitc library version */
    char    IQMEASURE_VERSION[MAX_BUFFER_SIZE];         /*!< IQmeasure library version */
    char    IQV_ANALYSIS_VERSION[MAX_BUFFER_SIZE];      /*!< IQv_analysis library version */
    char    IQV_MW_VERSION[MAX_BUFFER_SIZE];            /*!< IQv_middleware version */
    char    IQ_NXN_VERSION[MAX_BUFFER_SIZE];            /*!< IQnxn version */
    char    IQ_NXN_SIGNAL_VERSION[MAX_BUFFER_SIZE];     /*!< IQnxn signal library version */
    char    IQ_MAX_SIGNAL_VERSION[MAX_BUFFER_SIZE];     /*!< IQmax signal library version */
    char    IQ_ZIGBEE_SIGNAL_VERSION[MAX_BUFFER_SIZE];  /*!< IQzigbee signal library version */
    char    BLUETOOTH_VERSION[MAX_BUFFER_SIZE];         /*!< Bluetooth library version */
    char    TEST_MANAGER_VERSION[MAX_BUFFER_SIZE];		/*!< Information of the TestManager DLL, such as version */
    char	ERROR_MESSAGE[MAX_BUFFER_SIZE];				/*!< Error message */
	char    IQTESTER_HW_VERSION_01[MAX_BUFFER_SIZE];    /*!< Tester number 1 HW version */ 
    char    IQTESTER_HW_VERSION_02[MAX_BUFFER_SIZE];    /*!< Tester number 2 HW version */ 
    char    IQTESTER_HW_VERSION_03[MAX_BUFFER_SIZE];    /*!< Tester number 3 HW version */ 
    char    IQTESTER_HW_VERSION_04[MAX_BUFFER_SIZE];    /*!< Tester number 4 HW version */ 
} l_connectTesterReturn;

#ifndef WIN32
int initConnectTesterContainers = InitializeConnectTesterContainers();
#endif

int ClearConnectLPReturn(void)
{
	l_connectTesterParamMap.clear();
	l_connectTesterReturnMap.clear();
	return 0;
}

//! Connect to Tester (IQview/flex, IQmax, IQnxn, IQmimo, etc.)
/*!
 * Input Parameters
 *
 *  - Mandatory 
 *      -# A string that contains the IP address (or Serial Number) of the tester
 *
 * Return Values
 *      -# A string that contains all tester info, such as s/w versions, h/w serial number, etc.
 *      -# Tester number 1 serial number
 *      -# Tester number 2 serial number
 *      -# Tester number 3 serial number
 *      -# Tester number 4 serial number
 *      -# IQapi version
 *      -# IQapitc library version
 *      -# IQmeasure library version
 *      -# IQv_analysis library version
 *      -# IQv middle wear version
 *      -# IQnxn version
 *      -# IQnxn signal library version
 *      -# IQmax signal library version
 *      -# IQzigbee signal library version
 *      -# Bluetooth library version
 *      -# A string for error message
 *
 * \return 0 No error has occurred
 * \return -1 DUT failed to insert.  Please see the returned error message for details.
 */

BT_TEST_API int BT_Connect_IQTester(void)
{
    int		err = ERR_OK;	
    int		dummyValue = 0;
	char	logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*----------------------------------------*
     * Clear Return Parameters and Containers *
     *----------------------------------------*/
	ClearReturnParameters(l_connectTesterReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_connectTesterParamMap);
        return err;
    }
	else
	{
		// do nothing
	}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_connectTesterReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{

	   /*---------------------------------------*
		* g_BT_Test_ID need to be valid (>=0) *
		*---------------------------------------*/
		TM_ClearReturns(g_BT_Test_ID);
		if( g_BT_Test_ID<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] BT_Test_ID not valid.\n");
			throw logMessage;	
		}
		else
		{
			// do nothing
		}

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_connectTesterParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Input parameters are not complete.\n");
			throw logMessage;
		}

        g_Tester_Reconnect = l_connectTesterParam.IQTESTER_RECONNECT;
        if( (0==g_Tester_Number) || g_Tester_Reconnect )
        {
			// At least IP01 must be valid; otherwise, return error directly
			err = ValidateIPAddress(l_connectTesterParam.IQTESTER_IP);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Invalid IP address (%s).\n", l_connectTesterParam.IQTESTER_IP);
				throw logMessage;
			}

			// To initialize the IQtester environment
			/***** Modified by Jim 20120518 *****/
			if (l_connectTesterParam.IQTESTER_TYPE == IQTYPE_2010)
			{
				g_iTesterName = IQTYPE_2010;
				err = LP_Init(IQTYPE_2010);
			}
			else
			{
				g_iTesterName = IQTYPE_XEL;
				err = LP_Init(IQTYPE_XEL,l_connectTesterParam.IQTESTER_CONTROL_METHOD); 
			}
			/*
			err = LP_Init(IQTYPE_XEL,l_connectTesterParam.IQTESTER_CONTROL_METHOD); 
			*/
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Function LP_Init() Failed.\n");
				throw logMessage;
			}			

			// If tester initialization is okay, then validate other IP addresses
			g_Tester_Type   = IQ_View;
			g_Tester_Number = 1;
			err = LP_InitTester(l_connectTesterParam.IQTESTER_IP);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Function LP_InitTester(%s) Failed.\n", l_connectTesterParam.IQTESTER_IP);
				throw logMessage;
			}		

			// Call LP_SetVsa() for apply VSA and VSG port setting 
			err = LP_SetVsaBluetooth(  2402*1e6,
									   0,
									   g_BTGlobalSettingParam.VSA_PORT,
									   g_BTGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
									   g_BTGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000
									 );
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[BT] Fail to setup VSA, LP_SetVsa() return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] LP_SetVsa() return OK.\n");
			}

			// For retrieval of the IQtester version information
			err = LP_GetVersion(l_connectTesterReturn.IQTESTER_INFO, MAX_BUFFER_SIZE);		
			if (false==err)
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Failed to get IQTester version info.\n");
				throw logMessage;
			}
			else
			{
				err = 0;
			}

            // Parse version string to categorize the different test systems with their versions
            map<string, string> versionMap;
            ParseSplitString(l_connectTesterReturn.IQTESTER_INFO, ":", versionMap);

            strcpy_s( l_connectTesterReturn.IQTESTER_SERIAL_NUM, MAX_BUFFER_SIZE, versionMap["Tester 1 SN"].c_str() );
            strcpy_s( l_connectTesterReturn.IQAPI_VERSION, MAX_BUFFER_SIZE, versionMap["iqapi"].c_str() );
            strcpy_s( l_connectTesterReturn.IQAPITC_VERSION, MAX_BUFFER_SIZE, versionMap["iqapitc"].c_str() );
            strcpy_s( l_connectTesterReturn.IQMEASURE_VERSION, MAX_BUFFER_SIZE, versionMap["IQmeasure"].c_str() );
            strcpy_s( l_connectTesterReturn.IQV_ANALYSIS_VERSION, MAX_BUFFER_SIZE, versionMap["iqv_analysis"].c_str() );
            strcpy_s( l_connectTesterReturn.IQV_MW_VERSION, MAX_BUFFER_SIZE, versionMap["IQV_mw"].c_str() );
            strcpy_s( l_connectTesterReturn.IQ_NXN_VERSION, MAX_BUFFER_SIZE, versionMap["iq_nxn"].c_str() );
            strcpy_s( l_connectTesterReturn.IQ_NXN_SIGNAL_VERSION, MAX_BUFFER_SIZE, versionMap["iq_nxn_signal"].c_str() );
            strcpy_s( l_connectTesterReturn.IQ_MAX_SIGNAL_VERSION, MAX_BUFFER_SIZE, versionMap["iq_max_signal"].c_str() );
            strcpy_s( l_connectTesterReturn.IQ_ZIGBEE_SIGNAL_VERSION, MAX_BUFFER_SIZE, versionMap["iq_zigbee_signal"].c_str() );
            strcpy_s( l_connectTesterReturn.BLUETOOTH_VERSION, MAX_BUFFER_SIZE, versionMap["bluetooth"].c_str() );
            strcpy_s( l_connectTesterReturn.IQTESTER_HW_VERSION_01, MAX_BUFFER_SIZE, versionMap["Tester 1 hardware version"].c_str() );
            strcpy_s( l_connectTesterReturn.IQTESTER_HW_VERSION_02, MAX_BUFFER_SIZE, versionMap["Tester 2 hardware version"].c_str() );
            strcpy_s( l_connectTesterReturn.IQTESTER_HW_VERSION_03, MAX_BUFFER_SIZE, versionMap["Tester 3 hardware version"].c_str() );
            strcpy_s( l_connectTesterReturn.IQTESTER_HW_VERSION_04, MAX_BUFFER_SIZE, versionMap["Tester 4 hardware version"].c_str() );


			// For retrieval of the TestManager version information
			err = TM_GetVersion(l_connectTesterReturn.TEST_MANAGER_VERSION, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Failed to get TestManager version info.\n");
				throw logMessage;
			}
        }
        else
        {
            // Reconnect=0
            err = -1;

			// For retrieval of the IQtester version information
			err = LP_GetVersion(l_connectTesterReturn.IQTESTER_INFO, MAX_BUFFER_SIZE);		
			if (false==err)
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Failed to get IQTester version info.\n");
				throw logMessage;
			}
			else
			{
				err = 0;
			}

			// Parse version string to categorize the different test systems with their versions
            map<string, string> versionMap;
            ParseSplitString(l_connectTesterReturn.IQTESTER_INFO, ":", versionMap);

            strcpy_s( l_connectTesterReturn.IQTESTER_SERIAL_NUM, MAX_BUFFER_SIZE, versionMap["Tester 1 SN"].c_str() );
            strcpy_s( l_connectTesterReturn.IQAPI_VERSION, MAX_BUFFER_SIZE, versionMap["iqapi"].c_str() );
            strcpy_s( l_connectTesterReturn.IQAPITC_VERSION, MAX_BUFFER_SIZE, versionMap["iqapitc"].c_str() );
            strcpy_s( l_connectTesterReturn.IQMEASURE_VERSION, MAX_BUFFER_SIZE, versionMap["IQmeasure"].c_str() );
            strcpy_s( l_connectTesterReturn.IQV_ANALYSIS_VERSION, MAX_BUFFER_SIZE, versionMap["iqv_analysis"].c_str() );
            strcpy_s( l_connectTesterReturn.IQV_MW_VERSION, MAX_BUFFER_SIZE, versionMap["IQV_mw"].c_str() );
            strcpy_s( l_connectTesterReturn.IQ_NXN_VERSION, MAX_BUFFER_SIZE, versionMap["iq_nxn"].c_str() );
            strcpy_s( l_connectTesterReturn.IQ_NXN_SIGNAL_VERSION, MAX_BUFFER_SIZE, versionMap["iq_nxn_signal"].c_str() );
            strcpy_s( l_connectTesterReturn.IQ_MAX_SIGNAL_VERSION, MAX_BUFFER_SIZE, versionMap["iq_max_signal"].c_str() );
            strcpy_s( l_connectTesterReturn.IQ_ZIGBEE_SIGNAL_VERSION, MAX_BUFFER_SIZE, versionMap["iq_zigbee_signal"].c_str() );
            strcpy_s( l_connectTesterReturn.BLUETOOTH_VERSION, MAX_BUFFER_SIZE, versionMap["bluetooth"].c_str() );
            strcpy_s( l_connectTesterReturn.IQTESTER_HW_VERSION_01, MAX_BUFFER_SIZE, versionMap["Tester 1 hardware version"].c_str() );
            strcpy_s( l_connectTesterReturn.IQTESTER_HW_VERSION_02, MAX_BUFFER_SIZE, versionMap["Tester 2 hardware version"].c_str() );
            strcpy_s( l_connectTesterReturn.IQTESTER_HW_VERSION_03, MAX_BUFFER_SIZE, versionMap["Tester 3 hardware version"].c_str() );
            strcpy_s( l_connectTesterReturn.IQTESTER_HW_VERSION_04, MAX_BUFFER_SIZE, versionMap["Tester 4 hardware version"].c_str() );


			// For retrieval of the TestManager version information
			err = TM_GetVersion(l_connectTesterReturn.TEST_MANAGER_VERSION, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Failed to get TestManager version info.\n");
				throw logMessage;
			}
        }

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_connectTesterReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_connectTesterReturnMap);
		}
		else
		{
			// do nothing
		}
	}
    catch(char *msg)
    {
        ReturnErrorMessage(l_connectTesterReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_connectTesterReturn.ERROR_MESSAGE, "[BT] Unknown Error!\n");
		err = -1;
    }

    return err;
}

int InitializeConnectTesterContainers(void)
{
    /*------------------*
     * Input Paramters: *
     * IQTESTER_IP01    *
     *------------------*/
    l_connectTesterParamMap.clear();

    BT_SETTING_STRUCT setting;

    strcpy_s(l_connectTesterParam.IQTESTER_IP, MAX_BUFFER_SIZE, "192.168.100.254");
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterParam.IQTESTER_IP))    // Type_Checking
    {
        setting.value       = (void*)l_connectTesterParam.IQTESTER_IP;
        setting.unit        = "";
        setting.helpText    = "IP address of LitePoint tester";
        l_connectTesterParamMap.insert( pair<string,BT_SETTING_STRUCT>("IQTESTER_IP", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterParam.IQTESTER_RECONNECT = 0;
    setting.type = BT_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_connectTesterParam.IQTESTER_RECONNECT))    // Type_Checking
    {
        setting.value       = (void*)&l_connectTesterParam.IQTESTER_RECONNECT;
        setting.unit        = "";
        setting.helpText    = "A flag to control reconnect of IQ tester.\r\n1-Reconnect;0-No reconnect; Default=0";
        l_connectTesterParamMap.insert( pair<string, BT_SETTING_STRUCT>("IQTESTER_RECONNECT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	
	/*-------------------*
	 * Input Parameters:  *
	 * IQTESTER_TYPE*
	 *-------------------*/

	l_connectTesterParam.IQTESTER_TYPE = IQTYPE_2010;
	setting.type = BT_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_connectTesterParam.IQTESTER_TYPE))	// Type_Checking
	{
		setting.value		= (void*)&l_connectTesterParam.IQTESTER_TYPE;
		setting.unit		= "";
		setting.helpText	= "Tester type.0: non-IQXel;  1: IQXel.\r\n Default vaule: 0 ";
		l_connectTesterParamMap.insert( pair<string,BT_SETTING_STRUCT>("IQTESTER_TYPE", setting) );
	}
	else	
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*-------------------*
	 * Input Parameters:  *
	 * IQTESTER_CONTROL_METHOD*
	 *-------------------*/

	l_connectTesterParam.IQTESTER_CONTROL_METHOD = TESTER_CONTROL_IQAPI;
	setting.type = BT_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_connectTesterParam.IQTESTER_CONTROL_METHOD))	// Type_Checking
	{
		setting.value		= (void*)&l_connectTesterParam.IQTESTER_CONTROL_METHOD;
		setting.unit		= "";
		setting.helpText	= "Tester Control Method. 0: IQAPI; 1: SCPI.\r\n Default vaule: 0 ";
		l_connectTesterParamMap.insert( pair<string,BT_SETTING_STRUCT>("IQTESTER_CONTROL_METHOD", setting) );
	}
	else	
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_connectTesterReturnMap.clear();

	l_connectTesterReturn.IQTESTER_INFO[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQTESTER_INFO))    // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQTESTER_INFO;
        setting.unit        = "";
        setting.helpText    = "IQTester info, such as HW version, firmware version, etc.";
        l_connectTesterReturnMap.insert( pair<string,BT_SETTING_STRUCT>("IQTESTER_INFO", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQTESTER_SERIAL_NUM[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQTESTER_SERIAL_NUM))  // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQTESTER_SERIAL_NUM;
        setting.unit        = "";
        setting.helpText    = "Tester's serial number";
        l_connectTesterReturnMap.insert( pair<string,BT_SETTING_STRUCT>("IQTESTER_SERIAL_NUM", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQAPI_VERSION[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQAPI_VERSION))   // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQAPI_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQapi version";
        l_connectTesterReturnMap.insert( pair<string,BT_SETTING_STRUCT>("IQAPI_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQAPITC_VERSION[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQAPITC_VERSION)) // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQAPITC_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQapitc library version";
        l_connectTesterReturnMap.insert( pair<string,BT_SETTING_STRUCT>("IQAPITC_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQMEASURE_VERSION[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQMEASURE_VERSION))   // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQMEASURE_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQmeasure library version";
        l_connectTesterReturnMap.insert( pair<string,BT_SETTING_STRUCT>("IQMEASURE_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQV_ANALYSIS_VERSION[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQV_ANALYSIS_VERSION))    // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQV_ANALYSIS_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQv_analysis library version";
        l_connectTesterReturnMap.insert( pair<string,BT_SETTING_STRUCT>("IQV_ANALYSIS_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQV_MW_VERSION[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQV_MW_VERSION))  // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQV_MW_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQv middle wear version";
        l_connectTesterReturnMap.insert( pair<string,BT_SETTING_STRUCT>("IQV_MW_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQ_NXN_VERSION[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQ_NXN_VERSION))  // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQ_NXN_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQnxn version";
        l_connectTesterReturnMap.insert( pair<string,BT_SETTING_STRUCT>("IQ_NXN_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQ_NXN_SIGNAL_VERSION[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQ_NXN_SIGNAL_VERSION))   // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQ_NXN_SIGNAL_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQnxn signal library version";
        l_connectTesterReturnMap.insert( pair<string,BT_SETTING_STRUCT>("IQ_NXN_SIGNAL_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQ_MAX_SIGNAL_VERSION[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQ_MAX_SIGNAL_VERSION))   // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQ_MAX_SIGNAL_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQmax signal library version";
        l_connectTesterReturnMap.insert( pair<string,BT_SETTING_STRUCT>("IQ_MAX_SIGNAL_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQ_ZIGBEE_SIGNAL_VERSION[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQ_ZIGBEE_SIGNAL_VERSION))    // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQ_ZIGBEE_SIGNAL_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQzigbee signal library version";
        l_connectTesterReturnMap.insert( pair<string,BT_SETTING_STRUCT>("IQ_ZIGBEE_SIGNAL_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.BLUETOOTH_VERSION[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.BLUETOOTH_VERSION))   // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.BLUETOOTH_VERSION;
        setting.unit        = "";
        setting.helpText    = "Bluetooth library version";
        l_connectTesterReturnMap.insert( pair<string,BT_SETTING_STRUCT>("BLUETOOTH_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.TEST_MANAGER_VERSION[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.TEST_MANAGER_VERSION))    // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.TEST_MANAGER_VERSION;
        setting.unit        = "";
        setting.helpText    = "Information of the TestManager DLL, such as version.";
        l_connectTesterReturnMap.insert( pair<string,BT_SETTING_STRUCT>("TEST_MANAGER_VERSION", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_connectTesterReturnMap.insert( pair<string,BT_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}

int ValidateIPAddress(char* ipAddress)
{
    int err = ERR_OK;

    if ( ipAddress!=NULL )
    {
		if ( 0!=strcmp(ipAddress,"") ) 
		{
			string			serialNumber = ipAddress;
			vector<string>	splits;


			splits.clear();
			SplitString(ipAddress, splits, ":");
			if( splits.size()==3 )
			{
				//Format is assumed to be right for LP_InitTester with IQ2010 testers.

			}
			else if(serialNumber.find("IQ") == 0)
			{
				//Serial number.  Add the correct formating to the string.
				string fomatString = "127.0.0.1:4001:" + serialNumber;
				strcpy_s(ipAddress, fomatString.size()+1, fomatString.c_str());
			}
			else
			{
				// Assume it is a valid IP address, don't need to check the IP format here.
				// Because the LP_InitTester function will take care the error return.
			}
		}
        else
        {
            // IP = "", return error
            err = ERR_INVALID_IP_ADDRESS;
        }
    }
    else
    {
        // IP = NULL, return error
        err = ERR_INVALID_IP_ADDRESS;
    }

    return err;
}

void ParseSplitString(const std::string &splitteValue, const std::string spliteString, std::map<std::string, std::string> &spliteMap)
{
    /*
    IQmeasure: 1.7.12.1.2 (2009-05-29)
    iqapi:      1.7.12.1 (May 19  2009)
    iqapitc:     1.2.28 May 15, 2009 
    iq_nxn:      1.1.5g (2009-05-13)
    IQV_mw:      3.2.5.f Sep 08, 2008
    Tester 1 SN: IQV08441
    Tester 2 SN: IQV08185
    iq_nxn_signal:    1.1.14h (2008-08-07)
    iq_max_signal:    1.0.46 (2008-08-26)
    iq_zigbee_signal:    1.0.7 (2009-01-23)
    bluetooth: 1.1.1.a, Febr 5, 2009
    iqv_analysis: 1.4.0.v Jan 29, 2009
    */
    string::size_type start = 0;
    string::size_type end = splitteValue.find( '\n', start );
    while( string::npos != end )
    {
        string line = splitteValue.substr(start, end - start);

		string::size_type delimiter = line.find(spliteString);
        string ver = line.substr(0, delimiter);
        string val = line.substr(delimiter + 1);

        spliteMap[Trim( ver )] = Trim( val );

        start = end + 1;
        end = splitteValue.find( '\n', start );
    }
}
