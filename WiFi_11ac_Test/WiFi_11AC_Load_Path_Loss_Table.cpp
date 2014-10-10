#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_11AC_Test.h"
#include "WiFi_11AC_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

using namespace std;

// These global variables are declared in WiFi_Test_Internal.cpp
extern TM_ID                 g_WiFi_11ac_Test_ID;    
extern vDUT_ID               g_WiFi_11ac_Dut;
extern bool					 g_vDutTxActived;
extern bool					 g_dutConfigChanged;
extern WIFI_RECORD_PARAM     g_RecordedParam;

// This global variable is declared in WiFi_Global_Setting.cpp
extern WIFI_GLOBAL_SETTING g_WiFi11ACGlobalSettingParam;

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_11ACloadPathLossTableParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_11ACloadPathLossTableReturnMap;

struct tagParam
{
    char    TX_PATH_LOSS_FILE[MAX_BUFFER_SIZE];
	char    RX_PATH_LOSS_FILE[MAX_BUFFER_SIZE];
} l_11ACloadPathLossTableParam;

struct tagReturn
{
    char    ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_11ACloadPathLossTableReturn;

#ifndef WIN32
int init11ACloadPathLossTableContainers = Initialize11ACloadPathLossTableContainers();
#endif

WIFI_11AC_TEST_API int WIFI_11AC_LoadPathLossTable(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	//bool vDutTxActived = false;
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};


    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_11ACloadPathLossTableReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_11ACloadPathLossTableParamMap);
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
        RespondToQueryReturn(l_11ACloadPathLossTableReturnMap);
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

       /*----------------------*
        * Get input parameters *
        *----------------------*/
        err = GetInputParameters(l_11ACloadPathLossTableParamMap);
		if ( ERR_OK!=err )
		{
			// For the backward compatible, we need to read "PATH_LOSS_FILE".
			err = ::TM_GetStringParameter(g_WiFi_11ac_Test_ID, "PATH_LOSS_FILE", l_11ACloadPathLossTableParam.TX_PATH_LOSS_FILE, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{				
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Input parameters are not complete.\n");
				throw logMessage;	
			}
			else
			{
				// If "ERR_OK" means got this keyword, then we must make it backward compatible
				strcpy_s(l_11ACloadPathLossTableParam.RX_PATH_LOSS_FILE, MAX_BUFFER_SIZE, l_11ACloadPathLossTableParam.TX_PATH_LOSS_FILE); 				
			}	
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Get input parameters return OK.\n");
		}

        // Error return of this function is irrelevant
        CheckDutTransmitStatus();

		// For TX table
        err = TM_UpdatePathLossByFile(g_WiFi_11ac_Test_ID, l_11ACloadPathLossTableParam.TX_PATH_LOSS_FILE, TX_TABLE);	
        switch( err )
        {
			case ERR_OK:
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] TM_UpdatePathLossByFile(TX_PATH_LOSS_FILE) return OK.\n");
				break;
			case TM_ERR_FAILED_TO_OPEN_FILE:
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Failed to open %s.\n", l_11ACloadPathLossTableParam.TX_PATH_LOSS_FILE);
				throw logMessage;
				break;
			default:
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown error in LOAD_TX_PATH_LOSS_TABLE.\n");
				throw logMessage;
				break;
        }

		// For RX table
        err = TM_UpdatePathLossByFile(g_WiFi_11ac_Test_ID, l_11ACloadPathLossTableParam.RX_PATH_LOSS_FILE, RX_TABLE);	
        switch( err )
        {
			case ERR_OK:
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] TM_UpdatePathLossByFile(RX_PATH_LOSS_FILE) return OK.\n");
				break;
			case TM_ERR_FAILED_TO_OPEN_FILE:
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Failed to open %s.\n", l_11ACloadPathLossTableParam.RX_PATH_LOSS_FILE);
				throw logMessage;
				break;
			default:
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown error in LOAD_RX_PATH_LOSS_TABLE.\n");
				throw logMessage;
				break;
        }

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_11ACloadPathLossTableReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_11ACloadPathLossTableReturnMap);
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_11ACloadPathLossTableReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_11ACloadPathLossTableReturn.ERROR_MESSAGE, "[WiFi_11AC] Unknown Error!\n");
    }

    return err;
}

int Initialize11ACloadPathLossTableContainers(void)
{
    /*------------------*
     * Input Parameters: *
     * IQTESTER_IP01    *
     *------------------*/
    l_11ACloadPathLossTableParamMap.clear();

    WIFI_SETTING_STRUCT setting;

    setting.unit        = "";
    setting.helpText    = "A comma delimited text file that contains Tx path loss table.\r\nEach row starts with a frequency in MHz, followed by a number of path loss values.\r\n";
    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(l_11ACloadPathLossTableParam.TX_PATH_LOSS_FILE, MAX_BUFFER_SIZE, "path_loss.csv");
    if (MAX_BUFFER_SIZE==sizeof(l_11ACloadPathLossTableParam.TX_PATH_LOSS_FILE))    // Type_Checking
    {
        setting.value       = (void*)l_11ACloadPathLossTableParam.TX_PATH_LOSS_FILE;
        l_11ACloadPathLossTableParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_PATH_LOSS_FILE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.unit        = "";
    setting.helpText    = "A comma delimited text file that contains Rx path loss table.\r\nEach row starts with a frequency in MHz, followed by a number of path loss values.\r\n";
    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(l_11ACloadPathLossTableParam.RX_PATH_LOSS_FILE, MAX_BUFFER_SIZE, "path_loss.csv");
    if (MAX_BUFFER_SIZE==sizeof(l_11ACloadPathLossTableParam.RX_PATH_LOSS_FILE))    // Type_Checking
    {
        setting.value       = (void*)l_11ACloadPathLossTableParam.RX_PATH_LOSS_FILE;
        l_11ACloadPathLossTableParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX_PATH_LOSS_FILE", setting) );
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
    l_11ACloadPathLossTableReturnMap.clear();

    l_11ACloadPathLossTableReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACloadPathLossTableReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_11ACloadPathLossTableReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_11ACloadPathLossTableReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}
