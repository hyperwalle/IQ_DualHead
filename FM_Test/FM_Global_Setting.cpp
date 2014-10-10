#include "stdafx.h"
#include "TestManager.h"
#include "FM_Test.h"
#include "FM_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

// This variable is declared in FM_Test_Internal.cpp
extern vDUT_ID      g_FM_Dut;
extern TM_ID        g_FM_Test_ID;

using namespace std;

// Input Parameter Container
map<string, FM_SETTING_STRUCT> g_FMGlobalSettingParamMap;

// Return Value Container 
map<string, FM_SETTING_STRUCT> l_FMGlobalSettingReturnMap;

FM_GLOBAL_SETTING g_FMGlobalSettingParam;

struct tagReturn
{
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} g_FMGlobalSettingReturn;

void ClearGlobalSettingReturn(void)
{
	g_FMGlobalSettingParamMap.clear();
	l_FMGlobalSettingReturnMap.clear();
}

FM_TEST_API int FM_Global_Setting(void)
{
    int		err = ERR_OK;
    int		dummyValue = 0;
	char	logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_FMGlobalSettingReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_FM_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(g_FMGlobalSettingParamMap);
        return err;
    }
	else
	{
		// do nothing
	}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_FM_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_FMGlobalSettingReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
	   /*---------------------------------------*
		* g_FM_Test_ID need to be valid (>=0) *
		*---------------------------------------*/
		TM_ClearReturns(g_FM_Test_ID);
		if( g_FM_Test_ID<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] FM_Test_ID not valid.\n");
			throw logMessage;
		}
		else
		{
			// do nothing
		}

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		GetInputParameters(g_FMGlobalSettingParamMap);
		err = ERR_OK;	// Global Setting are optinal parameters, thus err always = ERR_OK

		//Set minimum values
		if(g_FMGlobalSettingParam.RX_AUDIO_AVERAGE < 1)
			g_FMGlobalSettingParam.RX_AUDIO_AVERAGE = 1;

		if(g_FMGlobalSettingParam.TX_AUDIO_AVERAGE < 1)
			g_FMGlobalSettingParam.TX_AUDIO_AVERAGE = 1;

		if(g_FMGlobalSettingParam.TX_RF_MEASUREMENTS_AVERAGE < 1)
			g_FMGlobalSettingParam.TX_RF_MEASUREMENTS_AVERAGE = 1;



	    
		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(g_FMGlobalSettingReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_FMGlobalSettingReturnMap);
		}
		else
		{
			// do nothing
		}
	}
    catch(char *msg)
    {
        ReturnErrorMessage(g_FMGlobalSettingReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(g_FMGlobalSettingReturn.ERROR_MESSAGE, "[FM] Unknown Error!\n");
		err = -1;
    }

    return err;
}

