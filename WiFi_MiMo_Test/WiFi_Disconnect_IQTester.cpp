#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_MiMo_Test.h"
#include "WiFi_MiMo_Test_Internal.h"
#include "IQmeasure.h"


// This global variable is declared in WiFi_Test_Internal.cpp
extern TM_ID    g_WiFi_Test_ID;
extern int      g_Tester_Number;
extern int      g_Tester_Reconnect;

using namespace std;

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_disconnectTesterParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_disconnectTesterReturnMap;

// WiFi_Disconnect_IQTester does not require any inputs

struct tagReturn
{
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_disconnectTesterReturn;


#ifndef WIN32 
int initDisconnectTesterMIMOContainers = InitializeDisconnectTesterContainers();
#endif
//! Disonnect to Tester (IQview/flex, IQmax, IQnxn, IQmimo, etc.)
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
WIFI_MIMO_TEST_API int WiFi_Disconnect_IQTester(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_disconnectTesterReturnMap);

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
        RespondToQueryReturn(l_disconnectTesterReturnMap);
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

        if( g_Tester_Reconnect && 0!=g_Tester_Number )
        {
            err = LP_Term();
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to terminate IQTester.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_Term() successful.\n");
			} 
        }
        else
        {
			if ( 0!=g_Tester_Number )	// IQTester number != 0 means there are some testers connected, so can return OK.
			{
				err = ERR_OK;
			}
			else
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] No IQtester is connected.\n");
				throw logMessage;
			}
        }

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_disconnectTesterReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_disconnectTesterReturnMap);
		}
		else
		{
			// do nothing
		}
	}
    catch(char *msg)
    {
        ReturnErrorMessage(l_disconnectTesterReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_disconnectTesterReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
		err = -1;
    }

    return err;
}

int InitializeDisconnectTesterContainers(void)
{
    /*------------------*
     * Input Parameters: *
     * (None)           *
     *------------------*/
    l_disconnectTesterParamMap.clear();

    WIFI_SETTING_STRUCT setting;
    setting.unit = "";
    setting.helpText = "";

 	/*-----------------------------------*
     * Return Values:                    *
     * Error Msg while disconnect Tester *
     *-----------------------------------*/

	l_disconnectTesterReturnMap.clear();

    l_disconnectTesterReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_disconnectTesterReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_disconnectTesterReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_disconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}

// IQlite merge; Tracy Yu ; 2012-03-31
void CleanupTXDisconnectIQTester(void)
{

	l_disconnectTesterParamMap.clear();
	l_disconnectTesterReturnMap.clear();

}