#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_11AC_Test.h"
#include "WiFi_11AC_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"

// This variable is declared in WiFi_Test_Internal.cpp
extern vDUT_ID      g_WiFi_11ac_Dut;
extern TM_ID        g_WiFi_11ac_Test_ID;
extern int          g_WiFi_11AC_Test_timer;
extern int          g_WiFi_11AC_Test_logger;
extern bool			g_vDutTxActived;
extern bool			g_dutConfigChanged;

using namespace std;

#define MAX_RESULT_GROUP_NUM  3
#define LP_RESULT_FORMAT      "LitePoint_Data_Format"

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_11ACrunDutCmdParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_11ACrunDutCmdReturnMap;

struct tagParam
{
    char    DUT_COMMAND[MAX_BUFFER_SIZE];
	int     WAIT_TIME_MS;
} l_11ACrunDutCmdParam;

struct tagReturn
{
	char    RESPONSE[MAX_BUFFER_SIZE];
	char    ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_11ACrunDutCmdReturn;

#ifndef WIN32
int Init11ACRunDutCommandMPSContainers = Initialize11ACRunDutCommandContainers();
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

WIFI_11AC_TEST_API int WIFI_11AC_Run_DUT_Command(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_11ACrunDutCmdReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_11ACrunDutCmdParamMap);
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
        RespondToQueryReturn(l_11ACrunDutCmdReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
		TM_ClearReturns(g_WiFi_11ac_Test_ID);

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_11ACrunDutCmdParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Get input parameters return OK.\n");
		}

		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_11ac_Dut);

#pragma region Execute external program

        // Then execute the DUT command
        vDUT_AddStringParameter(g_WiFi_11ac_Dut, "DUT_COMMAND", l_11ACrunDutCmdParam.DUT_COMMAND);
		if ( (g_dutConfigChanged==true)||(g_vDutTxActived==false) )	
		{
			if ( g_vDutTxActived==true )
			{
			   /*-----------*
				*  Tx Stop  *
				*-----------*/
				err = ::vDUT_Run(g_WiFi_11ac_Dut, "TX_STOP");		
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
						throw logMessage;
					}
					else	// Just return normal error message in this case
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(TX_STOP) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					g_vDutTxActived = false;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(TX_STOP) return OK.\n");
				}
			}
			else
			{
				// continue Dut configuration				
			}
		}

	    /*-------------------------------*
        *  Passing wl cmd file into dut  *
        *--------------------------------*/
        err = ::vDUT_Run(g_WiFi_11ac_Dut, "DUT_MISC_FUNCTION2");		
        if ( ERR_OK!=err )
        {	
			// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
            err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
            if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
            {
                err = -1;	// set err to -1, means "Error".
                LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
                throw logMessage;
            }
            else	// Just return normal error message in this case
            {
                LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(Run_Dut_Cmd) return error.\n");
                throw logMessage;
            }

        }
        else
        {
            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(Run_Dut_Cmd) return OK.\n");


			// For retrieval of the DUT info
			err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "RESPONSE", l_11ACrunDutCmdReturn.RESPONSE, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetStringReturn(RESPONSE) return OK.\n");
			}
        }
#pragma endregion	   

#pragma region Execute sleep enough time till external program over
		Sleep(l_11ACrunDutCmdParam.WAIT_TIME_MS);
#pragma endregion	

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_11ACrunDutCmdReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_11ACrunDutCmdReturnMap);
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_11ACrunDutCmdReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_11ACrunDutCmdReturn.ERROR_MESSAGE, "[WiFi_11AC] Unknown Error!\n");
    }

    return err;
}

int Initialize11ACRunDutCommandContainers(void)
{
    /*------------------*
     * Input Parameters: *
     *------------------*/
    l_11ACrunDutCmdParamMap.clear();
    WIFI_SETTING_STRUCT setting;

    l_11ACrunDutCmdParam.WAIT_TIME_MS = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_11ACrunDutCmdParam.WAIT_TIME_MS))    // Type_Checking
    {
        setting.value = (void*)&l_11ACrunDutCmdParam.WAIT_TIME_MS;
        setting.unit        = "ms";
        setting.helpText    = "Specify the wait time before extract the value from result file.";
        l_11ACrunDutCmdParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("WAIT_TIME_MS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.unit        = "";
    setting.helpText    = "Specify DUT command to run, case sensitive\r\nSuch as: band a";
    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(l_11ACrunDutCmdParam.DUT_COMMAND, MAX_BUFFER_SIZE, "");
    if (MAX_BUFFER_SIZE==sizeof(l_11ACrunDutCmdParam.DUT_COMMAND))    // Type_Checking
    {
        setting.value       = (void*)l_11ACrunDutCmdParam.DUT_COMMAND;
        l_11ACrunDutCmdParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DUT_COMMAND", setting) );
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
    l_11ACrunDutCmdReturnMap.clear();

    l_11ACrunDutCmdReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACrunDutCmdReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_11ACrunDutCmdReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_11ACrunDutCmdReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_11ACrunDutCmdReturn.RESPONSE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACrunDutCmdReturn.RESPONSE))    // Type_Checking
	{
		setting.value       = (void*)l_11ACrunDutCmdReturn.RESPONSE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_11ACrunDutCmdReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("RESPONSE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


    return 0;
}