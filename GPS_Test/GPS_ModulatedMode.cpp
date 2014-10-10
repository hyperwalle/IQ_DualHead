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
map<string, GPS_SETTING_STRUCT> l_MMParamMap;

// Return Value Container
map<string, GPS_SETTING_STRUCT> l_MMReturnMap;

struct tagParam
{
	int     SATELLITE_NUMBER_1;
	double  POWER_1;
	int     SATELLITE_NUMBER_2;
	double  POWER_2;
	int     SATELLITE_NUMBER_3;
	double  POWER_3;
	int     SATELLITE_NUMBER_4;
	double  POWER_4;
	int     SATELLITE_NUMBER_5;
	double  POWER_5;
	int     SATELLITE_NUMBER_6;
	double  POWER_6;

	//char    SCENARIO_FILE[MAX_BUFFER_SIZE];
	int     TIMEOUT;
	//double  POWER;
	double  CABLE_LOSS_DB;

	int     MODULATED_MODE_NAV_DATA;

	// [Jarir Fadlullah] Added for Brcm 2076 GPS
	int ANTENNA;
	////////////////////////////////////////////
} l_MMParam;

struct tagReturn
{
	int     SATELLITE_NUMBER_1;
	vector <double> CN_1_ARRAY;
	double  CN_1_AVG;				// [Jarir Fadlullah] : Modified CN_1 to CN_1_AVG
	double	CN_BB_1_AVG;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_1_AVG;	// [Jarir Fadlullah] : Added
	double  CN_1_MIN;				// [Jarir Fadlullah] : Added
	double	CN_BB_1_MIN;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_1_MIN;	// [Jarir Fadlullah] : Added
	double  CN_1_MAX;				// [Jarir Fadlullah] : Added
	double	CN_BB_1_MAX;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_1_MAX;	// [Jarir Fadlullah] : Added
	int     SATELLITE_NUMBER_2;
	double  CN_2_AVG;				// [Jarir Fadlullah] : Modified CN_2 to CN_2_AVG
	double	CN_BB_2_AVG;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_2_AVG;	// [Jarir Fadlullah] : Added
	double  CN_2_MIN;				// [Jarir Fadlullah] : Added
	double	CN_BB_2_MIN;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_2_MIN;	// [Jarir Fadlullah] : Added
	double  CN_2_MAX;				// [Jarir Fadlullah] : Added
	double	CN_BB_2_MAX;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_2_MAX;	// [Jarir Fadlullah] : Added
	int     SATELLITE_NUMBER_3;
	double  CN_3_AVG;				// [Jarir Fadlullah] : Modified CN_3 to CN_3_AVG
	double	CN_BB_3_AVG;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_3_AVG;	// [Jarir Fadlullah] : Added
	double  CN_3_MIN;				// [Jarir Fadlullah] : Added
	double	CN_BB_3_MIN;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_3_MIN;	// [Jarir Fadlullah] : Added
	double  CN_3_MAX;				// [Jarir Fadlullah] : Added
	double	CN_BB_3_MAX;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_3_MAX;	// [Jarir Fadlullah] : Added
	int     SATELLITE_NUMBER_4;
	double  CN_4_AVG;				// [Jarir Fadlullah] : Modified CN_4 to CN_4_AVG
	double	CN_BB_4_AVG;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_4_AVG;	// [Jarir Fadlullah] : Added
	double  CN_4_MIN;				// [Jarir Fadlullah] : Added
	double	CN_BB_4_MIN;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_4_MIN;	// [Jarir Fadlullah] : Added
	double  CN_4_MAX;				// [Jarir Fadlullah] : Added
	double	CN_BB_4_MAX;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_4_MAX;	// [Jarir Fadlullah] : Added
	int     SATELLITE_NUMBER_5;
	double  CN_5_AVG;				// [Jarir Fadlullah] : Modified CN_5 to CN_5_AVG
	double	CN_BB_5_AVG;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_5_AVG;	// [Jarir Fadlullah] : Added
	double  CN_5_MIN;				// [Jarir Fadlullah] : Added
	double	CN_BB_5_MIN;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_5_MIN;	// [Jarir Fadlullah] : Added
	double  CN_5_MAX;				// [Jarir Fadlullah] : Added
	double	CN_BB_5_MAX;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_5_MAX;	// [Jarir Fadlullah] : Added
	int     SATELLITE_NUMBER_6;
	double  CN_6_AVG;				// [Jarir Fadlullah] : Modified CN_6 to CN_6_AVG
	double	CN_BB_6_AVG;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_6_AVG;	// [Jarir Fadlullah] : Added
	double  CN_6_MIN;				// [Jarir Fadlullah] : Added
	double	CN_BB_6_MIN;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_6_MIN;	// [Jarir Fadlullah] : Added
	double  CN_6_MAX;				// [Jarir Fadlullah] : Added
	double	CN_BB_6_MAX;			// [Jarir Fadlullah] : Added
	double	SIGNAL_STRENGTH_6_MAX;	// [Jarir Fadlullah] : Added


