#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_Test.h"
#include "WiFi_Test_Internal.h"
#include "IQmeasure.h"
#include "vDut.h"

using namespace std;

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_vDutEnabledParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_vDutEnabledReturnMap;

void ClearDutEnabledReturn(void)
{
	l_vDutEnabledParamMap.clear();
	l_vDutEnabledReturnMap.clear();
}

// WiFi_vDut_Enabled() does not require any inputs

struct tagReturn
{
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_vDutEnabledReturn;

//! Special Function - Enable the vDut control layer
/*!
 * Input Parameters
 *
 *  - None
 *
 * Return Values
 *      -# ERROR_MESSAGE (char):			A string for error message 
 *
 * \return 0 No error occurred
 * \return -1 DUT failed to insert.  Please see the returned error message for details
 */
WIFI_TEST_API int WiFi_vDut_Enabled(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_vDutEnabledReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        TM_ClearReturns(g_WiFi_Test_ID);
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
        RespondToQueryReturn(l_vDutEnabledReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
	   /*---------------------------------------*
		* g_WiFi_Test_ID need to be valid (>=0) *
		*---------------------------------------*/		
		if( g_WiFi_Test_ID<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] WiFi_Test_ID not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] WiFi_Test_ID = %d.\n", g_WiFi_Test_ID);
		}

		TM_ClearReturns(g_WiFi_Test_ID);

	   /*-------------------------------------------------------------------*
		* Turn on the Dut control, 0 means bypass, 1 means need Dut control *
		*-------------------------------------------------------------------*/
		err = vDUT_ByPassDutControl(1);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to enable vDut control, vDUT_ByPassDutControl(1) return error.\n");
			throw logMessage;
		}
		else
		{
			g_vDutControlDisabled = false;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_ByPassDutControl(1) successful.\n");
		} 

	   /*------------------------------*
		* Set g_WiFi_Dut = dummy value *
		*------------------------------*/	
		if ( 99999==g_WiFi_Dut )
		{
			g_WiFi_Dut = -1;
		}
		else
		{
			// do nothing
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_vDutEnabledReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_vDutEnabledReturnMap);
		}
		else
		{
			// do nothing
		}
	}
    catch(char *msg)
    {
        ReturnErrorMessage(l_vDutEnabledReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_vDutEnabledReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
		err = -1;
    }

    return err;
}

int InitializeDutEnabledContainers(void)
{
    /*------------------*
     * Input Parameters: *
     * (None)           *
     *------------------*/
    l_vDutEnabledParamMap.clear();

    WIFI_SETTING_STRUCT setting;
    setting.unit = "";
    setting.helpText = "";

 	/*-----------------------------------*
     * Return Values:                    *
     * Error Msg while disconnect Tester *
     *-----------------------------------*/

	l_vDutEnabledReturnMap.clear();

    l_vDutEnabledReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_vDutEnabledReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_vDutEnabledReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_vDutEnabledReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}

