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
map<string, BT_SETTING_STRUCT> l_resetDutParamMap;

// Return Value Container 
map<string, BT_SETTING_STRUCT> l_resetDutReturnMap;

struct tagParam
{
} l_resetDutParam;

struct tagReturn
{
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_resetDutReturn;

//! Reset BT DUT
/*!
* Input Parameters
*
*  - Mandatory 
*
* Return Values
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 DUT failed to insert.  Please see the returned error message for details
*/
BT_TEST_API int BT_Reset_Dut(void)
{
    int		err = ERR_OK;	
    int		dummyValue = 0;
    char	logMessage[MAX_BUFFER_SIZE] = {'\0'};
    char	vDutErrorMsg[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
    * Clear Return Parameters and Container *
    *---------------------------------------*/
    ClearReturnParameters(l_resetDutReturnMap);

    /*------------------------*
    * Respond to QUERY_INPUT *
    *------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_resetDutParamMap);
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
        RespondToQueryReturn(l_resetDutReturnMap);
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
            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] BT_Test_ID or BT_Dut not valid.\n");
            throw logMessage;
        }
        else
        {
            // do nothing
        }

        /*----------------------*
        * Get input parameters *
        *----------------------*/
        err = GetInputParameters(l_resetDutParamMap);
        if ( ERR_OK!=err )
        {
            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Input parameters are not complete.\n");
            throw logMessage;
        }

        // reset DUT
        err = vDUT_Run(g_BT_Dut, "RESET_DUT");
        if ( ERR_OK!=err )
        {	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
            err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
            if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
            {
                err = -1;	// set err to -1, means "Error".
                LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
                throw logMessage;
            }
        }
        else
        {
			sprintf_s(l_resetDutReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_resetDutReturnMap);
        }
    }
    catch(char *msg)
    {
        ReturnErrorMessage(l_resetDutReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
        ReturnErrorMessage(l_resetDutReturn.ERROR_MESSAGE, "[BT] Unknown Error!\n");
    }

    return err;
}

int InitializeResetDutContainers(void)
{
    /*------------------*
    * Input Parameters: *
    *------------------*/
    l_resetDutParamMap.clear();

    BT_SETTING_STRUCT setting;

    /*----------------*
    * Return Values: *
    * ERROR_MESSAGE  *
    *----------------*/
    l_resetDutReturnMap.clear();

    l_resetDutReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_resetDutReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_resetDutReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_resetDutReturnMap.insert( pair<string,BT_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}

