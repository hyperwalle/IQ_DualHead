#include "stdafx.h"
#include "TestManager.h"
#include "BT_Test.h"
#include "BT_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

// This variable is declared in BT_Test_Internal.cpp
extern vDUT_ID      g_BT_Dut;
extern TM_ID        g_BT_Test_ID;

using namespace std;

// Input Parameter Container
map<string, BT_SETTING_STRUCT> l_btleInitializeParamMap;

// Return Value Container 
map<string, BT_SETTING_STRUCT> l_btleInitializeReturnMap;

struct tagParam
{
	char BTLE_INIT_FILE[MAX_BUFFER_SIZE];		/*!< Specify which image file name is to be used; for example, BCM_BT.hcd. */
} l_btleInitializeParam;

struct tagReturn
{
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_btleInitializeReturn;

int ClearbtleInitializeReturn(void)
{
	l_btleInitializeParamMap.clear();
	l_btleInitializeReturnMap.clear();
	return 0;
}

//! Initialize BT DUT
/*!
 * Input Parameters
 *
 *  - Mandatory 
 *      -# A string that is used to determine which DUT register function will be used
 *
 * Return Values
 *      -# A string that contains all DUT information, such as FW version, driver version, chip revision, etc.
 *      -# A string for error message
 *
 * \return 0 No error occurred
 * \return -1 DUT failed to insert.  Please see the returned error message for details
 */
BT_TEST_API int BT_LE_Initialize(void)
{
    int		err = ERR_OK;	
    int		dummyValue = 0;
	char	logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char	vDutErrorMsg[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_btleInitializeReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_btleInitializeParamMap);
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
        RespondToQueryReturn(l_btleInitializeReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
	   /*-----------------------------------------------------------*
		* Both g_BT_Test_ID and g_BT_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		TM_ClearReturns(g_BT_Test_ID);
		if( g_BT_Test_ID<0 || g_BT_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] BT_Test_ID or BT_Dut not valid. BT_Test_ID = %d and BT_Dut = %d.\n", g_BT_Test_ID, g_BT_Dut);
			throw logMessage;
		}
		else
		{
			// do nothing
		}

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_btleInitializeParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Input parameters are not complete.\n");
			throw logMessage;
		}

		// BTLE_INIT_FILE, Specify which image file name has to be used.
		err = vDUT_AddStringParameter (g_BT_Dut, "BTLE_INIT_FILE", l_btleInitializeParam.BTLE_INIT_FILE);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_AddStringParameter(BTLE_INIT_FILE) return error.\n", l_btleInitializeParam.BTLE_INIT_FILE);
			throw logMessage;
		}

		// perform BTLE Init
		err = vDUT_Run(g_BT_Dut, "BTLE_INITIALIZE");
		if ( ERR_OK!=err )
		{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.
			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1; indicates there is an "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else	// Return error message
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(BTLE_INIT_FILE) return error.\n");
				throw logMessage;
			}
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
			sprintf_s(l_btleInitializeReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_btleInitializeReturnMap);
		}
		else
		{
			// do nothing
		}
	}
    catch(char *msg)
    {
        ReturnErrorMessage(l_btleInitializeReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_btleInitializeReturn.ERROR_MESSAGE, "[BT] Unknown Error!\n");
		err = -1;
    }

    return err;
}

int InitializebtleInitializeContainers(void)
{
    /*------------------*
     * Input Parameters: *
     *------------------*/
    l_btleInitializeParamMap.clear();

    BT_SETTING_STRUCT setting;

    setting.unit        = "";
    setting.helpText    = "Specify which image file name to be used.";
    setting.type = BT_SETTING_TYPE_STRING;
    strcpy_s(l_btleInitializeParam.BTLE_INIT_FILE, MAX_BUFFER_SIZE, "");
    if (MAX_BUFFER_SIZE==sizeof(l_btleInitializeParam.BTLE_INIT_FILE))    // Type_Checking
    {
        setting.value       = (void*)l_btleInitializeParam.BTLE_INIT_FILE;
        l_btleInitializeParamMap.insert( pair<string,BT_SETTING_STRUCT>("BTLE_INIT_FILE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
      
    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_btleInitializeReturnMap.clear();

    l_btleInitializeReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_btleInitializeReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_btleInitializeReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_btleInitializeReturnMap.insert( pair<string,BT_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}

