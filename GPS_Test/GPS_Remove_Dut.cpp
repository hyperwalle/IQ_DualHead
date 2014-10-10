#include "stdafx.h"
#include "TestManager.h"
#include "GPS_Test.h"
#include "GPS_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"


// This global variable is declared in GPS_Test_Internal.cpp
extern vDUT_ID      g_GPS_Dut;
extern  bool        g_ReloadDutDll;
extern TM_ID        g_GPS_Test_ID;

using namespace std;

// Input Parameter Container
map<string, GPS_SETTING_STRUCT> l_removeDutParamMap;

// Return Value Container 
map<string, GPS_SETTING_STRUCT> l_removeDutReturnMap;

struct tagReturn
{
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_removeDutReturn;

void ClearRemoveDutReturn(void)
{
	l_removeDutParamMap.clear();
	l_removeDutReturnMap.clear();
}


//! Remove GPS DUT
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
GPS_TEST_API int GPS_Remove_Dut(void)
{
    int		err = ERR_OK;	
    int		dummyValue = 0;
	char	logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char	vDutErrorMsg[MAX_BUFFER_SIZE] = {'\0'};

#pragma region Step 1
    /*
    1. Clear Return Parameters and Container
    2. Respond to QUERY_INPUT
    3. Respond to QUERY_RETURN
    */

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_removeDutReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_GPS_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_removeDutParamMap);
        return err;
    }
	else{}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_GPS_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_removeDutReturnMap);
        return err;
    }
	else{}
#pragma endregion

	try
	{
#pragma region Step 2
        /*
        1. g_GPS_Test_ID needs to be valid (>=0)
        X 2. Get input parameters
        */

	   /*-----------------------------------------------------------*
		* Both g_GPS_Test_ID and g_GPS_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		TM_ClearReturns(g_GPS_Test_ID);
		if( g_GPS_Test_ID<0 || g_GPS_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] GPS_Test_ID or GPS_Dut not valid.\n");
			throw logMessage;
		}
		else{}
#pragma endregion

#pragma region Step 4 : Main Step

		// REMOVE_DUT
		err = vDUT_Run(g_GPS_Dut, "REMOVE_DUT");	
		if ( ERR_OK!=err )
		{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.
			err = vDUT_GetStringReturn(g_GPS_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else	// Return error message
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] vDUT_Run(REMOVE_DUT) return error.\n");
				throw logMessage;
			}
		}
		else{}
	    
		// If specified to reload the DUT control DLL every time
		if( 0!=g_ReloadDutDll && -1!=g_GPS_Dut )
		{
			/*****************************************************************************
			 * NOTE: vDUT_UnregisterDutDll() will unload the DUT control DLL from memory *
			 *****************************************************************************/
			err = vDUT_UnregisterDutDll("GPS", &g_GPS_Dut);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] vDUT_UnregisterDutDll() return error.\n");
				throw logMessage;
			}
		}
		else{}
#pragma endregion

#pragma region Step 5
        /*
        1. Return Test Results
        */

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_removeDutReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_removeDutReturnMap);
		}
#pragma endregion

	}
	catch(char *msg)
	{
        ReturnErrorMessage(l_removeDutReturn.ERROR_MESSAGE, msg);
    }
	catch(...)
	{
		ReturnErrorMessage(l_removeDutReturn.ERROR_MESSAGE, "[GPS] Unknown Error!\n");
		err = -1;
    }

    return err;
}

void InitializeRemoveDutContainers(void)
{
    
    GPS_SETTING_STRUCT setting;

#pragma region Step 1 : Setup & Init InputParm
    /*------------------*
     * Input Paramters: *
     * IQTESTER_IP01    *
     *------------------*/
    l_removeDutParamMap.clear();

    setting.unit = "";
    setting.helpText = "";

#pragma endregion

#pragma region Step 2 : Setup & Init ReturnValue
    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_removeDutReturnMap.clear();

    /* return:error_message  */
    l_removeDutReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_removeDutReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_removeDutReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_removeDutReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
	else
	{
        printf("Parameter Type Error!\n");
        exit(1);
    }

#pragma endregion
}
