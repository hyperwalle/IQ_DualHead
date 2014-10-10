#include "stdafx.h"
#include "TestManager.h"
#include "GPS_Test.h"
#include "GPS_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"


using namespace std;

// These global variables are declared in GPS_Test_Internal.cpp
extern TM_ID                 g_GPS_Test_ID;    
extern vDUT_ID               g_GPS_Dut;

// This global variable is declared in GPS_Global_Setting.cpp
extern GPS_GLOBAL_SETTING g_GPSGlobalSettingParam;

// Input Parameter Container
map<string, GPS_SETTING_STRUCT> l_CWParamMap;

// Return Value Container 
map<string, GPS_SETTING_STRUCT> l_CWReturnMap;

struct tagParam
{
	int     TIMEOUT;
	double  POWER;
    double  CABLE_LOSS_DB;
	int FREQUENCY_OFFSET;

} l_CWParam;

struct tagReturn
{
    char    ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_CWReturn;

void ClearContinueWaveReturn(void)
{
	l_CWParamMap.clear();
	l_CWReturnMap.clear();
}


GPS_TEST_API int GPS_ContinueWave(void)
{
    int		err = ERR_OK;	
    int		dummyValue = 0;
//	double  cableLossDb = 0;
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
	ClearReturnParameters(l_CWReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_GPS_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_CWParamMap);
        return err;
    }
	else{}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_GPS_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_CWReturnMap);
        return err;
    }
	else{}
#pragma endregion

	try
	{
#pragma region Step 2
        /*
        1. g_GPS_Test_ID needs to be valid (>=0)
        2. Get input parameters
        */

	   /*--------------------------------------*
		* g_GPS_Test_ID needs to be valid (>=0) *
		*--------------------------------------*/
        TM_ClearReturns(g_GPS_Test_ID);
		if( g_GPS_Test_ID<0 || g_GPS_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] g_GPS_Test_ID or GPS_Dut is not valid.\n");
			throw logMessage;
		}
		else{}

       /*----------------------*
        * Get input parameters *
        *----------------------*/
        err = GetInputParameters(l_CWParamMap);
		if ( ERR_OK!=err )
		{
			//err = ::TM_GetStringParameter(g_GPS_Test_ID, "SCENARIO_FILE", l_MMParam.SCENARIO_FILE, MAX_BUFFER_SIZE);
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[GPS] Get input parameters return OK.\n");
		}
#pragma endregion


#pragma region Step 3
        /*
        1. Configure DUT to Receive
        */

        bool vDutActived = false;
		/*------------------------------------------*
		 * Configure DUT to Receive - PRBS9 Pattern *
		 *------------------------------------------*/
		// Set DUT RF frequency...
		// And clear vDUT parameters at the beginning.
		vDUT_ClearParameters(g_GPS_Dut);		

		err = vDUT_Run(g_GPS_Dut, "RX_CLEAR_STATS");
		if ( ERR_OK!=err )
		{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.
			err = vDUT_GetStringReturn(g_GPS_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else
			{	// Just return normal error message in this case
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] vDUT_Run(RX_CLEAR_STATS) return error.\n");
				throw logMessage;
			}
		}
		else{}

		err = vDUT_Run(g_GPS_Dut, "RX_START");		
		if ( ERR_OK!=err )
		{	
			vDutActived = false;
			//Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else
			{	// Return error message			
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] vDUT_Run(RX_START) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			vDutActived = true;
		}

		// Delay for DUT settle
        
		if (0!=g_GPSGlobalSettingParam.DUT_RX_SETTLE_TIME_MS)
		{
			Sleep(g_GPSGlobalSettingParam.DUT_RX_SETTLE_TIME_MS);
		}
		else{}
        
#pragma endregion