	char    ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_MMReturn;

void ClearModulatedModeReturn(void)
{
	l_MMParamMap.clear();
	l_MMReturnMap.clear();
}


GPS_TEST_API int GPS_ModulatedMode(void)
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
	ClearReturnParameters(l_MMReturnMap);

	/*------------------------*
	 * Respond to QUERY_INPUT *
	 *------------------------*/
	err = TM_GetIntegerParameter(g_GPS_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_MMParamMap);
		return err;
	}
	else{}

	/*-------------------------*
	 * Respond to QUERY_RETURN *
	 *-------------------------*/
	err = TM_GetIntegerParameter(g_GPS_Test_ID, "QUERY_RETURN", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryReturn(l_MMReturnMap);
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
		err = GetInputParameters(l_MMParamMap);
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
		err = ::vDUT_AddStringParameter(g_GPS_Dut, "LOG_FILE_PATH", g_GPSGlobalSettingParam.LOG_FILE_PATH);
		err = ::vDUT_AddIntegerParameter(g_GPS_Dut, "DELETE_TEMP_FILES", g_GPSGlobalSettingParam.DELETE_TEMP_FILES);

		err = ::vDUT_AddStringParameter(g_GPS_Dut, "GPS_HOST_PATH", g_GPSGlobalSettingParam.GPS_HOST_PATH);
		err = ::vDUT_AddStringParameter(g_GPS_Dut, "GPS_HOST_SHELL_SCRIPT", g_GPSGlobalSettingParam.GPS_HOST_SHELL_SCRIPT);
		err = ::vDUT_AddStringParameter(g_GPS_Dut, "CONFIG_TEMPLATE_XML", g_GPSGlobalSettingParam.CONFIG_TEMPLATE_XML);

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
			}else{	// Return error message

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

		double powerA[6] = {
			l_MMParam.POWER_1,l_MMParam.POWER_2,
			l_MMParam.POWER_3,l_MMParam.POWER_4,
			l_MMParam.POWER_5,l_MMParam.POWER_6
		};

		int satelliteNumA[6] = {
			l_MMParam.SATELLITE_NUMBER_1,l_MMParam.SATELLITE_NUMBER_2,
			l_MMParam.SATELLITE_NUMBER_3,l_MMParam.SATELLITE_NUMBER_4,
			l_MMParam.SATELLITE_NUMBER_5,l_MMParam.SATELLITE_NUMBER_6
		};

		err = LP_GPS_ModulatedMode(
				l_MMParam.MODULATED_MODE_NAV_DATA,
				l_MMParam.CABLE_LOSS_DB,
				powerA,
				satelliteNumA,
				g_GPSGlobalSettingParam.DOPPLER_FREQUENCY,
				(IQV_GPS_TRIGGER_STATE)g_GPSGlobalSettingParam.TRIGGER_STATE);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] Fail to start modulated mode signal, LP_GPS_ModulatedMode() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[GPS] Start modulated mode signal LP_GPS_ModulatedMode() return OK.\n");
		}

		//itoa(err, l_MMReturn.ERROR_MESSAGE, 10);
		////////////////////////////////////////////////
		// [Jarir Fadlullah] For Brcm 2076 GPS DUT, want TIMEOUT to pass to DUT executable, not sleep here
		// Sleep(l_MMParam.TIMEOUT*1000);
		////////////////////////////////////////////////

		// [Jarir Fadlullah] Start: For Brcm 2076 GPS DUT
		err = ::vDUT_AddIntegerParameter(g_GPS_Dut, "SATELLITE_NUMBER_1",			l_MMParam.SATELLITE_NUMBER_1);
		err = ::vDUT_AddIntegerParameter(g_GPS_Dut, "SATELLITE_NUMBER_2",			l_MMParam.SATELLITE_NUMBER_2);
		err = ::vDUT_AddIntegerParameter(g_GPS_Dut, "SATELLITE_NUMBER_3",			l_MMParam.SATELLITE_NUMBER_3);
		err = ::vDUT_AddIntegerParameter(g_GPS_Dut, "SATELLITE_NUMBER_4",			l_MMParam.SATELLITE_NUMBER_4);
		err = ::vDUT_AddIntegerParameter(g_GPS_Dut, "SATELLITE_NUMBER_5",			l_MMParam.SATELLITE_NUMBER_5);
		err = ::vDUT_AddIntegerParameter(g_GPS_Dut, "SATELLITE_NUMBER_6",			l_MMParam.SATELLITE_NUMBER_6);
		err = ::vDUT_AddIntegerParameter(g_GPS_Dut, "TIMEOUT",			l_MMParam.TIMEOUT);
		err = ::vDUT_AddIntegerParameter(g_GPS_Dut, "ANTENNA", l_MMParam.ANTENNA);
		err = ::vDUT_AddStringParameter(g_GPS_Dut, "LOG_FILE_PATH", g_GPSGlobalSettingParam.LOG_FILE_PATH);
		err = ::vDUT_AddStringParameter(g_GPS_Dut, "CONFIG_TEMPLATE_XML", g_GPSGlobalSettingParam.CONFIG_TEMPLATE_XML);
		// [Jarir Fadlullah] End: For Brcm 2076 GPS DUT
		//////////////////////////////////////////

		err = vDUT_Run(g_GPS_Dut, "RX_GET_MEASUREMENTS");
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
		else
		{
			vDutActived = true;
		}

		err = ::vDUT_GetIntegerReturn( g_GPS_Dut, "SATELLITE_NUMBER_1", &l_MMReturn.SATELLITE_NUMBER_1);
		err = ::vDUT_GetIntegerReturn( g_GPS_Dut, "SATELLITE_NUMBER_2", &l_MMReturn.SATELLITE_NUMBER_2);
		err = ::vDUT_GetIntegerReturn( g_GPS_Dut, "SATELLITE_NUMBER_3", &l_MMReturn.SATELLITE_NUMBER_3);
		err = ::vDUT_GetIntegerReturn( g_GPS_Dut, "SATELLITE_NUMBER_4", &l_MMReturn.SATELLITE_NUMBER_4);
		err = ::vDUT_GetIntegerReturn( g_GPS_Dut, "SATELLITE_NUMBER_5", &l_MMReturn.SATELLITE_NUMBER_5);
		err = ::vDUT_GetIntegerReturn( g_GPS_Dut, "SATELLITE_NUMBER_6", &l_MMReturn.SATELLITE_NUMBER_6);

		// [Jarir Fadlullah] Start: Added For Brcm 2076 GPS DUT
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_1_AVG", &l_MMReturn.CN_BB_1_AVG);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_2_AVG", &l_MMReturn.CN_BB_2_AVG);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_3_AVG", &l_MMReturn.CN_BB_3_AVG);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_4_AVG", &l_MMReturn.CN_BB_4_AVG);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_5_AVG", &l_MMReturn.CN_BB_5_AVG);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_6_AVG", &l_MMReturn.CN_BB_6_AVG);

		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_1_MIN", &l_MMReturn.CN_BB_1_MIN);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_2_MIN", &l_MMReturn.CN_BB_2_MIN);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_3_MIN", &l_MMReturn.CN_BB_3_MIN);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_4_MIN", &l_MMReturn.CN_BB_4_MIN);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_5_MIN", &l_MMReturn.CN_BB_5_MIN);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_6_MIN", &l_MMReturn.CN_BB_6_MIN);

		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_1_MAX", &l_MMReturn.CN_BB_1_MAX);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_2_MAX", &l_MMReturn.CN_BB_2_MAX);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_3_MAX", &l_MMReturn.CN_BB_3_MAX);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_4_MAX", &l_MMReturn.CN_BB_4_MAX);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_5_MAX", &l_MMReturn.CN_BB_5_MAX);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_BB_6_MAX", &l_MMReturn.CN_BB_6_MAX);
		// [Jarir Fadlullah] End: Added For Brcm 2076 GPS DUT

		// [Jarir Fadlullah] Start: Added For Brcm 2076 GPS DUT
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_1_AVG", &l_MMReturn.SIGNAL_STRENGTH_1_AVG);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_2_AVG", &l_MMReturn.SIGNAL_STRENGTH_2_AVG);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_3_AVG", &l_MMReturn.SIGNAL_STRENGTH_3_AVG);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_4_AVG", &l_MMReturn.SIGNAL_STRENGTH_4_AVG);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_5_AVG", &l_MMReturn.SIGNAL_STRENGTH_5_AVG);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_6_AVG", &l_MMReturn.SIGNAL_STRENGTH_6_AVG);

		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_1_MIN", &l_MMReturn.SIGNAL_STRENGTH_1_MIN);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_2_MIN", &l_MMReturn.SIGNAL_STRENGTH_2_MIN);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_3_MIN", &l_MMReturn.SIGNAL_STRENGTH_3_MIN);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_4_MIN", &l_MMReturn.SIGNAL_STRENGTH_4_MIN);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_5_MIN", &l_MMReturn.SIGNAL_STRENGTH_5_MIN);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_6_MIN", &l_MMReturn.SIGNAL_STRENGTH_6_MIN);

		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_1_MAX", &l_MMReturn.SIGNAL_STRENGTH_1_MAX);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_2_MAX", &l_MMReturn.SIGNAL_STRENGTH_2_MAX);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_3_MAX", &l_MMReturn.SIGNAL_STRENGTH_3_MAX);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_4_MAX", &l_MMReturn.SIGNAL_STRENGTH_4_MAX);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_5_MAX", &l_MMReturn.SIGNAL_STRENGTH_5_MAX);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "SIGNAL_STRENGTH_6_MAX", &l_MMReturn.SIGNAL_STRENGTH_6_MAX);
		// [Jarir Fadlullah] End: Added For Brcm 2076 GPS DUT

		// [Jarir Fadlullah] Start: Modified CN_# to CN_#_AVG
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_1_AVG", &l_MMReturn.CN_1_AVG);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_2_AVG", &l_MMReturn.CN_2_AVG);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_3_AVG", &l_MMReturn.CN_3_AVG);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_4_AVG", &l_MMReturn.CN_4_AVG);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_5_AVG", &l_MMReturn.CN_5_AVG);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_6_AVG", &l_MMReturn.CN_6_AVG);
		// [Jarir Fadlullah] End: Modified CN_# to CN_#_AVG

		// [Jarir Fadlullah] Start: Added for Brcm 2076 GPS DUT
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_1_MIN", &l_MMReturn.CN_1_MIN);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_2_MIN", &l_MMReturn.CN_2_MIN);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_3_MIN", &l_MMReturn.CN_3_MIN);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_4_MIN", &l_MMReturn.CN_4_MIN);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_5_MIN", &l_MMReturn.CN_5_MIN);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_6_MIN", &l_MMReturn.CN_6_MIN);

		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_1_MAX", &l_MMReturn.CN_1_MAX);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_2_MAX", &l_MMReturn.CN_2_MAX);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_3_MAX", &l_MMReturn.CN_3_MAX);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_4_MAX", &l_MMReturn.CN_4_MAX);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_5_MAX", &l_MMReturn.CN_5_MAX);
		err = ::vDUT_GetDoubleReturn( g_GPS_Dut, "CN_6_MAX", &l_MMReturn.CN_6_MAX);

		double temp[MAX_BUFFER_SIZE];

		err = ::vDUT_GetArrayDoubleReturn(g_GPS_Dut, "CN_1_ARRAY", temp, MAX_BUFFER_SIZE);
		for (int temp1 = 0; temp1<MAX_BUFFER_SIZE; temp1++)
			//for (int temp1 = 0; temp1<MAX_BUFFER_SIZE; temp1++)
		{
			if (temp[temp1]!=NULL)
				l_MMReturn.CN_1_ARRAY.push_back(temp[temp1]);
			else
				break;
		}
		//err = ::vDUT_GetStringReturn(g_GPS_Dut, "ERROR_MESSAGE", temp, MAX_BUFFER_SIZE);
		// [Jarir Fadlullah] End: Added for Brcm 2076 GPS DUT

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
			sprintf_s(l_MMReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_MMReturnMap);
		}
		else{}
