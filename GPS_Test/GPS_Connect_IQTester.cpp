#include "stdafx.h"
#include "TestManager.h"
#include "GPS_Test.h"
#include "GPS_Test_Internal.h"
#include "IQmeasure.h"
#include "StringUtil.h"

// This global variable is declared in GPS_Test_Internal.cpp
extern TM_ID    g_GPS_Test_ID;
extern int		g_Tester_Type;
extern int      g_Tester_Number;
extern int      g_Tester_Reconnect;
// This global variable is declared in GPS_Global_Setting.cpp
extern GPS_GLOBAL_SETTING g_GPSGlobalSettingParam;

using namespace std;

// Input Parameter Container
map<string, GPS_SETTING_STRUCT> l_connectTesterParamMap;

// Return Value Container 
map<string, GPS_SETTING_STRUCT> l_connectTesterReturnMap;

struct tagParam
{
    char    IQTESTER_IP[MAX_BUFFER_SIZE];
	int     IQTESTER_RECONNECT;
} l_connectTesterParam;

struct tagReturn
{
	char    IQLITE_IQTESTER_INFO[MAX_BUFFER_SIZE];				/*!< IQtester information string, such as HW version, firmware version, etc. */
    char    IQLITE_IQTESTER_SERIAL_NUM[MAX_BUFFER_SIZE];       /*!< Tester's serial number */
    char    IQLITE_IQAPI_VERSION[MAX_BUFFER_SIZE];             /*!< IQapi version number */
    char    IQLITE_IQAPITC_VERSION[MAX_BUFFER_SIZE];           /*!< IQapitc library version */
    char    IQLITE_IQMEASURE_VERSION[MAX_BUFFER_SIZE];         /*!< IQmeasure library version */
    char    IQLITE_IQV_ANALYSIS_VERSION[MAX_BUFFER_SIZE];      /*!< IQv_analysis library version */
    char    IQLITE_IQV_MW_VERSION[MAX_BUFFER_SIZE];            /*!< IQv_middleware version */
    char    IQLITE_IQ_NXN_VERSION[MAX_BUFFER_SIZE];            /*!< IQnxn version */
    char    IQLITE_IQ_NXN_SIGNAL_VERSION[MAX_BUFFER_SIZE];     /*!< IQnxn signal library version */
    char    IQLITE_IQ_MAX_SIGNAL_VERSION[MAX_BUFFER_SIZE];     /*!< IQmax signal library version */
    char    IQLITE_IQ_ZIGBEE_SIGNAL_VERSION[MAX_BUFFER_SIZE];  /*!< IQzigbee signal library version */
    char    IQLITE_BLUETOOTH_VERSION[MAX_BUFFER_SIZE];         /*!< Bluetooth library version */
    char    IQLITE_TEST_MANAGER_VERSION[MAX_BUFFER_SIZE];		/*!< Information of the TestManager DLL, such as version */
    char	IQLITE_ERROR_MESSAGE[MAX_BUFFER_SIZE];				/*!< Error message */
	char    IQLITE_IQTESTER_HW_VERSION_01[MAX_BUFFER_SIZE];    /*!< Tester number 1 HW version */ 
    char    IQLITE_IQTESTER_HW_VERSION_02[MAX_BUFFER_SIZE];    /*!< Tester number 2 HW version */ 
    char    IQLITE_IQTESTER_HW_VERSION_03[MAX_BUFFER_SIZE];    /*!< Tester number 3 HW version */ 
    char    IQLITE_IQTESTER_HW_VERSION_04[MAX_BUFFER_SIZE];    /*!< Tester number 4 HW version */ 
} l_connectTesterReturn;

