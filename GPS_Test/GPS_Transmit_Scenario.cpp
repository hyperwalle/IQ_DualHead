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
map<string, GPS_SETTING_STRUCT> l_TSParamMap;

// Return Value Container
map<string, GPS_SETTING_STRUCT> l_TSReturnMap;

struct tagParam
{
	char    SCENARIO_FILE[MAX_BUFFER_SIZE];
	int     TIMEOUT;
	double  POWER;
	double  CABLE_LOSS_DB;
	int		ANTENNA;
} l_TSParam;

struct tagReturn
{
	int     SATELLITE_NUMBER_1;
	double  CN_1;
	int     SATELLITE_NUMBER_2;
	double  CN_2;
	int     SATELLITE_NUMBER_3;
	double  CN_3;
	int     SATELLITE_NUMBER_4;
	double  CN_4;
	int     SATELLITE_NUMBER_5;
	double  CN_5;
	int     SATELLITE_NUMBER_6;
	double  CN_6;

	double  FIXED_TIME;
	double  LONGITUDE;
	double  LATITUDE;
	double  ALTITUDE;
	char    ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_TSReturn;

void ClearTransmitScenarioReturn(void)
{
	l_TSParamMap.clear();
	l_TSReturnMap.clear();
}


GPS_TEST_API int GPS_Transmit_Scenario(void)
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
	ClearReturnParameters(l_TSReturnMap);