#pragma endregion

	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_MMReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		ReturnErrorMessage(l_MMReturn.ERROR_MESSAGE, "[GPS] Unknown Error!\n");
		err = -1;
	}

	return err;
}



void InitializeGPSModulatedModeContainers(void)
{
	GPS_SETTING_STRUCT setting;

#pragma region Step 1 : Setup & Init InputParm
	/*-------------------*
	 * Input Parameters: *
	 * IQTESTER_IP01     *
	 *-------------------*/

	l_MMParamMap.clear();

	/* input:SATELLITE_NUMBER  */
	l_MMParam.SATELLITE_NUMBER_1 = 1;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_MMParam.SATELLITE_NUMBER_1))    // Type_Checking
	{
		setting.value = (void*)&l_MMParam.SATELLITE_NUMBER_1;
		setting.unit        = "";
		setting.helpText    = "Satellite Number (value:1~37).\r\n";
		l_MMParamMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_1", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	/* input:POWER  */
	l_MMParam.POWER_1 = -130.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMParam.POWER_1))    // Type_Checking
	{
		setting.value = (void*)&l_MMParam.POWER_1;
		setting.unit        = "dBm";
		setting.helpText    = "Power value of the GPS received.\n(-145~-60dBm) (default : -130dBm).\r\n";
		l_MMParamMap.insert( pair<string,GPS_SETTING_STRUCT>("POWER_1", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* input:SATELLITE_NUMBER  */
	l_MMParam.SATELLITE_NUMBER_2 = 0;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_MMParam.SATELLITE_NUMBER_2))    // Type_Checking
	{
		setting.value = (void*)&l_MMParam.SATELLITE_NUMBER_2;
		setting.unit        = "";
		setting.helpText    = "Satellite Number (value:1~37).\r\n";
		l_MMParamMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_2", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	/* input:POWER  */
	l_MMParam.POWER_2 = -130.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMParam.POWER_2))    // Type_Checking
	{
		setting.value = (void*)&l_MMParam.POWER_2;
		setting.unit        = "dBm";
		setting.helpText    = "Power value of the GPS received.\n(-145~-60dBm).\r\n";
		l_MMParamMap.insert( pair<string,GPS_SETTING_STRUCT>("POWER_2", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* input:SATELLITE_NUMBER  */
	l_MMParam.SATELLITE_NUMBER_3 = 0;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_MMParam.SATELLITE_NUMBER_3))    // Type_Checking
	{
		setting.value = (void*)&l_MMParam.SATELLITE_NUMBER_3;
		setting.unit        = "";
		setting.helpText    = "Satellite Number (value:1~37).\r\n";
		l_MMParamMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_3", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	/* input:POWER  */
	l_MMParam.POWER_3 = -130.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMParam.POWER_3))    // Type_Checking
	{
		setting.value = (void*)&l_MMParam.POWER_3;
		setting.unit        = "dBm";
		setting.helpText    = "Power value of the GPS received.\n(-145~-60dBm).\r\n";
		l_MMParamMap.insert( pair<string,GPS_SETTING_STRUCT>("POWER_3", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* input:SATELLITE_NUMBER  */
	l_MMParam.SATELLITE_NUMBER_4 = 0;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_MMParam.SATELLITE_NUMBER_4))    // Type_Checking
	{
		setting.value = (void*)&l_MMParam.SATELLITE_NUMBER_4;
		setting.unit        = "";
		setting.helpText    = "Satellite Number (value:1~37).\r\n";
		l_MMParamMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_4", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	/* input:POWER  */
	l_MMParam.POWER_4 = -130.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMParam.POWER_4))    // Type_Checking
	{
		setting.value = (void*)&l_MMParam.POWER_4;
		setting.unit        = "dBm";
		setting.helpText    = "Power value of the GPS received.\n(-145~-60dBm).\r\n";
		l_MMParamMap.insert( pair<string,GPS_SETTING_STRUCT>("POWER_4", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* input:SATELLITE_NUMBER  */
	l_MMParam.SATELLITE_NUMBER_5 = 0;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_MMParam.SATELLITE_NUMBER_5))    // Type_Checking
	{
		setting.value = (void*)&l_MMParam.SATELLITE_NUMBER_5;
		setting.unit        = "";
		setting.helpText    = "Satellite Number (value:1~37).\r\n";
		l_MMParamMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_5", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	/* input:POWER  */
	l_MMParam.POWER_5 = -130.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMParam.POWER_5))    // Type_Checking
	{
		setting.value = (void*)&l_MMParam.POWER_5;
		setting.unit        = "dBm";
		setting.helpText    = "Power value of the GPS received.\n(-145~-60dBm).\r\n";
		l_MMParamMap.insert( pair<string,GPS_SETTING_STRUCT>("POWER_5", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* input:SATELLITE_NUMBER  */
	l_MMParam.SATELLITE_NUMBER_6 = 0;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_MMParam.SATELLITE_NUMBER_6))    // Type_Checking
	{
		setting.value = (void*)&l_MMParam.SATELLITE_NUMBER_6;
		setting.unit        = "";
		setting.helpText    = "Satellite Number (value:1~37).\r\n";
		l_MMParamMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_6", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	/* input:POWER  */
	l_MMParam.POWER_6 = -130.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMParam.POWER_6))    // Type_Checking
	{
		setting.value = (void*)&l_MMParam.POWER_6;
		setting.unit        = "dBm";
		setting.helpText    = "Power value of the GPS received.\n(-145~-60dBm).\r\n";
		l_MMParamMap.insert( pair<string,GPS_SETTING_STRUCT>("POWER_6", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* input:Timeout */
	l_MMParam.TIMEOUT = 20;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_MMParam.TIMEOUT))    // Type_Checking
	{
		setting.value = (void*)&l_MMParam.TIMEOUT;
		setting.unit        = "sec";
		setting.helpText    = "Timeout : transmit signal till timeout (default 20 second).\r\n";
		l_MMParamMap.insert( pair<string,GPS_SETTING_STRUCT>("TIMEOUT", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* input:CABLE_LOSS_DB */
	l_MMParam.CABLE_LOSS_DB = 0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMParam.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value = (void*)&l_MMParam.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cableloss between transmiter & receiver (default : 0dB).\r\n";
		l_MMParamMap.insert( pair<string,GPS_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/* input:MODULATED_MODE_NAV_DATA */
	l_MMParam.MODULATED_MODE_NAV_DATA = 1; // NAV_OFF mode : 0 ; NAV_ON mode : 1
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_MMParam.MODULATED_MODE_NAV_DATA))    // Type_Checking
	{
		setting.value = (void*)&l_MMParam.MODULATED_MODE_NAV_DATA;
		setting.unit        = "";
		setting.helpText    = "Navigation info on/off, this might make the receiver get trouble with Navigation Data. (default : 1(on)).\r\n";
		l_MMParamMap.insert( pair<string,GPS_SETTING_STRUCT>("MODULATED_MODE_NAV_DATA", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	// [Jarir Fadlullah] Start: Added for Brcm 2076 GPS
	l_MMParam.ANTENNA = 0; // 0: GL_RF_BRCM_2076, 1: GL_RF_BRCM_2076_EXT_LNA
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_MMParam.ANTENNA))    // Type_Checking
	{
		setting.value = (void*)&l_MMParam.ANTENNA;
		setting.unit        = "";
		setting.helpText    = "Choose INT LNA or EXT LNA for BRCM 2076 GPS DUT. (0: GL_RF_2076_BRCM (default INT LNA), 1: GL_RF_2076_BRCM_EXT_LNA).\r\n";
		l_MMParamMap.insert( pair<string,GPS_SETTING_STRUCT>("ANTENNA", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	// [Jarir Fadlullah] End: Added for Brcm 2076 GPS
	///////////////////////////////////////////////////


#pragma endregion

#pragma region Step 2 : Setup & Init ReturnValue
	/*----------------*
	 * Return Values: *
	 * ERROR_MESSAGE  *
	 *----------------*/

	/* return:SATELLITE_NUMBER  */
	l_MMReturn.SATELLITE_NUMBER_1 = 0;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_MMReturn.SATELLITE_NUMBER_1))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SATELLITE_NUMBER_1;
		setting.unit        = "";
		setting.helpText    = "Satellite Number.\r\n";
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_1", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// [Jarir Fadlullah] Start: Added CN ratio array return
	l_MMReturn.CN_1_ARRAY.clear();
	setting.type = GPS_SETTING_TYPE_ARRAY_DOUBLE;
	if (0 == l_MMReturn.CN_1_ARRAY.size())    // Type_Checking
	{
		setting.value = (void*) &l_MMReturn.CN_1_ARRAY;
		setting.unit        = "dB-Hz";
		setting.helpText    = "C/N values at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		//setting.helpText    = "C/N value of the satellite.\r\n";
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_1_ARRAY", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	// [Jarir Fadlullah] End: Added CN ratio array return

	// [Jarir Fadlullah] Start: Modified CN_# to CN_#_AVG
	/* return:CN  */
	l_MMReturn.CN_1_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_1_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_1_AVG;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Average C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_1_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	// [Jarir Fadlullah] End: Modified CN_# to CN_#_AVG

	// [Jarir Fadlullah] Start: Added for Brcm 2076 GPS DUT
	l_MMReturn.CN_1_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_1_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_1_MAX;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Max C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_1_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_1_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_1_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_1_MIN;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Min C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_1_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.SIGNAL_STRENGTH_1_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_1_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_1_AVG;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Average Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_1_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.SIGNAL_STRENGTH_1_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_1_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_1_MAX;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Max Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_1_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	l_MMReturn.SIGNAL_STRENGTH_1_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_1_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_1_MIN;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Min Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_1_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_1_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_1_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_1_AVG;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Average C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_1_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_1_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_1_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_1_MIN;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Min C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_1_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_1_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_1_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_1_MAX;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Max C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_1_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// [Jarir Fadlullah] End: Added for Brcm 2076 GPS DUT

	/* return:SATELLITE_NUMBER  */
	l_MMReturn.SATELLITE_NUMBER_2 = 0;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_MMReturn.SATELLITE_NUMBER_2))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SATELLITE_NUMBER_2;
		setting.unit        = "";
		setting.helpText    = "Satellite Number.\r\n";
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_2", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// [Jarir Fadlullah] Start: Modified CN_# to CN_#_AVG
	/* return:CN  */
	l_MMReturn.CN_2_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_2_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_2_AVG;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Average C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_2_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	// [Jarir Fadlullah] End: Modified CN_# to CN_#_AVG

	// [Jarir Fadlullah] Start: Added for Brcm 2076 GPS DUT
	l_MMReturn.CN_2_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_2_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_2_MAX;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Max C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_2_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_2_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_2_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_2_MIN;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Min C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_2_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.SIGNAL_STRENGTH_2_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_2_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_2_AVG;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Average Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_2_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.SIGNAL_STRENGTH_2_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_2_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_2_MAX;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Max Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_2_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	l_MMReturn.SIGNAL_STRENGTH_2_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_2_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_2_MIN;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Min Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_2_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_2_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_2_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_2_AVG;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Average C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_2_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_2_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_2_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_2_MIN;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Min C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_2_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_2_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_2_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_2_MAX;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Max C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_2_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// [Jarir Fadlullah] End: Added for Brcm 2076 GPS DUT


	/* return:SATELLITE_NUMBER  */
	l_MMReturn.SATELLITE_NUMBER_3 = 0;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_MMReturn.SATELLITE_NUMBER_3))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SATELLITE_NUMBER_3;
		setting.unit        = "";
		setting.helpText    = "Satellite Number.\r\n";
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_3", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// [Jarir Fadlullah] Start: Modified CN_# to CN_#_AVG
	/* return:CN  */
	l_MMReturn.CN_3_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_3_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_3_AVG;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Average C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_3_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	// [Jarir Fadlullah] End: Modified CN_# to CN_#_AVG

	// [Jarir Fadlullah] Start: Added for Brcm 2076 GPS DUT
	l_MMReturn.CN_3_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_3_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_3_MAX;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Max C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_3_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_3_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_3_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_3_MIN;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Min C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_3_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.SIGNAL_STRENGTH_3_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_3_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_3_AVG;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Average Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_3_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.SIGNAL_STRENGTH_3_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_3_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_3_MAX;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Max Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_3_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	l_MMReturn.SIGNAL_STRENGTH_3_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_3_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_3_MIN;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Min Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_3_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_3_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_3_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_3_AVG;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Average C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_3_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_3_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_3_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_3_MIN;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Min C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_3_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_3_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_3_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_3_MAX;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Max C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_3_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// [Jarir Fadlullah] End: Added for Brcm 2076 GPS DUT


	/* return:SATELLITE_NUMBER  */
	l_MMReturn.SATELLITE_NUMBER_4 = 0;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_MMReturn.SATELLITE_NUMBER_4))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SATELLITE_NUMBER_4;
		setting.unit        = "";
		setting.helpText    = "Satellite Number.\r\n";
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_4", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// [Jarir Fadlullah] Start: Modified CN_# to CN_#_AVG
	/* return:CN  */
	l_MMReturn.CN_4_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_4_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_4_AVG;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Average C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_4_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	// [Jarir Fadlullah] End: Modified CN_# to CN_#_AVG

	// [Jarir Fadlullah] Start: Added for Brcm 2076 GPS DUT
	l_MMReturn.CN_4_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_4_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_4_MAX;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Max C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_4_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_4_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_4_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_4_MIN;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Min C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_4_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.SIGNAL_STRENGTH_4_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_4_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_4_AVG;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Average Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_4_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.SIGNAL_STRENGTH_4_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_4_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_4_MAX;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Max Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_4_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	l_MMReturn.SIGNAL_STRENGTH_4_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_4_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_4_MIN;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Min Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_4_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_4_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_4_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_4_AVG;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Average C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_4_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_4_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_4_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_4_MIN;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Min C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_4_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_4_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_4_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_4_MAX;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Max C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_4_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// [Jarir Fadlullah] End: Added for Brcm 2076 GPS DUT


	/* return:SATELLITE_NUMBER  */
	l_MMReturn.SATELLITE_NUMBER_5 = 0;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_MMReturn.SATELLITE_NUMBER_5))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SATELLITE_NUMBER_5;
		setting.unit        = "";
		setting.helpText    = "Satellite Number.\r\n";
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_5", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// [Jarir Fadlullah] Start: Modified CN_# to CN_#_AVG
	/* return:CN  */
	l_MMReturn.CN_5_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_5_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_5_AVG;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Average C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_5_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	// [Jarir Fadlullah] End: Modified CN_# to CN_#_AVG

	// [Jarir Fadlullah] Start: Added for Brcm 2076 GPS DUT
	l_MMReturn.CN_5_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_5_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_5_MAX;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Max C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_5_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_5_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_5_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_5_MIN;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Min C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_5_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.SIGNAL_STRENGTH_5_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_5_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_5_AVG;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Average Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_5_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.SIGNAL_STRENGTH_5_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_5_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_5_MAX;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Max Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_5_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	l_MMReturn.SIGNAL_STRENGTH_5_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_5_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_5_MIN;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Min Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_5_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_5_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_5_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_5_AVG;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Average C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_5_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_5_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_5_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_5_MIN;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Min C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_5_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_5_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_5_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_5_MAX;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Max C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_5_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// [Jarir Fadlullah] End: Added for Brcm 2076 GPS DUT


	/* return:SATELLITE_NUMBER  */
	l_MMReturn.SATELLITE_NUMBER_6 = 0;
	setting.type = GPS_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_MMReturn.SATELLITE_NUMBER_6))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SATELLITE_NUMBER_6;
		setting.unit        = "";
		setting.helpText    = "Satellite Number.\r\n";
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SATELLITE_NUMBER_6", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// [Jarir Fadlullah] Start: Modified CN_# to CN_#_AVG
	/* return:CN  */
	l_MMReturn.CN_6_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_6_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_6_AVG;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Average C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_6_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	// [Jarir Fadlullah] End: Modified CN_# to CN_#_AVG

	// [Jarir Fadlullah] Start: Added for Brcm 2076 GPS DUT
	l_MMReturn.CN_6_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_6_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_6_MAX;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Max C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_6_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_6_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_6_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_6_MIN;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Min C/N ratio at antenna for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_6_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.SIGNAL_STRENGTH_6_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_6_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_6_AVG;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Average Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_6_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.SIGNAL_STRENGTH_6_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_6_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_6_MAX;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Max Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_6_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	l_MMReturn.SIGNAL_STRENGTH_6_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.SIGNAL_STRENGTH_6_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.SIGNAL_STRENGTH_6_MIN;
		setting.unit        = "dBm";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Received Min Signal Strength for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("SIGNAL_STRENGTH_6_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_6_AVG = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_6_AVG))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_6_AVG;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Average C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_6_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_6_MIN = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_6_MIN))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_6_MIN;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Min C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_6_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_MMReturn.CN_BB_6_MAX = 0.0;
	setting.type = GPS_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_MMReturn.CN_BB_6_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_MMReturn.CN_BB_6_MAX;
		setting.unit        = "dB-Hz";
		//setting.helpText    = "C/N value of the satellite.\r\n";
		setting.helpText    = "Max C/N ratio at Baseband for the satellite.\r\n"; // [Jarir Fadlullah] Changed for Brcm 2076
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("CN_BB_6_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// [Jarir Fadlullah] End: Added for Brcm 2076 GPS DUT



	/* return:error_message  */
	strcpy_s(l_MMReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "");
	setting.type = GPS_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_MMReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value = (void*)l_MMReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "";
		l_MMReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

#pragma endregion
}


