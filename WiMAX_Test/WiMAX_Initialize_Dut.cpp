#include "stdafx.h"
#include "TestManager.h"
#include "WIMAX_Test.h"
#include "WIMAX_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

// This variable is declared in WIMAX_Test_Internal.cpp
extern vDUT_ID      g_WIMAX_Dut;
extern TM_ID        g_WIMAX_Test_ID;

using namespace std;

// Input Parameter Container
map<string, WIMAX_SETTING_STRUCT> l_initializeDutParamMap;

// Return Value Container 
map<string, WIMAX_SETTING_STRUCT> l_initializeDutReturnMap;

struct tagParam
{
} l_initializeDutParam;

struct tagReturn
{
	char DUT_NAME[MAX_BUFFER_SIZE];				/*!< DUT name. */
    char DUT_DRIVER_INFO[MAX_BUFFER_SIZE];		/*!< DUT driver info, such as driver version, firmware version, etc. */

    char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_initializeDutReturn;

void ClearInitializeDutReturn(void)
{
	l_initializeDutParamMap.clear();
	l_initializeDutReturnMap.clear();
}

//! Initialize WIMAX DUT
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
WIMAX_TEST_API int WIMAX_Initialize_Dut(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_initializeDutReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WIMAX_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_initializeDutParamMap);
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
        RespondToQueryReturn(l_initializeDutReturnMap);
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

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_initializeDutParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Get input parameters return OK.\n");
		}

		// Initialize DUT
		err = vDUT_Run(g_WIMAX_Dut, "INITIALIZE_DUT");
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(INITIALIZE_DUT) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(INITIALIZE_DUT) return OK.\n");
		}
	        
		// For retrieval of the DUT info
		err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "DUT_NAME", l_initializeDutReturn.DUT_NAME, MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			err = ERR_OK;	// This is an optional return parameter, thus always return OK
			sprintf_s(l_initializeDutReturn.DUT_NAME, MAX_BUFFER_SIZE, "No Return, Unknown Dut Name.\n");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetStringReturn(DUT_NAME) return OK.\n");
		}

		err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "DUT_DRIVER_INFO", l_initializeDutReturn.DUT_DRIVER_INFO, MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			err = ERR_OK;	// This is an optional return parameter, thus always return OK
			sprintf_s(l_initializeDutReturn.DUT_DRIVER_INFO, MAX_BUFFER_SIZE, "No Return, Unknown Dut Driver Info.\n");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetStringReturn(DUT_DRIVER_INFO) return OK.\n");
		}

		/*----------------------*
		 *  Function Completed  *
		 *----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_initializeDutReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
		}
		else
		{
			// do nothing
		}
	}
    catch(char *msg)
    {
        ReturnErrorMessage(l_initializeDutReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_initializeDutReturn.ERROR_MESSAGE, "[WIMAX] Unknown Error!\n");
		err = -1;
    }


	/*-----------------------*
	 *  Return Test Results  *
	 *-----------------------*/
	ReturnTestResults(l_initializeDutReturnMap);


    return err;
}

void InitializeInitializeDutContainers(void)
{
    /*------------------*
     * Input Parameters: *
     *------------------*/
    l_initializeDutParamMap.clear();

    WIMAX_SETTING_STRUCT setting;
      
    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_initializeDutReturnMap.clear();

    l_initializeDutReturn.DUT_NAME[0] = '\0';
    setting.type = WIMAX_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_initializeDutReturn.DUT_NAME))    // Type_Checking
    {
        setting.value       = (void*)l_initializeDutReturn.DUT_NAME;
        setting.unit        = "";
        setting.helpText    = "DUT name.";
        l_initializeDutReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("DUT_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_initializeDutReturn.DUT_DRIVER_INFO[0] = '\0';
    setting.type = WIMAX_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_initializeDutReturn.DUT_DRIVER_INFO))    // Type_Checking
    {
        setting.value       = (void*)l_initializeDutReturn.DUT_DRIVER_INFO;
        setting.unit        = "";
        setting.helpText    = "DUT driver info, such as driver version, firmware version, etc.";
        l_initializeDutReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("DUT_DRIVER_INFO", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_initializeDutReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIMAX_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_initializeDutReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_initializeDutReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_initializeDutReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return;
}