	/*------------------------*
	 * Respond to QUERY_INPUT *
	 *------------------------*/
	err = TM_GetIntegerParameter(g_GPS_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_TSParamMap);
		return err;
	}
	else{}

	/*-------------------------*
	 * Respond to QUERY_RETURN *
	 *-------------------------*/
	err = TM_GetIntegerParameter(g_GPS_Test_ID, "QUERY_RETURN", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryReturn(l_TSReturnMap);
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
		err = GetInputParameters(l_TSParamMap);
		if ( ERR_OK!=err )
		{
			//err = ::TM_GetStringParameter(g_GPS_Test_ID, "SCENARIO_FILE", l_TSParam.SCENARIO_FILE, MAX_BUFFER_SIZE);
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
		err = ::vDUT_AddStringParameter(g_GPS_Dut, "LOG_FILE_PATH", g_GPSGlobalSettingParam.LOG_FILE_PATH);
		err = ::vDUT_AddIntegerParameter(g_GPS_Dut, "DELETE_TEMP_FILES", g_GPSGlobalSettingParam.DELETE_TEMP_FILES);

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
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[GPS] Set GPS activate LP_GPS_SetActivate() return OK.\n");
		}

		char fullfile[MAX_BUFFER_SIZE] = {'\0'};
		sprintf_s(fullfile, MAX_BUFFER_SIZE, "%s/%s", g_GPSGlobalSettingParam.SCENARIO_FILE_PATH, l_TSParam.SCENARIO_FILE);

		err = LP_GPS_LoadScenarioFile(
				fullfile, //"../mod/IQNavScenarioFile2008241_LondonBigBen.xml",
				(IQV_GPS_TRIGGER_STATE)g_GPSGlobalSettingParam.TRIGGER_STATE);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] Fail to load scenario file, LP_GPS_LoadScenarioFile() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[GPS] load scenario file LP_GPS_LoadScenarioFile() return OK.\n");
		}

		err = LP_GPS_PlayScenarioFile(
				l_TSParam.POWER,
				l_TSParam.CABLE_LOSS_DB);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] Fail to play scenario file, LP_GPS_PlayScenarioFile() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[GPS] Play scenario file LP_GPS_PlayScenarioFile() return OK.\n");
		}

		// [Jarir Fadlullah] Pass timeout value to GPS factory test configuration, for Brcm 2076 GPS DUT
		//Sleep(l_TSParam.TIMEOUT*1000);
		///////////////////////////////

		// [Jarir Fadlullah] Commented for Brcm 2076 GPS DUT
		//err = vDUT_Run(g_GPS_Dut, "RX_GET_MEASUREMENTS");
		/////////////////////////////////////////////////////

		// [Jarir Fadlullah] Add vDUT parameters for Brcm 2076 GPS DUT
		err = ::vDUT_AddIntegerParameter(g_GPS_Dut, "ANTENNA", l_TSParam.ANTENNA);
		err = ::vDUT_AddIntegerParameter(g_GPS_Dut, "TIMEOUT", l_TSParam.TIMEOUT);
		err = ::vDUT_AddStringParameter(g_GPS_Dut, "LOG_FILE_PATH", g_GPSGlobalSettingParam.LOG_FILE_PATH);
		err = ::vDUT_AddStringParameter(g_GPS_Dut, "CONFIG_TEMPLATE_XML", g_GPSGlobalSettingParam.CONFIG_TEMPLATE_XML);
		/////////////////////////////////////////////////////

		// [Jarir Fadlullah] Moved from below for Brcm 2076 GPS DUT
		err = vDUT_Run(g_GPS_Dut, "RX_GET_POSITION");
		if ( ERR_OK!=err )
		{
			vDutActived = false;
			//Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			//err = vDUT_GetStringReturn(g_GPS_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==(vDUT_GetStringReturn(g_GPS_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE)) )	// Get "ERROR_MESSAGE" from vDUT
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}else{	// Return error message

				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] vDUT_Run(RX_GET_MEASUREMENTS) return error.\n");
				throw logMessage;
			}
		}

		/////////////////////////////////////////////////////

		err = ::vDUT_GetIntegerReturn( g_GPS_Dut, "SATELLITE_NUMBER_1", &l_TSReturn.SATELLITE_NUMBER_1);
		err = ::vDUT_GetIntegerReturn( g_GPS_Dut, "SATELLITE_NUMBER_2", &l_TSReturn.SATELLITE_NUMBER_2);
		err = ::vDUT_GetIntegerReturn( g_GPS_Dut, "SATELLITE_NUMBER_3", &l_TSReturn.SATELLITE_NUMBER_3);
		err = ::vDUT_GetIntegerReturn( g_GPS_Dut, "SATELLITE_NUMBER_4", &l_TSReturn.SATELLITE_NUMBER_4);
		err = ::vDUT_GetIntegerReturn( g_GPS_Dut, "SATELLITE_NUMBER_5", &l_TSReturn.SATELLITE_NUMBER_5);
		err = ::vDUT_GetIntegerReturn( g_GPS_Dut, "SATELLITE_NUMBER_6", &l_TSReturn.SATELLITE_NUMBER_6);

		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_1", &l_TSReturn.CN_1);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_2", &l_TSReturn.CN_2);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_3", &l_TSReturn.CN_3);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_4", &l_TSReturn.CN_4);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_5", &l_TSReturn.CN_5);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_6", &l_TSReturn.CN_6);


		// [Jarir Fadlullah] Moved to above for Brcm 2076 GPS DUT
		//err = vDUT_Run(g_GPS_Dut, "RX_GET_POSITION");
		/////////////////////////////////////////////////////////

		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "LONGITUDE", &l_TSReturn.LONGITUDE);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "LATITUDE", &l_TSReturn.LATITUDE);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "ALTITUDE", &l_TSReturn.ALTITUDE);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "FIXED_TIME", &l_TSReturn.FIXED_TIME);

		// [Jarir Fadlullah] Include these in data reporting from Brcm 2076 GPS DUT
		// PDOP (position dilution of precision) from $GPGSA
		// HDOP (horizontal dilution of precision) from $GPGSA
		// VDOP (vertical dilution of precision) from $GPGSA
		// TTFF (Time to Fix) from $PGLOR,0,FIX
		// coordinates also from $GPRMC, fix valid indicator, speed, track
		// Better data from GL100 Tim, GL100Pos, Satellites from GL103
		// [Jarir Fadlullah]


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
			sprintf_s(l_TSReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_TSReturnMap);
		}
		else{}
#pragma endregion

	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_TSReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		ReturnErrorMessage(l_TSReturn.ERROR_MESSAGE, "[GPS] Unknown Error!\n");
		err = -1;
	}

	return err;
}



