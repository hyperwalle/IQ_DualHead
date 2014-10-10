#include "stdafx.h"
#include "TestManager.h"
#include "GPS_Test.h"
#include "GPS_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

// This variable is declared in GPS_Test_Internal.cpp
extern vDUT_ID      g_GPS_Dut;
extern TM_ID        g_GPS_Test_ID;

using namespace std;

// Input Parameter Container
map<string, GPS_SETTING_STRUCT> g_GPSGlobalSettingParamMap;

// Return Value Container 
map<string, GPS_SETTING_STRUCT> l_GPSGlobalSettingReturnMap;

GPS_GLOBAL_SETTING g_GPSGlobalSettingParam;

struct tagReturn
{
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} g_GPSGlobalSettingReturn;

void ClearGlobalSettingReturn(void)
{
	g_GPSGlobalSettingParamMap.clear();
	l_GPSGlobalSettingReturnMap.clear();
}


GPS_TEST_API int GPS_Global_Setting(void)
{
    int		err = ERR_OK;
    int		dummyValue = 0;
	char	logMessage[MAX_BUFFER_SIZE] = {'\0'};

#pragma region Step 1
    /*
    1. Clear Return Parameters and Container
    2. Respond to QUERY_INPUT
    3. Respond to QUERY_RETURN
    */

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_GPSGlobalSettingReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_GPS_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(g_GPSGlobalSettingParamMap);
        return err;
    }else{}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_GPS_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_GPSGlobalSettingReturnMap);
        return err;
    }else{}
#pragma endregion

	try
	{
#pragma region Step 2
        /*
        1. g_GPS_Test_ID needs to be valid (>=0)
        2. Get input parameters
        */

	   /*---------------------------------------*
		* g_GPS_Test_ID need to be valid (>=0) *
		*---------------------------------------*/
		TM_ClearReturns(g_GPS_Test_ID);
		if( g_GPS_Test_ID<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] GPS_Test_ID not valid.\n");
			throw logMessage;
		}else{}

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		GetInputParameters(g_GPSGlobalSettingParamMap);
		err = ERR_OK;	// Global Setting are optinal parameters, thus err always = ERR_OK
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
			sprintf_s(g_GPSGlobalSettingReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_GPSGlobalSettingReturnMap);
		}
		else
		{
			// do nothing 
		}
#pragma endregion

	}
	catch(char *msg)
	{
        ReturnErrorMessage(g_GPSGlobalSettingReturn.ERROR_MESSAGE, msg);
    }
	catch(...)
	{
		ReturnErrorMessage(g_GPSGlobalSettingReturn.ERROR_MESSAGE, "[GPS] Unknown Error!\n");
		err = -1;
    }

    return err;
}

