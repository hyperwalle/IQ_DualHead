#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_11AC_Test.h"
#include "WiFi_11AC_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

// This variable is declared in WiFi_Test_Internal.cpp
extern vDUT_ID      g_WiFi_11ac_Dut;
extern TM_ID        g_WiFi_11ac_Test_ID;

using namespace std;

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_11ACinitializeDutParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_11ACinitializeDutReturnMap;

struct tagParam
{
} l_11ACinitializeDutParam;

struct tagReturn
{
	char DUT_NAME[MAX_BUFFER_SIZE];				/*!< DUT name. */
    char DUT_DRIVER_INFO[MAX_BUFFER_SIZE];		/*!< DUT driver info, such as driver version, firmware version, etc. */

    char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_11ACinitializeDutReturn;


#ifndef WIN32 
int init11ACInitializeDutContainers = Initialize11ACInitializeDutContainers();
#endif

//! Initialize WiFi DUT
/*!
 * Input Parameters
 *
 *  - Mandatory 
 *      -# A string that is used to determine which DUT register function will be used
 *
 * Return Values
 *      -# A string that contains all DUT info, such as FW version, driver version, chip revision, etc.
 *      -# A string for error message
 *
 * \return 0 No error occurred
 * \return -1 DUT failed to insert.  Please see the returned error message for details
 */
WIFI_11AC_TEST_API int WIFI_11AC_Initialize_Dut(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_11ACinitializeDutReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_11ACinitializeDutParamMap);
        return err;
    }
	else
	{
		// do nothing
	}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_11ACinitializeDutReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
	   /*-----------------------------------------------------------*
		* Both g_WiFi_11ac_Test_ID and g_WiFi_11ac_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		if( g_WiFi_11ac_Test_ID<0 || g_WiFi_11ac_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] WiFi_Test_ID or WiFi_Dut not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_11ac_Test_ID, g_WiFi_11ac_Dut);
		}
		
		TM_ClearReturns(g_WiFi_11ac_Test_ID);

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_11ACinitializeDutParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Get input parameters return OK.\n");
		}

		// Initialize DUT
		err = vDUT_Run(g_WiFi_11ac_Dut, "INITIALIZE_DUT");
		if ( ERR_OK!=err )
		{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				throw logMessage;
			}
			else	// Just return normal error message in this case
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(INITIALIZE_DUT) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(INITIALIZE_DUT) return OK.\n");
		}
	        
		// For retrieval of the DUT info
		err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "DUT_NAME", l_11ACinitializeDutReturn.DUT_NAME, MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			err = ERR_OK;	// This is an optional return parameter, thus always return OK
			sprintf_s(l_11ACinitializeDutReturn.DUT_NAME, MAX_BUFFER_SIZE, "No Return, Unknown Dut Name.\n");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetStringReturn(DUT_NAME) return OK.\n");
		}

		err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "DUT_DRIVER_INFO", l_11ACinitializeDutReturn.DUT_DRIVER_INFO, MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			err = ERR_OK;	// This is an optional return parameter, thus always return OK
			sprintf_s(l_11ACinitializeDutReturn.DUT_DRIVER_INFO, MAX_BUFFER_SIZE, "No Return, Unknown Dut Driver Info.\n");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetStringReturn(DUT_DRIVER_INFO) return OK.\n");
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_11ACinitializeDutReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_11ACinitializeDutReturnMap);
		}
		else
		{
			// do nothing
		}
	}
    catch(char *msg)
    {
        ReturnErrorMessage(l_11ACinitializeDutReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_11ACinitializeDutReturn.ERROR_MESSAGE, "[WiFi_11AC] Unknown Error!\n");
		err = -1;
    }

    return err;
}

int Initialize11ACInitializeDutContainers(void)
{
    /*------------------*
     * Input Parameters: *
     *------------------*/
    l_11ACinitializeDutParamMap.clear();

    WIFI_SETTING_STRUCT setting;
      
    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_11ACinitializeDutReturnMap.clear();

    l_11ACinitializeDutReturn.DUT_NAME[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACinitializeDutReturn.DUT_NAME))    // Type_Checking
    {
        setting.value       = (void*)l_11ACinitializeDutReturn.DUT_NAME;
        setting.unit        = "";
        setting.helpText    = "DUT name.";
        l_11ACinitializeDutReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("DUT_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACinitializeDutReturn.DUT_DRIVER_INFO[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACinitializeDutReturn.DUT_DRIVER_INFO))    // Type_Checking
    {
        setting.value       = (void*)l_11ACinitializeDutReturn.DUT_DRIVER_INFO;
        setting.unit        = "";
        setting.helpText    = "DUT driver info, such as driver version, firmware version, etc.";
        l_11ACinitializeDutReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("DUT_DRIVER_INFO", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACinitializeDutReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACinitializeDutReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_11ACinitializeDutReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_11ACinitializeDutReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}

