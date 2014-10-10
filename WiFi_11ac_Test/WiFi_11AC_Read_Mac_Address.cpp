#include "stdafx.h"
#include "WiFi_11AC_Test.h"
#include "IQmeasure.h"

// This variable is declared in WiFi_Test_Internal.cpp
extern vDUT_ID      g_WiFi_11ac_Dut;
extern TM_ID        g_WiFi_11ac_Test_ID;
extern bool			g_vDutTxActived;

using namespace std;

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_11ACreadMacAddressParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_11ACreadMacAddressReturnMap;

struct tagParam
{
} l_11ACreadMacAddressParam;

struct tagReturn
{
    char MAC_ADDRESS[MAX_BUFFER_SIZE];           /*!< A string contains MAC address processed by DUT control.  */
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];         /*!< A string for error message. */
} l_11ACreadMacAddressReturn;

#ifndef WIN32
int init11ACReadMacAddressContainers = Initialize11ACReadMacAddressContainers();
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
WIFI_11AC_TEST_API int WIFI_11AC_Read_Mac_Address(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_11ACreadMacAddressReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_11ACreadMacAddressParamMap);
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
        RespondToQueryReturn(l_11ACreadMacAddressReturnMap);
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

		if ( g_vDutTxActived==true )
		{
		   /*-----------*
			*  Tx Stop  *
			*-----------*/
			err = ::vDUT_Run(g_WiFi_11ac_Dut, "TX_STOP");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(TX_STOP) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(TX_STOP) return OK.\n");
			}
		}
		else
		{
			// no need for TX_STOP				
		}

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_11ACreadMacAddressParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Get input parameters return OK.\n");
		}

        // Error return of this function is irrelevant
        CheckDutTransmitStatus();

		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_11ac_Dut);
	    
		err = vDUT_Run(g_WiFi_11ac_Dut, "READ_MAC_ADDRESS");		
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(READ_MAC_ADDRESS) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(READ_MAC_ADDRESS) return OK.\n");
		}

		// TODO: Example, get Return parameters here
		err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "MAC_ADDRESS", l_11ACreadMacAddressReturn.MAC_ADDRESS, MAX_BUFFER_SIZE);		
		if ( ERR_OK!=err )
		{
			err = ERR_OK;	// This is an optional return parameter, thus always return OK
			sprintf_s(l_11ACreadMacAddressReturn.MAC_ADDRESS, MAX_BUFFER_SIZE, "");
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetStringReturn(MAC_ADDRESS) return error.\n");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetStringReturn(MAC_ADDRESS) return OK.\n");
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_11ACreadMacAddressReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_11ACreadMacAddressReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_11ACreadMacAddressReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_11ACreadMacAddressReturn.ERROR_MESSAGE, "[WiFi_11AC] Unknown Error!\n");
		err = -1;
    }

    return err;
}

int Initialize11ACReadMacAddressContainers(void)
{
    /*------------------*
     * Input Parameters: *
     *------------------*/
    l_11ACreadMacAddressParamMap.clear();

    WIFI_SETTING_STRUCT setting;
    
    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_11ACreadMacAddressReturnMap.clear();

    l_11ACreadMacAddressReturn.MAC_ADDRESS[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACreadMacAddressReturn.MAC_ADDRESS))    // Type_Checking
    {
        setting.value       = (void*)l_11ACreadMacAddressReturn.MAC_ADDRESS;
        setting.unit        = "";
        setting.helpText    = "MAC address read from DUT.";
        l_11ACreadMacAddressReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("MAC_ADDRESS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACreadMacAddressReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACreadMacAddressReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_11ACreadMacAddressReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_11ACreadMacAddressReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}