void ClearConnectLPReturn(void)
{
	l_connectTesterParamMap.clear();
	l_connectTesterReturnMap.clear();
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

GPS_TEST_API int GPS_Connect_IQTester(void)
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
    err = TM_GetIntegerParameter(g_GPS_Test_ID, "QUERY_INPUT", &dummyValue);
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
    err = TM_GetIntegerParameter(g_GPS_Test_ID, "QUERY_RETURN", &dummyValue);
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
		* g_GPS_Test_ID need to be valid (>=0) *
		*---------------------------------------*/
		TM_ClearReturns(g_GPS_Test_ID);
		if( g_GPS_Test_ID<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] GPS_Test_ID not valid.\n");
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
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] Input parameters are not complete.\n");
			throw logMessage;
		}

        g_Tester_Reconnect = l_connectTesterParam.IQTESTER_RECONNECT;

        if( (0==g_Tester_Number) || g_Tester_Reconnect )
        {
			// At least IP01 must be valid; otherwise, return error directly
			err = ValidateIPAddress(l_connectTesterParam.IQTESTER_IP);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] Invalid IP address (%s).\n", l_connectTesterParam.IQTESTER_IP);
				throw logMessage;
			}

			// To initialize the IQtester environment
			err = LP_Init();
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] Function LP_Init() Failed.\n");
				throw logMessage;
			}			

			// If tester initialization is okay, then validate other IP addresses
			g_Tester_Type   = IQ_View;
			g_Tester_Number = 1;
			err = LP_InitTester(l_connectTesterParam.IQTESTER_IP);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] Function LP_InitTester(%s) Failed.\n", l_connectTesterParam.IQTESTER_IP);
				throw logMessage;
			}		

			

			// For retrieval of the IQtester version information
			err = LP_GetVersion(l_connectTesterReturn.IQLITE_IQTESTER_INFO, MAX_BUFFER_SIZE);		
			if (false==err)
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] Failed to get IQTester version info.\n");
				throw logMessage;
			}
			else
			{
				err = 0;
			}

            // Parse version string to categorize the different test systems with their versions
            map<string, string> versionMap;
            ParseSplitString(l_connectTesterReturn.IQLITE_IQTESTER_INFO, ":", versionMap);

            strcpy_s( l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM, MAX_BUFFER_SIZE, versionMap["Tester 1 SN"].c_str() );
            strcpy_s( l_connectTesterReturn.IQLITE_IQAPI_VERSION, MAX_BUFFER_SIZE, versionMap["iqapi"].c_str() );
            strcpy_s( l_connectTesterReturn.IQLITE_IQAPITC_VERSION, MAX_BUFFER_SIZE, versionMap["iqapitc"].c_str() );
            strcpy_s( l_connectTesterReturn.IQLITE_IQMEASURE_VERSION, MAX_BUFFER_SIZE, versionMap["IQmeasure"].c_str() );
            strcpy_s( l_connectTesterReturn.IQLITE_IQV_ANALYSIS_VERSION, MAX_BUFFER_SIZE, versionMap["iqv_analysis"].c_str() );
            strcpy_s( l_connectTesterReturn.IQLITE_IQV_MW_VERSION, MAX_BUFFER_SIZE, versionMap["IQV_mw"].c_str() );
            strcpy_s( l_connectTesterReturn.IQLITE_IQ_NXN_VERSION, MAX_BUFFER_SIZE, versionMap["iq_nxn"].c_str() );
            strcpy_s( l_connectTesterReturn.IQLITE_IQ_NXN_SIGNAL_VERSION, MAX_BUFFER_SIZE, versionMap["iq_nxn_signal"].c_str() );
            strcpy_s( l_connectTesterReturn.IQLITE_IQ_MAX_SIGNAL_VERSION, MAX_BUFFER_SIZE, versionMap["iq_max_signal"].c_str() );
            strcpy_s( l_connectTesterReturn.IQLITE_IQ_ZIGBEE_SIGNAL_VERSION, MAX_BUFFER_SIZE, versionMap["iq_zigbee_signal"].c_str() );
            strcpy_s( l_connectTesterReturn.IQLITE_BLUETOOTH_VERSION, MAX_BUFFER_SIZE, versionMap["bluetooth"].c_str() );
            strcpy_s( l_connectTesterReturn.IQLITE_IQTESTER_HW_VERSION_01, MAX_BUFFER_SIZE, versionMap["Tester Hardware version"].c_str() );
            strcpy_s( l_connectTesterReturn.IQLITE_IQTESTER_HW_VERSION_02, MAX_BUFFER_SIZE, versionMap["Tester Hardware version"].c_str() );
            strcpy_s( l_connectTesterReturn.IQLITE_IQTESTER_HW_VERSION_03, MAX_BUFFER_SIZE, versionMap["Tester Hardware version"].c_str() );
            strcpy_s( l_connectTesterReturn.IQLITE_IQTESTER_HW_VERSION_04, MAX_BUFFER_SIZE, versionMap["Tester Hardware version"].c_str() );

			// IQ2010 trigger setting tweak
			if (strstr(l_connectTesterReturn.IQLITE_IQTESTER_HW_VERSION_01, "1.7.") && g_GPSGlobalSettingParam.VSA_TRIGGER_TYPE == 6)
            {
                g_GPSGlobalSettingParam.VSA_TRIGGER_TYPE = 13; //IQV_TRIG_TYPE_FOR_IQ2010
                
            }
            else
            {
                //do nothing, using the global setting input from user
            }

			// For retrieval of the TestManager version information
			err = TM_GetVersion(l_connectTesterReturn.IQLITE_TEST_MANAGER_VERSION, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] Failed to get TestManager version info.\n");
				throw logMessage;
			}
        }
        else
        {
            // Reconnect=0
            err = ERR_OK;
        }

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_connectTesterReturn.IQLITE_ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_connectTesterReturnMap);
		}
		else
		{
			// do nothing
		}
	}
    catch(char *msg)
    {
        ReturnErrorMessage(l_connectTesterReturn.IQLITE_ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_connectTesterReturn.IQLITE_ERROR_MESSAGE, "[GPS] Unknown Error!\n");
		err = -1;
    }

    return err;
}

