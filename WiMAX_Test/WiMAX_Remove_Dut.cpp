#include "stdafx.h"
#include "TestManager.h"
#include "WIMAX_Test.h"
#include "WIMAX_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"


// This global variable is declared in WIMAX_Test_Internal.cpp
extern vDUT_ID      g_WIMAX_Dut;
extern  bool        g_ReloadDutDll;
extern TM_ID        g_WIMAX_Test_ID;

using namespace std;

// Input Parameter Container
map<string, WIMAX_SETTING_STRUCT> l_removeDutParamMap;

// Return Value Container 
map<string, WIMAX_SETTING_STRUCT> l_removeDutReturnMap;

struct tagReturn
{
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_removeDutReturn;

void ClearRemoveDutReturn(void)
{
	l_removeDutParamMap.clear();
	l_removeDutReturnMap.clear();
}

//! Remove WIMAX DUT
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
WIMAX_TEST_API int WIMAX_Remove_Dut(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_removeDutReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WIMAX_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_removeDutParamMap);
        return err;
    }
	else
	{
		// do nothing
	}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_WIMAX_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_removeDutReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
	   /*-----------------------------------------------------------*
		* Both g_WIMAX_Test_ID and g_WIMAX_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		if( g_WIMAX_Test_ID<0 || g_WIMAX_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] WIMAX_Test_ID or WIMAX_Dut not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] WIMAX_Test_ID = %d and WIMAX_Dut = %d.\n", g_WIMAX_Test_ID, g_WIMAX_Dut);
		}
		
		TM_ClearReturns(g_WIMAX_Test_ID);
        
        // Error return of this function is irrelevant
        CheckDutTransmitStatus();
 
        // REMOVE_DUT
		err = ::vDUT_Run(g_WIMAX_Dut, "REMOVE_DUT");		
		if ( ERR_OK!=err )
		{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				throw logMessage;
			}
			else	// Just return normal error message in this case
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(REMOVE_DUT) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(REMOVE_DUT) return OK.\n");
		}
	    
		// If specified to reload the DUT control DLL every time
		if( 0!=g_ReloadDutDll && -1!=g_WIMAX_Dut )
		{
			/*****************************************************************************
			 * NOTE: vDUT_UnregisterDutDll() will unload the DUT control DLL from memory *
			 *****************************************************************************/
			err = ::vDUT_UnregisterDutDll("WIMAX", &g_WIMAX_Dut);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Unload the DUT control DLL from memory vDUT_UnregisterDutDll() return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Unload the DUT control DLL from memory vDUT_UnregisterDutDll() return OK.\n");
			}
		}
		else
		{
			// do nothing
		}

		/*----------------------*
		 *  Function Completed  *
		 *----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_removeDutReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_removeDutReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_removeDutReturn.ERROR_MESSAGE, "[WIMAX] Unknown Error!\n");
		err = -1;
    }

	/*-----------------------*
	 *  Return Test Results  *
	 *-----------------------*/
	ReturnTestResults(l_removeDutReturnMap);


    return err;
}

void InitializeRemoveDutContainers(void)
{
    /*------------------*
     * Input Parameters: *
     * IQTESTER_IP01    *
     *------------------*/
    l_removeDutParamMap.clear();

    WIMAX_SETTING_STRUCT setting;
    setting.unit = "";
    setting.helpText = "";


    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_removeDutReturnMap.clear();

    l_removeDutReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIMAX_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_removeDutReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_removeDutReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_removeDutReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return;
}