void InitializeGlobalSettingContainers(void)
{
    GPS_SETTING_STRUCT setting;

#pragma region Step 1 : Setup & Init InputParm
    /*------------------*
     * Input Paramters: *
     *------------------*/
    g_GPSGlobalSettingParamMap.clear();

    /* input:DUT_RX_SETTLE_TIME_MS */
    setting.type = GPS_SETTING_TYPE_INTEGER;
	g_GPSGlobalSettingParam.DUT_RX_SETTLE_TIME_MS = 0;	
    if (sizeof(int)==sizeof(g_GPSGlobalSettingParam.DUT_RX_SETTLE_TIME_MS))    // Type_Checking
    {
        setting.value = (void*)&g_GPSGlobalSettingParam.DUT_RX_SETTLE_TIME_MS;
        setting.unit  = "ms";
        setting.helpText  = "A delay time for DUT (RX) settle, Default = 0(ms).";
        g_GPSGlobalSettingParamMap.insert( pair<string, GPS_SETTING_STRUCT>("DUT_RX_SETTLE_TIME_MS", setting) );
    }else{
        printf("Parameter Type Error!\n");
        exit(1);
    }

    /* input:doppler_Frequency */
    g_GPSGlobalSettingParam.DOPPLER_FREQUENCY = 0;
    setting.type = GPS_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(g_GPSGlobalSettingParam.DOPPLER_FREQUENCY))    // Type_Checking
    {
        setting.value = (void*)&g_GPSGlobalSettingParam.DOPPLER_FREQUENCY;
        setting.unit        = "Hz";
        setting.helpText    = "doppler_Frequency : doppler_Frequency for red-motivation fix (default 0).\r\n";
        g_GPSGlobalSettingParamMap.insert( pair<string,GPS_SETTING_STRUCT>("DOPPLER_FREQUENCY", setting) );
    }else{
        printf("Parameter Type Error!\n");
        exit(1);
    }

    /* input:trigger_State */
    g_GPSGlobalSettingParam.TRIGGER_STATE = 0;
    setting.type = GPS_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(g_GPSGlobalSettingParam.TRIGGER_STATE))    // Type_Checking
    {
        setting.value = (void*)&g_GPSGlobalSettingParam.TRIGGER_STATE;
        setting.unit        = "";
        setting.helpText    = "triggerState : setting tester trigger state \n (default internal:0 ; external:1).\r\n";
        g_GPSGlobalSettingParamMap.insert( pair<string,GPS_SETTING_STRUCT>("TRIGGER_STATE", setting) );
    }else{
        printf("Parameter Type Error!\n");
        exit(1);
    }

    g_GPSGlobalSettingParam.SCENARIO_FILE_PATH[0] = '\0';
    sprintf_s(g_GPSGlobalSettingParam.SCENARIO_FILE_PATH, MAX_BUFFER_SIZE, "../mod");
	setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(g_GPSGlobalSettingParam.SCENARIO_FILE_PATH))    // Type_Checking
    {
        setting.value       = (void*)g_GPSGlobalSettingParam.SCENARIO_FILE_PATH;
        setting.unit        = "";
        setting.helpText    = "FilePath to access the ScenarioFile from execute location.\r\n";
        g_GPSGlobalSettingParamMap.insert( pair<string,GPS_SETTING_STRUCT>("SCENARIO_FILE_PATH", setting) );
    }else{
        printf("Parameter Type Error!\n");
        exit(1);
    }

	///////////////////////////////////////////////////////////
	// [Jarir Fadlullah] Start: Added for Brcm 2076 GPS DUT
    g_GPSGlobalSettingParam.LOG_FILE_PATH[0] = '\0';
    sprintf_s(g_GPSGlobalSettingParam.LOG_FILE_PATH, MAX_BUFFER_SIZE, "../GPSLog");
	setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(g_GPSGlobalSettingParam.LOG_FILE_PATH))    // Type_Checking
    {
        setting.value       = (void*)g_GPSGlobalSettingParam.LOG_FILE_PATH;
        setting.unit        = "";
        setting.helpText    = "FilePath to store GPS DUT output log from execute location.\r\n";
        g_GPSGlobalSettingParamMap.insert( pair<string,GPS_SETTING_STRUCT>("LOG_FILE_PATH", setting) );
    }else{
        printf("Parameter Type Error!\n");
        exit(1);
    }

    g_GPSGlobalSettingParam.CONFIG_TEMPLATE_XML[0] = '\0';
    sprintf_s(g_GPSGlobalSettingParam.CONFIG_TEMPLATE_XML, MAX_BUFFER_SIZE, "gpsconfig_h4.xml");
	setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(g_GPSGlobalSettingParam.CONFIG_TEMPLATE_XML))    // Type_Checking
    {
        setting.value       = (void*)g_GPSGlobalSettingParam.CONFIG_TEMPLATE_XML;
        setting.unit        = "";
        setting.helpText    = "BRCM 2076 GPS xml config template file.\r\n";
        g_GPSGlobalSettingParamMap.insert( pair<string,GPS_SETTING_STRUCT>("CONFIG_TEMPLATE_XML", setting) );
    }else{
        printf("Parameter Type Error!\n");
        exit(1);
    }

    g_GPSGlobalSettingParam.DELETE_TEMP_FILES = 1;
    setting.type = GPS_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(g_GPSGlobalSettingParam.DELETE_TEMP_FILES))    // Type_Checking
    {
        setting.value = (void*)&g_GPSGlobalSettingParam.DELETE_TEMP_FILES;
        setting.unit        = "";
		setting.helpText    = "Delete temporary GPS logs and XML config files (default: 1).\r\n";
        g_GPSGlobalSettingParamMap.insert( pair<string,GPS_SETTING_STRUCT>("DELETE_TEMP_FILES", setting) );
    }else{
        printf("Parameter Type Error!\n");
        exit(1);
    }

	g_GPSGlobalSettingParam.GPS_HOST_SHELL_SCRIPT[0] = '\0';
    sprintf_s(g_GPSGlobalSettingParam.GPS_HOST_SHELL_SCRIPT, MAX_BUFFER_SIZE, "gpstest.sh");
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(g_GPSGlobalSettingParam.GPS_HOST_SHELL_SCRIPT))	// Type_Checking
    {
 	   setting.value	   = (void*)g_GPSGlobalSettingParam.GPS_HOST_SHELL_SCRIPT;
 	   setting.unit 	   = "";
 	   setting.helpText    = "Host OS path to store test scripts and data files.\r\n";
 	   g_GPSGlobalSettingParamMap.insert( pair<string,GPS_SETTING_STRUCT>("GPS_HOST_SHELL_SCRIPT", setting) );
    }else{
 	   printf("Parameter Type Error!\n");
 	   exit(1);
    }

	g_GPSGlobalSettingParam.GPS_HOST_PATH[0] = '\0';
    sprintf_s(g_GPSGlobalSettingParam.GPS_HOST_PATH, MAX_BUFFER_SIZE, "/data/");
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(g_GPSGlobalSettingParam.GPS_HOST_PATH))	// Type_Checking
    {
 	   setting.value	   = (void*)g_GPSGlobalSettingParam.GPS_HOST_PATH;
 	   setting.unit 	   = "";
 	   setting.helpText    = "Host OS path to store test scripts and data files.\r\n";
 	   g_GPSGlobalSettingParamMap.insert( pair<string,GPS_SETTING_STRUCT>("GPS_HOST_PATH", setting) );
    }else{
 	   printf("Parameter Type Error!\n");
 	   exit(1);
    }


	// [Jarir Fadlullah] End: Added for Brcm 2076 GPS DUT
	///////////////////////////////////////////////////////////


#pragma endregion

#pragma region Step 2 : Setup & Init ReturnValue
    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_GPSGlobalSettingReturnMap.clear();

    g_GPSGlobalSettingReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(g_GPSGlobalSettingReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)g_GPSGlobalSettingReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_GPSGlobalSettingReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }else{
        printf("Parameter Type Error!\n");
        exit(1);
    }

#pragma endregion
}