#pragma region Step 4 : Main Step
        /*-----------------------*
		 *  Control Tester       *
		 *-----------------------*/
        err = LP_GPS_SetActivate();
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] Fail to set GPS activate, LP_GPS_SetActivate() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[GPS] set GPS activate LP_GPS_SetActivate() return OK.\n");
		}

        err = LP_GPS_ContinueWave(
								  l_CWParam.POWER, 
								  l_CWParam.CABLE_LOSS_DB, 
								  (IQV_GPS_TRIGGER_STATE)g_GPSGlobalSettingParam.TRIGGER_STATE,
								  l_CWParam.FREQUENCY_OFFSET);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] Fail to start continue wave, LP_GPS_ContinueWave() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[GPS] Start continue wave LP_GPS_SetActivate() return OK.\n");
		}

        //itoa(err, l_CWReturn.ERROR_MESSAGE, 10);
        Sleep(l_CWParam.TIMEOUT*1000);

        err = LP_GPS_SetDeactivate();
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] Fail to set GPS deactivate, LP_GPS_SetDeactivate() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[GPS] Set GPS deactivate LP_GPS_SetDeactivate() return OK.\n");
		}

#pragma endregion

#pragma region Step 5
		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
            /*-----------------------*
		     *  Get Info from DUT    *
		     *-----------------------*/
			sprintf_s(l_CWReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_CWReturnMap);
		}
		else{}

#pragma endregion

	}
	catch(char *msg)
	{
        ReturnErrorMessage(l_CWReturn.ERROR_MESSAGE, msg);
    }
	catch(...)
	{
		ReturnErrorMessage(l_CWReturn.ERROR_MESSAGE, "[GPS] Unknown Error!\n");
		err = -1;
    }


    return err;
}



void InitializeGPSContinueWaveContainers(void)
{
    GPS_SETTING_STRUCT setting;

#pragma region Step 1 : Setup & Init InputParm
    /*-------------------*
     * Input Parameters: *
     * IQTESTER_IP01     *
     *-------------------*/

    l_CWParamMap.clear();

    /* input:POWER  */
    l_CWParam.POWER = -60.0;
    setting.type = GPS_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_CWParam.POWER))    // Type_Checking
    {
        setting.value = (void*)&l_CWParam.POWER;
        setting.unit        = "dBm";
        setting.helpText    = "Power value of the GPS received.\n(-145~-60dBm).\r\n";
        l_CWParamMap.insert( pair<string,GPS_SETTING_STRUCT>("POWER", setting) );
    }
	else
	{
        printf("Parameter Type Error!\n");
        exit(1);
    }

    /* input:Timeout */
    l_CWParam.TIMEOUT = 20;
    setting.type = GPS_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_CWParam.TIMEOUT))    // Type_Checking
    {
        setting.value = (void*)&l_CWParam.TIMEOUT;
        setting.unit        = "sec";
        setting.helpText    = "Timeout : transmit signal till timeout (default 20 second).\r\n";
        l_CWParamMap.insert( pair<string,GPS_SETTING_STRUCT>("TIMEOUT", setting) );
    }
	else
	{
        printf("Parameter Type Error!\n");
        exit(1);
    }


    /* input:CABLE_LOSS_DB */
    l_CWParam.CABLE_LOSS_DB = 0;
    setting.type = GPS_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_CWParam.CABLE_LOSS_DB))    // Type_Checking
    {
        setting.value = (void*)&l_CWParam.CABLE_LOSS_DB;
        setting.unit        = "dB";
        setting.helpText    = "Cableloss between transmiter & receiver (default : 0dB).\r\n";
        l_CWParamMap.insert( pair<string,GPS_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
    }
	else
	{
        printf("Parameter Type Error!\n");
        exit(1);
    }
	
	/* input:FREQUENCY_OFFSET */
	l_CWParam.FREQUENCY_OFFSET = 0;
    setting.type = GPS_SETTING_TYPE_INTEGER;
    if (sizeof(long int)==sizeof(l_CWParam.FREQUENCY_OFFSET))    // Type_Checking
    {
        setting.value = (void*)&l_CWParam.FREQUENCY_OFFSET;
        setting.unit        = "kHz";
        setting.helpText    = "Frequency offset (default : 0kHz). It can range from -500kHz to 500kHz.\r\n";
        l_CWParamMap.insert( pair<string,GPS_SETTING_STRUCT>("FREQUENCY_OFFSET", setting) );
    }
	else
	{
        printf("Parameter Type Error!\n");
        exit(1);
    }
    
#pragma endregion

#pragma region Step 2 : Setup & Init ReturnValue
    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/

    /* return:error_message  */
    strcpy_s(l_CWReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "");
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_CWReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value = (void*)l_CWReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "";
        l_CWReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
	else
	{
        printf("Parameter Type Error!\n");
        exit(1);
    }
#pragma endregion
}


