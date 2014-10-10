#include "stdafx.h"
#include "TestManager.h"
#include "WIMAX_Test.h"
#include "WIMAX_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

using namespace std;

// These global variables are declared in WIMAX_Test_Internal.cpp
extern TM_ID                 g_WIMAX_Test_ID;    
extern vDUT_ID               g_WIMAX_Dut;
extern bool					 g_vDutTxActived;
extern bool					 g_dutConfigChanged;
extern WIMAX_RECORD_PARAM    g_RecordedParam;

// This global variable is declared in WIMAX_GLOBAL_SETTING.cpp
extern WIMAX_GLOBAL_SETTING g_WIMAXGlobalSettingParam;

// Input Parameter Container
map<string, WIMAX_SETTING_STRUCT> l_loadPathLossTableParamMap;

// Return Value Container 
map<string, WIMAX_SETTING_STRUCT> l_loadPathLossTableReturnMap;

struct tagParam
{
    char    TX_PATH_LOSS_FILE[MAX_BUFFER_SIZE];
	char    RX_PATH_LOSS_FILE[MAX_BUFFER_SIZE];
} l_loadPathLossTableParam;

struct tagReturn
{
    char    ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_loadPathLossTableReturn;

void ClearLoadPathLossTableReturn(void)
{
	l_loadPathLossTableParamMap.clear();
	l_loadPathLossTableReturnMap.clear();
}


WIMAX_TEST_API int WIMAX_LoadPathLossTable(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	bool vDutTxActived = false;
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};


    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_loadPathLossTableReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WIMAX_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_loadPathLossTableParamMap);
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
        RespondToQueryReturn(l_loadPathLossTableReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}


	try
	{
	   /*---------------------------------------*
		* g_WIMAX_Test_ID need to be valid (>=0) *
		*---------------------------------------*/
		if( g_WIMAX_Test_ID<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] WIMAX_Test_ID not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] WIMAX_Test_ID = %d.\n", g_WIMAX_Test_ID);
		}

		TM_ClearReturns(g_WIMAX_Test_ID);

       /*----------------------*
        * Get input parameters *
        *----------------------*/
        err = GetInputParameters(l_loadPathLossTableParamMap);
		if ( ERR_OK!=err )
		{
			// For the backward compatible, we need to read "PATH_LOSS_FILE".
			err = ::TM_GetStringParameter(g_WIMAX_Test_ID, "PATH_LOSS_FILE", l_loadPathLossTableParam.TX_PATH_LOSS_FILE, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{				
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Input parameters are not complete.\n");
				throw logMessage;	
			}
			else
			{
				// If "ERR_OK" means got this keyword, then we must make it backward compatible
				strcpy_s(l_loadPathLossTableParam.RX_PATH_LOSS_FILE, MAX_BUFFER_SIZE, l_loadPathLossTableParam.TX_PATH_LOSS_FILE); 				
			}	
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Get input parameters return OK.\n");
		}

        // Error return of this function is irrelevant
        CheckDutTransmitStatus();

		// For TX table
        err = TM_UpdatePathLossByFile(g_WIMAX_Test_ID, l_loadPathLossTableParam.TX_PATH_LOSS_FILE, TX_TABLE);	
        switch( err )
        {
			case ERR_OK:
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] TM_UpdatePathLossByFile(TX_PATH_LOSS_FILE) return OK.\n");
				break;
			case TM_ERR_FAILED_TO_OPEN_FILE:
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Failed to open %s.\n", l_loadPathLossTableParam.TX_PATH_LOSS_FILE);
				throw logMessage;
				break;
			default:
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Unknown error in LOAD_TX_PATH_LOSS_TABLE.\n");
				throw logMessage;
				break;
        }

		// For RX table
        err = TM_UpdatePathLossByFile(g_WIMAX_Test_ID, l_loadPathLossTableParam.RX_PATH_LOSS_FILE, RX_TABLE);	
        switch( err )
        {
			case ERR_OK:
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] TM_UpdatePathLossByFile(RX_PATH_LOSS_FILE) return OK.\n");
				break;
			case TM_ERR_FAILED_TO_OPEN_FILE:
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Failed to open %s.\n", l_loadPathLossTableParam.RX_PATH_LOSS_FILE);
				throw logMessage;
				break;
			default:
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Unknown error in LOAD_RX_PATH_LOSS_TABLE.\n");
				throw logMessage;
				break;
        }

		/*----------------------*
		 *  Function Completed  *
		 *----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_loadPathLossTableReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_loadPathLossTableReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_loadPathLossTableReturn.ERROR_MESSAGE, "[WIMAX] Unknown Error!\n");
    }

	/*-----------------------*
	 *  Return Test Results  *
	 *-----------------------*/
	ReturnTestResults(l_loadPathLossTableReturnMap);


    return err;
}

void InitializeloadPathLossTableContainers(void)
{
    /*------------------*
     * Input Parameters: *
     * IQTESTER_IP01    *
     *------------------*/
    l_loadPathLossTableParamMap.clear();

    WIMAX_SETTING_STRUCT setting;

    setting.unit        = "";
    setting.helpText    = "A comma delimited text file that contains Tx path loss table.\r\nEach row starts with a frequency in MHz, followed by a number of path loss values.\r\n";
    setting.type = WIMAX_SETTING_TYPE_STRING;
    strcpy_s(l_loadPathLossTableParam.TX_PATH_LOSS_FILE, MAX_BUFFER_SIZE, "path_loss_WiMAX.csv");
    if (MAX_BUFFER_SIZE==sizeof(l_loadPathLossTableParam.TX_PATH_LOSS_FILE))    // Type_Checking
    {
        setting.value       = (void*)l_loadPathLossTableParam.TX_PATH_LOSS_FILE;
        l_loadPathLossTableParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX_PATH_LOSS_FILE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.unit        = "";
    setting.helpText    = "A comma delimited text file that contains Rx path loss table.\r\nEach row starts with a frequency in MHz, followed by a number of path loss values.\r\n";
    setting.type = WIMAX_SETTING_TYPE_STRING;
    strcpy_s(l_loadPathLossTableParam.RX_PATH_LOSS_FILE, MAX_BUFFER_SIZE, "path_loss_WiMAX.csv");
    if (MAX_BUFFER_SIZE==sizeof(l_loadPathLossTableParam.RX_PATH_LOSS_FILE))    // Type_Checking
    {
        setting.value       = (void*)l_loadPathLossTableParam.RX_PATH_LOSS_FILE;
        l_loadPathLossTableParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RX_PATH_LOSS_FILE", setting) );
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
    l_loadPathLossTableReturnMap.clear();

    l_loadPathLossTableReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIMAX_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_loadPathLossTableReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_loadPathLossTableReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_loadPathLossTableReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return;
}