void InitializeGlobalSettingContainers(void)
{
    /*------------------*
     * Input Paramters: *
     *------------------*/
    g_FMGlobalSettingParamMap.clear();

    FM_SETTING_STRUCT setting;

	setting.type = FM_SETTING_TYPE_INTEGER;
	g_FMGlobalSettingParam.TX_RF_RESOLUTION_BANDWIDTH_HZ = 200;
    if (sizeof(int)==sizeof(g_FMGlobalSettingParam.TX_RF_RESOLUTION_BANDWIDTH_HZ))    // Type_Checking
    {
        setting.value = (void*)&g_FMGlobalSettingParam.TX_RF_RESOLUTION_BANDWIDTH_HZ;
        setting.unit  = "Hz";
		setting.helpText = "RF Resolution bandwidth in Hertz. MIN:100 Hz, MAX:100,000 Hz ";
        g_FMGlobalSettingParamMap.insert( pair<string, FM_SETTING_STRUCT>("TX_RF_RESOLUTION_BANDWIDTH_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = FM_SETTING_TYPE_INTEGER;
	g_FMGlobalSettingParam.TX_OCCUPIED_BANDWIDTH_PERCENT = 99;
    if (sizeof(int)==sizeof(g_FMGlobalSettingParam.TX_OCCUPIED_BANDWIDTH_PERCENT))    // Type_Checking
    {
        setting.value = (void*)&g_FMGlobalSettingParam.TX_OCCUPIED_BANDWIDTH_PERCENT;
        setting.unit  = "%";
		setting.helpText = "Occupied bandwidth in Percent for Power measurement.";
        g_FMGlobalSettingParamMap.insert( pair<string, FM_SETTING_STRUCT>("TX_OCCUPIED_BANDWIDTH_PERCENT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = FM_SETTING_TYPE_INTEGER;
	g_FMGlobalSettingParam.TX_RF_POWER_MEASUREMENT_BANDWIDTH_HZ = 200000;
    if (sizeof(int)==sizeof(g_FMGlobalSettingParam.TX_RF_POWER_MEASUREMENT_BANDWIDTH_HZ))    // Type_Checking
    {
        setting.value = (void*)&g_FMGlobalSettingParam.TX_RF_POWER_MEASUREMENT_BANDWIDTH_HZ;
        setting.unit  = "Hz";
		setting.helpText = "RF Power Measurement bandwidth in Hertz. MIN:50 Hz, MAX:1000 Hz ";
        g_FMGlobalSettingParamMap.insert( pair<string, FM_SETTING_STRUCT>("TX_RF_POWER_MEASUREMENT_BANDWIDTH_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = FM_SETTING_TYPE_INTEGER;
	g_FMGlobalSettingParam.AUDIO_SAMPLING_RATE_HZ = 48000;
    if (sizeof(int)==sizeof(g_FMGlobalSettingParam.AUDIO_SAMPLING_RATE_HZ))    // Type_Checking
    {
        setting.value = (void*)&g_FMGlobalSettingParam.AUDIO_SAMPLING_RATE_HZ;
        setting.unit  = "Hz";
        setting.helpText = "Audio Sampling Rate in Hertz";
        g_FMGlobalSettingParamMap.insert( pair<string, FM_SETTING_STRUCT>("AUDIO_SAMPLING_RATE_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = FM_SETTING_TYPE_INTEGER;
	g_FMGlobalSettingParam.AUDIO_RESOLUTION_BANDWIDTH_HZ = 50;
    if (sizeof(int)==sizeof(g_FMGlobalSettingParam.AUDIO_RESOLUTION_BANDWIDTH_HZ))    // Type_Checking
    {
        setting.value = (void*)&g_FMGlobalSettingParam.AUDIO_RESOLUTION_BANDWIDTH_HZ;
        setting.unit  = "Hz";
		setting.helpText = "Audio Resolution bandwidth in Hertz. MIN:50 Hz, MAX:1000 Hz ";
        g_FMGlobalSettingParamMap.insert( pair<string, FM_SETTING_STRUCT>("AUDIO_RESOLUTION_BANDWIDTH_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = FM_SETTING_TYPE_INTEGER;
	g_FMGlobalSettingParam.TX_RF_MEASUREMENTS_AVERAGE = 1;
    if (sizeof(int)==sizeof(g_FMGlobalSettingParam.TX_RF_MEASUREMENTS_AVERAGE))    // Type_Checking
    {
        setting.value = (void*)&g_FMGlobalSettingParam.TX_RF_MEASUREMENTS_AVERAGE;
        setting.unit  = "times";
        setting.helpText = "Required minumum number of TX_RF_MEASUREMENTS capture and measurements.";
        g_FMGlobalSettingParamMap.insert( pair<string, FM_SETTING_STRUCT>("TX_RF_MEASUREMENTS_AVERAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = FM_SETTING_TYPE_INTEGER;
	g_FMGlobalSettingParam.TX_AUDIO_AVERAGE = 1;
    if (sizeof(int)==sizeof(g_FMGlobalSettingParam.TX_AUDIO_AVERAGE))    // Type_Checking
    {
        setting.value = (void*)&g_FMGlobalSettingParam.TX_AUDIO_AVERAGE;
        setting.unit  = "times";
        setting.helpText = "Required minumum number of TX AUDIO capture and measurements.";
        g_FMGlobalSettingParamMap.insert( pair<string, FM_SETTING_STRUCT>("TX_AUDIO_AVERAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = FM_SETTING_TYPE_INTEGER;
	g_FMGlobalSettingParam.RX_AUDIO_AVERAGE = 1;
    if (sizeof(int)==sizeof(g_FMGlobalSettingParam.RX_AUDIO_AVERAGE))    // Type_Checking
    {
        setting.value = (void*)&g_FMGlobalSettingParam.RX_AUDIO_AVERAGE;
        setting.unit  = "times";
        setting.helpText = "Required minumum number of RX AUDIO capture and measurements.";
        g_FMGlobalSettingParamMap.insert( pair<string, FM_SETTING_STRUCT>("RX_AUDIO_AVERAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = FM_SETTING_TYPE_DOUBLE;
	g_FMGlobalSettingParam.VSG_MAX_POWER_FM = -40;
	if (sizeof(double)==sizeof(g_FMGlobalSettingParam.VSG_MAX_POWER_FM))    // Type_Checking
	{
		setting.value = (void*)&g_FMGlobalSettingParam.VSG_MAX_POWER_FM;
		setting.unit  = "dBm";
		setting.helpText  = "VSG RF Max output power limit for FM signal \r\nDefault value is -40 dBm";
		g_FMGlobalSettingParamMap.insert( pair<string, FM_SETTING_STRUCT>("VSG_MAX_POWER_FM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = FM_SETTING_TYPE_INTEGER;
	g_FMGlobalSettingParam.DUT_TX_SETTLE_TIME_MS = 0;
	if (sizeof(int)==sizeof(g_FMGlobalSettingParam.DUT_TX_SETTLE_TIME_MS))    // Type_Checking
	{
		setting.value = (void*)&g_FMGlobalSettingParam.DUT_TX_SETTLE_TIME_MS;
		setting.unit  = "ms";
		setting.helpText  = "minimum Time in ms for DUT TX to Settle";
		g_FMGlobalSettingParamMap.insert( pair<string, FM_SETTING_STRUCT>("DUT_TX_SETTLE_TIME_MS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = FM_SETTING_TYPE_INTEGER;
	g_FMGlobalSettingParam.DUT_RX_SETTLE_TIME_MS = 0;
	if (sizeof(int)==sizeof(g_FMGlobalSettingParam.DUT_RX_SETTLE_TIME_MS))    // Type_Checking
	{
		setting.value = (void*)&g_FMGlobalSettingParam.DUT_RX_SETTLE_TIME_MS;
		setting.unit  = "ms";
		setting.helpText  = "minimum Time in ms to wit for DUT RX to Settle";
		g_FMGlobalSettingParamMap.insert( pair<string, FM_SETTING_STRUCT>("DUT_RX_SETTLE_TIME_MS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = FM_SETTING_TYPE_DOUBLE;
	g_FMGlobalSettingParam.DUT_PEAK_VOLTAGE_LEVEL_VOLTS = 0.5;
	if (sizeof(double)==sizeof(g_FMGlobalSettingParam.DUT_PEAK_VOLTAGE_LEVEL_VOLTS))    // Type_Checking
	{
		setting.value = (void*)&g_FMGlobalSettingParam.DUT_PEAK_VOLTAGE_LEVEL_VOLTS;
		setting.unit  = "Volts";
		setting.helpText  = "Maximum Peak Voltage(Vp) the DUT Tx needs to produce the peak deviation specified";
		g_FMGlobalSettingParamMap.insert( pair<string, FM_SETTING_STRUCT>("DUT_PEAK_VOLTAGE_LEVEL_VOLTS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = FM_SETTING_TYPE_INTEGER;
	g_FMGlobalSettingParam.retryTestItem = 0;
	if (sizeof(int)==sizeof(g_FMGlobalSettingParam.retryTestItem))    // Type_Checking
	{
		setting.value = (void*)&g_FMGlobalSettingParam.retryTestItem;
		setting.unit  = "";
		setting.helpText = "Specify no. of retry on test item if results are failed.";
		g_FMGlobalSettingParamMap.insert( pair<string, FM_SETTING_STRUCT>("RETRY_TEST_ITEM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = FM_SETTING_TYPE_INTEGER;
	g_FMGlobalSettingParam.retryTestItemDuringRetry = 0;
	if (sizeof(int)==sizeof(g_FMGlobalSettingParam.retryTestItemDuringRetry))    // Type_Checking
	{
		setting.value = (void*)&g_FMGlobalSettingParam.retryTestItemDuringRetry;
		setting.unit  = "";
		setting.helpText = "Specify whether a DUT reset function will be called when a retry occurs,\ncalls the \"RESET_TEST_ITEM\" test function.";
		g_FMGlobalSettingParamMap.insert( pair<string, FM_SETTING_STRUCT>("RESET_TEST_ITEM_DURING_RETRY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = FM_SETTING_TYPE_INTEGER;
	g_FMGlobalSettingParam.retryErrorItems = 0;
	if (sizeof(int)==sizeof(g_FMGlobalSettingParam.retryErrorItems))    // Type_Checking
	{
		setting.value = (void*)&g_FMGlobalSettingParam.retryErrorItems;
		setting.unit  = "";
		setting.helpText = "Specify whether a retry an item if an error is returned,\nthis will not affect whether an item is retried for a limit failure";
		g_FMGlobalSettingParamMap.insert( pair<string, FM_SETTING_STRUCT>("RETRY_ERROR_ITEMS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    setting.type = FM_SETTING_TYPE_STRING;
    strcpy_s(g_FMGlobalSettingParam.FM_AUDIO_INTERFACE, MAX_BUFFER_SIZE, "ANALOG");
    if (MAX_BUFFER_SIZE==sizeof(g_FMGlobalSettingParam.FM_AUDIO_INTERFACE))    // Type_Checking
    {
        setting.value = (void*)g_FMGlobalSettingParam.FM_AUDIO_INTERFACE;
        setting.unit  = "";
        setting.helpText = "Specifies the Audio Interface to the FM Audio ANALOG/DIGITAL";
        g_FMGlobalSettingParamMap.insert( pair<string, FM_SETTING_STRUCT>("FM_AUDIO_INTERFACE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = FM_SETTING_TYPE_STRING;
    strcpy_s(g_FMGlobalSettingParam.I2S_TYPE, MAX_BUFFER_SIZE, "SLAVE");
    if (MAX_BUFFER_SIZE==sizeof(g_FMGlobalSettingParam.I2S_TYPE))    // Type_Checking
    {
        setting.value = (void*)g_FMGlobalSettingParam.I2S_TYPE;
        setting.unit  = "";
        setting.helpText = "Specifies if the I2S interface is MASTER/SLAVE - valid only for Digital Audio Interface";
        g_FMGlobalSettingParamMap.insert( pair<string, FM_SETTING_STRUCT>("I2S_TYPE", setting) );
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
    l_FMGlobalSettingReturnMap.clear();

    g_FMGlobalSettingReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(g_FMGlobalSettingReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)g_FMGlobalSettingReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_FMGlobalSettingReturnMap.insert( pair<string,FM_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return;
}