void InitializeGPSTransmitScenarioContainers(void)
{
	GPS_SETTING_STRUCT setting;

#pragma region Step 1 : Setup & Init InputParm
	/*-------------------*
	 * Input Parameters: *
	 * IQTESTER_IP01     *
	 *-------------------*/

	l_TSParamMap.clear();

	/* input:Scenario_File */
	strcpy_s(l_TSParam.SCENARIO_FILE, MAX_BUFFER_SIZE, "IQNavScenarioFile2008241_LondonBigBen.xml");
	setting.type = GPS_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_TSParam.SCENARIO_FILE))    // Type_Checking
	{
		setting.value = (void*)l_TSParam.SCENARIO_FILE;
		setting.unit        = "";
		setting.helpText    = "the ScenarioFile under \"SCENARIO_FILE_PATH\".\r\n";
		l_TSParamMap.insert( pair<string,GPS_SETTING_STRUCT>("SCENARIO_FILE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* input:Timeout */
	l_TSParam.TIMEOUT = 60;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_TSParam.TIMEOUT))    // Type_Checking
	{
		setting.value = (void*)&l_TSParam.TIMEOUT;
		setting.unit        = "sec";
		setting.helpText    = "Timeout : transmit signal till timeout (default 60 second).\r\n";
		l_TSParamMap.insert( pair<string,GPS_SETTING_STRUCT>("TIMEOUT", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* input:Power */
	l_TSParam.POWER = -130;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_TSParam.POWER))    // Type_Checking
	{
		setting.value = (void*)&l_TSParam.POWER;
		setting.unit        = "dBm";
		setting.helpText    = "Power at receiver (default : -130dBm).\r\n";
		l_TSParamMap.insert( pair<string,GPS_SETTING_STRUCT>("POWER", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* input:CABLE_LOSS_DB */
	l_TSParam.CABLE_LOSS_DB = 0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_TSParam.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value = (void*)&l_TSParam.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cableloss between transmiter & receiver (default : 0dB).\r\n";
		l_TSParamMap.insert( pair<string,GPS_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// [Jarir Fadlullah] Start: Added for Brcm 2076 GPS
	l_TSParam.ANTENNA = 0; // 0: GL_RF_BRCM_2076, 1: GL_RF_BRCM_2076_EXT_LNA
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_TSParam.ANTENNA))    // Type_Checking
	{
		setting.value = (void*)&l_TSParam.ANTENNA;
		setting.unit        = "";
		setting.helpText    = "Choose INT LNA or EXT LNA for BRCM 2076 GPS DUT. (0: GL_RF_2076_BRCM (default INT LNA), 1: GL_RF_2076_BRCM_EXT_LNA).\r\n";
		l_TSParamMap.insert( pair<string,GPS_SETTING_STRUCT>("ANTENNA", setting) );
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

	/** ----------   **/
	/* return:SATELLITE_NUMBER  */
	l_TSReturn.SATELLITE_NUMBER_1 = 0;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_TSReturn.SATELLITE_NUMBER_1))    // Type_Checking
	{
		setting.value = (void*)&l_TSReturn.SATELLITE_NUMBER_1;
		setting.unit        = "";
		setting.helpText    = "Satellite Number.(between 1-37)\r\n";
		l_TSReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_1", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	/* return:CN Value  */
	l_TSReturn.CN_1 = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_TSReturn.CN_1))    // Type_Checking
	{
		setting.value = (void*)&l_TSReturn.CN_1;
		setting.unit        = "dB";
		setting.helpText    = "C/N vlaue of the satellite.\r\n";
		l_TSReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_1", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* return:SATELLITE_NUMBER  */
	l_TSReturn.SATELLITE_NUMBER_2 = 0;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_TSReturn.SATELLITE_NUMBER_2))    // Type_Checking
	{
		setting.value = (void*)&l_TSReturn.SATELLITE_NUMBER_2;
		setting.unit        = "";
		setting.helpText    = "Satellite Number.(between 1-37)\r\n";
		l_TSReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_2", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* return:CN Value  */
	l_TSReturn.CN_2 = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_TSReturn.CN_2))    // Type_Checking
	{
		setting.value = (void*)&l_TSReturn.CN_2;
		setting.unit        = "dB";
		setting.helpText    = "C/N vlaue of the satellite.\r\n";
		l_TSReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_2", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* return:SATELLITE_NUMBER  */
	l_TSReturn.SATELLITE_NUMBER_3 = 0;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_TSReturn.SATELLITE_NUMBER_3))    // Type_Checking
	{
		setting.value = (void*)&l_TSReturn.SATELLITE_NUMBER_3;
		setting.unit        = "";
		setting.helpText    = "Satellite Number.(between 1-37)\r\n";
		l_TSReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_3", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* return:CN Value  */
	l_TSReturn.CN_3 = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_TSReturn.CN_3))    // Type_Checking
	{
		setting.value = (void*)&l_TSReturn.CN_3;
		setting.unit        = "dB";
		setting.helpText    = "C/N vlaue of the satellite.\r\n";
		l_TSReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_3", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* return:SATELLITE_NUMBER  */
	l_TSReturn.SATELLITE_NUMBER_4 = 0;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_TSReturn.SATELLITE_NUMBER_4))    // Type_Checking
	{
		setting.value = (void*)&l_TSReturn.SATELLITE_NUMBER_4;
		setting.unit        = "";
		setting.helpText    = "Satellite Number.(between 1-37)\r\n";
		l_TSReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_4", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* return:CN Value  */
	l_TSReturn.CN_4 = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_TSReturn.CN_4))    // Type_Checking
	{
		setting.value = (void*)&l_TSReturn.CN_4;
		setting.unit        = "dB";
		setting.helpText    = "C/N vlaue of the satellite.\r\n";
		l_TSReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_4", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* return:SATELLITE_NUMBER  */
	l_TSReturn.SATELLITE_NUMBER_5 = 0;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_TSReturn.SATELLITE_NUMBER_5))    // Type_Checking
	{
		setting.value = (void*)&l_TSReturn.SATELLITE_NUMBER_5;
		setting.unit        = "";
		setting.helpText    = "Satellite Number.(between 1-37)\r\n";
		l_TSReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_5", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	/* return:CN Value  */
	l_TSReturn.CN_5 = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_TSReturn.CN_5))    // Type_Checking
	{
		setting.value = (void*)&l_TSReturn.CN_5;
		setting.unit        = "dB";
		setting.helpText    = "C/N vlaue of the satellite.\r\n";
		l_TSReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_5", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* return:SATELLITE_NUMBER  */
	l_TSReturn.SATELLITE_NUMBER_6 = 0;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_TSReturn.SATELLITE_NUMBER_6))    // Type_Checking
	{
		setting.value = (void*)&l_TSReturn.SATELLITE_NUMBER_6;
		setting.unit        = "";
		setting.helpText    = "Satellite Number.(between 1-37)\r\n";
		l_TSReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_6", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* return:CN Value  */
	l_TSReturn.CN_6 = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_TSReturn.CN_6))    // Type_Checking
	{
		setting.value = (void*)&l_TSReturn.CN_6;
		setting.unit        = "dB";
		setting.helpText    = "C/N vlaue of the satellite.\r\n";
		l_TSReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_6", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* return:LATITUDE  */
	l_TSReturn.LATITUDE = 0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_TSReturn.LATITUDE))    // Type_Checking
	{
		setting.value = (void*)&l_TSReturn.LATITUDE;
		setting.unit        = "degree";
		setting.helpText    = "Location : Latitude.\r\n";
		l_TSReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("LATITUDE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* return:LONGITUDE  */
	l_TSReturn.LONGITUDE = 0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_TSReturn.LONGITUDE))    // Type_Checking
	{
		setting.value = (void*)&l_TSReturn.LONGITUDE;
		setting.unit        = "degree";
		setting.helpText    = "Location : Longitude.\r\n";
		l_TSReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("LONGITUDE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* return:ALTITUDE  */
	l_TSReturn.ALTITUDE = 0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_TSReturn.ALTITUDE))    // Type_Checking
	{
		setting.value = (void*)&l_TSReturn.ALTITUDE;
		setting.unit        = "m";
		setting.helpText    = "Location : Altitude.\r\n";
		l_TSReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("ALTITUDE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* return:FIXED_TIME  */
	l_TSReturn.FIXED_TIME = 0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_TSReturn.FIXED_TIME))    // Type_Checking
	{
		setting.value = (void*)&l_TSReturn.FIXED_TIME;
		setting.unit        = "sec";
		setting.helpText    = "GPS Location fixed time (sec).\r\n";
		l_TSReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("FIXED_TIME", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* return:error_message  */
	strcpy_s(l_TSReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "");
	setting.type = GPS_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_TSReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value = (void*)l_TSReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "";
		l_TSReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

#pragma endregion
}