void InitializeConnectTesterContainers(void)
{
    /*------------------*
     * Input Paramters: *
     * IQTESTER_IP01    *
     *------------------*/
    l_connectTesterParamMap.clear();

    GPS_SETTING_STRUCT setting;

    strcpy_s(l_connectTesterParam.IQTESTER_IP, MAX_BUFFER_SIZE, "127.0.0.1");
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterParam.IQTESTER_IP))    // Type_Checking
    {
        setting.value       = (void*)l_connectTesterParam.IQTESTER_IP;
        setting.unit        = "";
        setting.helpText    = "IP address of LitePoint tester";
        l_connectTesterParamMap.insert( pair<string,GPS_SETTING_STRUCT>("IQTESTER_IP", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterParam.IQTESTER_RECONNECT = 0;
    setting.type = GPS_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_connectTesterParam.IQTESTER_RECONNECT))    // Type_Checking
    {
        setting.value       = (void*)&l_connectTesterParam.IQTESTER_RECONNECT;
        setting.unit        = "";
        setting.helpText    = "A flag to control reconnect of IQ tester.\r\n1-Reconnect;0-No reconnect; Default=0";
        l_connectTesterParamMap.insert( pair<string, GPS_SETTING_STRUCT>("IQTESTER_RECONNECT", setting) );
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

	l_connectTesterReturn.IQLITE_IQTESTER_INFO[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQTESTER_INFO))    // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQLITE_IQTESTER_INFO;
        setting.unit        = "";
        setting.helpText    = "IQTester info, such as HW version, firmware version, etc.";
        l_connectTesterReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("IQTESTER_INFO", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM))  // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM;
        setting.unit        = "";
        setting.helpText    = "Tester's serial number";
        l_connectTesterReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("IQTESTER_SERIAL_NUM", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQLITE_IQAPI_VERSION[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQAPI_VERSION))   // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQLITE_IQAPI_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQapi version";
        l_connectTesterReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("IQAPI_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQLITE_IQAPITC_VERSION[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQAPITC_VERSION)) // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQLITE_IQAPITC_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQapitc library version";
        l_connectTesterReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("IQAPITC_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQLITE_IQMEASURE_VERSION[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQMEASURE_VERSION))   // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQLITE_IQMEASURE_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQmeasure library version";
        l_connectTesterReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("IQMEASURE_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQLITE_IQV_ANALYSIS_VERSION[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQV_ANALYSIS_VERSION))    // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQLITE_IQV_ANALYSIS_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQv_analysis library version";
        l_connectTesterReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("IQV_ANALYSIS_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQLITE_IQV_MW_VERSION[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQV_MW_VERSION))  // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQLITE_IQV_MW_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQv middle wear version";
        l_connectTesterReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("IQV_MW_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQLITE_IQ_NXN_VERSION[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQ_NXN_VERSION))  // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQLITE_IQ_NXN_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQnxn version";
        l_connectTesterReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("IQ_NXN_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQLITE_IQ_NXN_SIGNAL_VERSION[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQ_NXN_SIGNAL_VERSION))   // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQLITE_IQ_NXN_SIGNAL_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQnxn signal library version";
        l_connectTesterReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("IQ_NXN_SIGNAL_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQLITE_IQ_MAX_SIGNAL_VERSION[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQ_MAX_SIGNAL_VERSION))   // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQLITE_IQ_MAX_SIGNAL_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQmax signal library version";
        l_connectTesterReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("IQ_MAX_SIGNAL_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQLITE_IQ_ZIGBEE_SIGNAL_VERSION[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQ_ZIGBEE_SIGNAL_VERSION))    // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQLITE_IQ_ZIGBEE_SIGNAL_VERSION;
        setting.unit        = "";
        setting.helpText    = "IQzigbee signal library version";
        l_connectTesterReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("IQ_ZIGBEE_SIGNAL_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQLITE_BLUETOOTH_VERSION[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_BLUETOOTH_VERSION))   // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQLITE_BLUETOOTH_VERSION;
        setting.unit        = "";
        setting.helpText    = "Bluetooth library version";
        l_connectTesterReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("BLUETOOTH_VERSION", setting) );
    }
    else
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQLITE_TEST_MANAGER_VERSION[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_TEST_MANAGER_VERSION))    // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQLITE_TEST_MANAGER_VERSION;
        setting.unit        = "";
        setting.helpText    = "Information of the TestManager DLL, such as version.";
        l_connectTesterReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("TEST_MANAGER_VERSION", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_connectTesterReturn.IQLITE_ERROR_MESSAGE[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_connectTesterReturn.IQLITE_ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_connectTesterReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return;
}

int ValidateIPAddress(char* ipAddress)
{
    int err = ERR_OK;

    if ( ipAddress!=NULL )
    {
        if ( 0!=strcmp(ipAddress,"") ) 
        {
            //Assume it is a valid IP address; The IP format does not have to be checked because the LP_InitTester function provides the error return.
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
