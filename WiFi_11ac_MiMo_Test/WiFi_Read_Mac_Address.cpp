#include "stdafx.h"
#include "WiFi_11ac_MiMo_Test.h"
#include "IQmeasure.h"

// This variable is declared in WiFi_Test_Internal.cpp
extern vDUT_ID      g_WiFi_Dut;
extern TM_ID        g_WiFi_Test_ID;

using namespace std;

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_readMacAddressParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_readMacAddressReturnMap;

struct tagParam
{
} l_readMacAddressParam;

struct tagReturn
{
    char MAC_ADDRESS[MAX_BUFFER_SIZE];           /*!< A string contains MAC address processed by DUT control.  */
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];         /*!< A string for error message. */
} l_readMacAddressReturn;

#ifndef WIN32
int initReadMacAddressMIMOContainers = InitializeReadMacAddressContainers();
#endif

//! WiFi_Read_Mac_Address
/*!
 * Input Parameters
 *
 *  - None 
 *
 * Return Values
 *      -# A string contains the generated MAC address
 *      -# A string for error message
 *
 * \return 0 No error occurred
 * \return -1 Failed
 */
WIFI_11AC_MIMO_TEST_API int WiFi_Read_Mac_Address(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_readMacAddressReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_readMacAddressParamMap);
        return err;
    }
	else
	{
		// do nothing
	}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_readMacAddressReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
	   /*-----------------------------------------------------------*
		* Both g_WiFi_Test_ID and g_WiFi_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		if( g_WiFi_Test_ID<0 || g_WiFi_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] WiFi_Test_ID or WiFi_Dut not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_Test_ID, g_WiFi_Dut);
		}
		
		TM_ClearReturns(g_WiFi_Test_ID);

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_readMacAddressParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Get input parameters return OK.\n");
		}

        // Error return of this function is irrelevant
        CheckDutStatus();

		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_Dut);
	    
		err = vDUT_Run(g_WiFi_Dut, "READ_MAC_ADDRESS");		
		if ( ERR_OK!=err )
		{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				throw logMessage;
			}
			else	// Just return normal error message in this case
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(READ_MAC_ADDRESS) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(READ_MAC_ADDRESS) return OK.\n");
		}

		// TODO: Example, get Return parameters here
		err = ::vDUT_GetStringReturn(g_WiFi_Dut, "MAC_ADDRESS", l_readMacAddressReturn.MAC_ADDRESS, MAX_BUFFER_SIZE);		
		if ( ERR_OK!=err )
		{
			err = ERR_OK;	// This is an optional return parameter, thus always return OK
			sprintf_s(l_readMacAddressReturn.MAC_ADDRESS, MAX_BUFFER_SIZE, "");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetStringReturn(MAC_ADDRESS) return OK.\n");
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_readMacAddressReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_readMacAddressReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_readMacAddressReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_readMacAddressReturn.ERROR_MESSAGE, "[WiFi_11ac_MiMo] Unknown Error!\n");
		err = -1;
    }

    return err;
}

int InitializeReadMacAddressContainers(void)
{
    /*------------------*
     * Input Parameters: *
     *------------------*/
    l_readMacAddressParamMap.clear();

    WIFI_SETTING_STRUCT setting;
    
    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_readMacAddressReturnMap.clear();

    l_readMacAddressReturn.MAC_ADDRESS[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_readMacAddressReturn.MAC_ADDRESS))    // Type_Checking
    {
        setting.value       = (void*)l_readMacAddressReturn.MAC_ADDRESS;
        setting.unit        = "";
        setting.helpText    = "MAC address read from DUT.";
        l_readMacAddressReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("MAC_ADDRESS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_readMacAddressReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_readMacAddressReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_readMacAddressReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_readMacAddressReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}

