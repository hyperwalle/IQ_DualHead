#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_Test.h"
#include "WiFi_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"

using namespace std;

#define MAX_RESULT_GROUP_NUM  3
#define LP_RESULT_FORMAT      "LitePoint_Data_Format"

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_runDutCmdParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_runDutCmdReturnMap;

struct tagParam
{
    char    DUT_COMMAND[MAX_BUFFER_SIZE];
	int     WAIT_TIME_MS;
} l_runDutCmdParam;

struct tagReturn
{
	char    RESPONSE[MAX_BUFFER_SIZE];
	char    ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_runDutCmdReturn;

#ifndef WIN32
int InitRunDutCommandMPSContainers = InitializeRunDutCommandContainers();
#endif

//! Insert WiFi DUT
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

WIFI_TEST_API int WiFi_Run_DUT_Command(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_runDutCmdReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_runDutCmdParamMap);
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
        RespondToQueryReturn(l_runDutCmdReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
		TM_ClearReturns(g_WiFi_Test_ID);

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_runDutCmdParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Get input parameters return OK.\n");
		}

		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_Dut);

#pragma region Execute external program

        // Then execute the DUT command
        vDUT_AddStringParameter(g_WiFi_Dut, "DUT_COMMAND", l_runDutCmdParam.DUT_COMMAND);

	    /*-------------------------------*
        *  Passing wl cmd file into dut  *
        *--------------------------------*/
        err = ::vDUT_Run(g_WiFi_Dut, "DUT_MISC_FUNCTION2");		
        if ( ERR_OK!=err )
        {	
			// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
            err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
            if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
            {
                err = -1;	// set err to -1, means "Error".
                LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
                throw logMessage;
            }
            else	// Just return normal error message in this case
            {
                LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(Run_Dut_Cmd) return error.\n");
                throw logMessage;
            }

        }
        else
        {
            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(Run_Dut_Cmd) return OK.\n");


			// For retrieval of the DUT info
			err = ::vDUT_GetStringReturn(g_WiFi_Dut, "RESPONSE", l_runDutCmdReturn.RESPONSE, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetStringReturn(RESPONSE) return OK.\n");
			}
        }
#pragma endregion	   

#pragma region Execute sleep enough time till external program over
		Sleep(l_runDutCmdParam.WAIT_TIME_MS);
#pragma endregion	

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_runDutCmdReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_runDutCmdReturnMap);
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_runDutCmdReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_runDutCmdReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
    }

    return err;
}

int InitializeRunDutCommandContainers(void)
{
    /*------------------*
     * Input Parameters: *
     *------------------*/
    l_runDutCmdParamMap.clear();
    WIFI_SETTING_STRUCT setting;

    l_runDutCmdParam.WAIT_TIME_MS = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_runDutCmdParam.WAIT_TIME_MS))    // Type_Checking
    {
        setting.value = (void*)&l_runDutCmdParam.WAIT_TIME_MS;
        setting.unit        = "ms";
        setting.helpText    = "Specify the wait time before extract the value from result file.";
        l_runDutCmdParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("WAIT_TIME_MS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.unit        = "";
    setting.helpText    = "Specify DUT command to run, case sensitive\r\nSuch as: band a";
    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(l_runDutCmdParam.DUT_COMMAND, MAX_BUFFER_SIZE, "");
    if (MAX_BUFFER_SIZE==sizeof(l_runDutCmdParam.DUT_COMMAND))    // Type_Checking
    {
        setting.value       = (void*)l_runDutCmdParam.DUT_COMMAND;
        l_runDutCmdParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DUT_COMMAND", setting) );
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
    l_runDutCmdReturnMap.clear();

    l_runDutCmdReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_runDutCmdReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_runDutCmdReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_runDutCmdReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_runDutCmdReturn.RESPONSE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_runDutCmdReturn.RESPONSE))    // Type_Checking
	{
		setting.value       = (void*)l_runDutCmdReturn.RESPONSE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_runDutCmdReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("RESPONSE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


    return 0;
}