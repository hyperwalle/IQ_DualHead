#include "stdafx.h"
#include "TestManager.h"
#include "BT_Test.h"
#include "BT_Test_Internal.h"
#include "IQmeasure.h"


// This global variable is declared in BT_Test_Internal.cpp
extern TM_ID    g_BT_Test_ID;
extern int      g_Tester_Number;
extern int      g_Tester_Reconnect;

using namespace std;

// Input Parameter Container
map<string, BT_SETTING_STRUCT> l_disconnectTesterParamMap;

// Return Value Container 
map<string, BT_SETTING_STRUCT> l_disconnectTesterReturnMap;

// BT_Disconnect_IQTester does not require any inputs

struct tagReturn
{
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_disconnectTesterReturn;

#ifndef WIN32
int initDisconnectTesterContainers = InitializeDisconnectTesterContainers();
#endif

int ClearDisconnectLPReturn(void)
{
	l_disconnectTesterParamMap.clear();
	l_disconnectTesterReturnMap.clear();
	return 0;
}

//! Disconnect to Tester (IQview/flex, IQmax, IQnxn, IQmimo, etc.)
/*!
 * Input Parameters
 *
 *  - None
 *
 * Return Values
 *      -# A string for possible error message
 *
 * \return 0 No error occurred
 * \return -1 DUT failed to insert.  Please see the returned error message for details
 */
BT_TEST_API int BT_Disconnect_IQTester(void)
{
    int		err = ERR_OK;	
    int		dummyValue = 0;
	char	logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_disconnectTesterReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        TM_ClearReturns(g_BT_Test_ID);
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


		err = LP_DualHead_ReleaseControl();
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Failed to release control.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_DualHead_ReleaseControl() return OK.\n");
		} 


        if( g_Tester_Reconnect && 0!=g_Tester_Number )
        {
			err = LP_Term();
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to terminate IQTester.\n");
				throw logMessage;
			}
        }
        else
        {
            err = ERR_OK;
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
		ReturnErrorMessage(l_disconnectTesterReturn.ERROR_MESSAGE, "[BT] Unknown Error!\n");
		err = -1;
    }

    return err;
}

int InitializeDisconnectTesterContainers(void)
{
    /*------------------*
     * Input Paramters: *
     * (None)           *
     *------------------*/
    l_disconnectTesterParamMap.clear();

    BT_SETTING_STRUCT setting;
    setting.unit = "";
    setting.helpText = "";

    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_disconnectTesterReturnMap.clear();

    l_disconnectTesterReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_disconnectTesterReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_disconnectTesterReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_disconnectTesterReturnMap.insert( pair<string,BT_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}

