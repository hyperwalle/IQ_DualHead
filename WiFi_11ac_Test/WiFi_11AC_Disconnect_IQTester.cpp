#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_11AC_Test.h"
#include "WiFi_11AC_Test_Internal.h"
#include "IQmeasure.h"


// This global variable is declared in WiFi_Test_Internal.cpp
extern TM_ID    g_WiFi_11ac_Test_ID;
extern int      g_Tester_Number;
extern int      g_Tester_Reconnect;

using namespace std;

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_11ACdisconnectTesterParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_11ACdisconnectTesterReturnMap;

// WiFi_Disconnect_IQTester does not require any inputs

struct tagReturn
{
	char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_11ACdisconnectTesterReturn;


#ifndef WIN32
int init11ACDisconnectTesterContainers = Initialize11ACDisconnectTesterContainers();
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
WIFI_11AC_TEST_API int WIFI_11AC_Disconnect_IQTester(void)
{
	int  err = ERR_OK;
	int  dummyValue = 0;
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

	/*---------------------------------------*
	 * Clear Return Parameters and Container *
	 *---------------------------------------*/
	ClearReturnParameters(l_11ACdisconnectTesterReturnMap);

	/*------------------------*
	 * Respond to QUERY_INPUT *
	 *------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		TM_ClearReturns(g_WiFi_11ac_Test_ID);
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
		RespondToQueryReturn(l_11ACdisconnectTesterReturnMap);
		return err;
	}
	else
	{
		// do nothing
	}

	try
	{
		/*---------------------------------------*
		 * g_WiFi_11ac_Test_ID need to be valid (>=0) *
		 *---------------------------------------*/
		if( g_WiFi_11ac_Test_ID<0 )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] WiFi_Test_ID not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] WiFi_Test_ID = %d.\n", g_WiFi_11ac_Test_ID);
		}

		TM_ClearReturns(g_WiFi_11ac_Test_ID);
		//err = LP_DualHead_ReleaseControl();
		if( g_Tester_Reconnect && 0!=g_Tester_Number )
		{
			err = LP_Term();
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to terminate IQTester.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_Term() successful.\n");
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] No IQtester is connected.\n");
				throw logMessage;
			}
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_11ACdisconnectTesterReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_11ACdisconnectTesterReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_11ACdisconnectTesterReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		ReturnErrorMessage(l_11ACdisconnectTesterReturn.ERROR_MESSAGE, "[WiFi_11AC] Unknown Error!\n");
		err = -1;
	}

	return err;
}

int Initialize11ACDisconnectTesterContainers(void)
{
	/*------------------*
	 * Input Parameters: *
	 * (None)           *
	 *------------------*/
	l_11ACdisconnectTesterParamMap.clear();

	WIFI_SETTING_STRUCT setting;
	setting.unit = "";
	setting.helpText = "";

	/*-----------------------------------*
	 * Return Values:                    *
	 * Error Msg while disconnect Tester *
	 *-----------------------------------*/

	l_11ACdisconnectTesterReturnMap.clear();

	l_11ACdisconnectTesterReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACdisconnectTesterReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_11ACdisconnectTesterReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_11ACdisconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	return 0;
}

