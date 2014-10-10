#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_11AC_Test.h"
#include "WiFi_11AC_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

// This variable is declared in WiFi_Test_Internal.cpp
extern vDUT_ID      g_WiFi_11ac_Dut;
extern TM_ID        g_WiFi_11ac_Test_ID;

using namespace std;

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> g_WiFi11ACGlobalSettingParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_WiFi11ACGlobalSettingReturnMap;

WIFI_GLOBAL_SETTING g_WiFi11ACGlobalSettingParam;

struct tagReturn
{
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} g_WiFi11ACGlobalSettingReturn;

#ifndef WIN32 
int init11ACGlobalSettingContainers = Initialize11ACGlobalSettingContainers();
#endif

WIFI_11AC_TEST_API int WIFI_11AC_Global_Setting(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_WiFi11ACGlobalSettingReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(g_WiFi11ACGlobalSettingParamMap);
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
        RespondToQueryReturn(l_WiFi11ACGlobalSettingReturnMap);
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
		GetInputParameters(g_WiFi11ACGlobalSettingParamMap);
		err = ERR_OK;	// Global Setting are optinal parameters, thus err always = ERR_OK
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Get input parameters return OK.\n");
	    
		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(g_WiFi11ACGlobalSettingReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_WiFi11ACGlobalSettingReturnMap);
		}
		else
		{
			// do nothing
		}
	}
    catch(char *msg)
    {
        ReturnErrorMessage(g_WiFi11ACGlobalSettingReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(g_WiFi11ACGlobalSettingReturn.ERROR_MESSAGE, "[WiFi_11AC] Unknown Error!\n");
		err = -1;
    }

    return err;
}

int Initialize11ACGlobalSettingContainers(void)
{
    /*------------------*
     * Input Parameters: *
     *------------------*/
    g_WiFi11ACGlobalSettingParamMap.clear();

    WIFI_SETTING_STRUCT setting;

    //Peak to Average Settings
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11B_1M = 2;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11B_1M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11B_1M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11b 1Mbps.\r\nDefault value is 2dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11B_1M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11B_2M = 2;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11B_2M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11B_2M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11b 2Mbps.\r\nDefault value is 2dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11B_2M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11B_5_5M = 2;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11B_5_5M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11B_5_5M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11b 5.5Mbps.\r\nDefault value is 2dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11B_5_5M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11B_11M = 2;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11B_11M))    // Type_Checking
    {
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11b 11Mbps.\r\nDefault value is 2dB";
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11B_11M;
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11B_11M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_6M = 10;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_6M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_6M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11a/g OFDM-6.\r\nDefault value is 10dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11AG_6M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_9M = 10;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_9M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_9M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11a/g OFDM-9.\r\nDefault value is 10dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11AG_9M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_12M = 10;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_12M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_12M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11a/g OFDM-12.\r\nDefault value is 10dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11AG_12M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_18M = 10;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_18M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_18M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11a/g OFDM-18.\r\nDefault value is 10dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11AG_18M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_24M = 10;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_24M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_24M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11a/g OFDM-24.\r\nDefault value is 10dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11AG_24M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_36M = 10;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_36M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_36M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11a/g OFDM-36.\r\nDefault value is 10dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11AG_36M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_48M = 10;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_48M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_48M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11a/g OFDM-48.\r\nDefault value is 10dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11AG_48M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_54M = 10;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_54M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_54M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11a/g OFDM-54.\r\nDefault value is 10dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11AG_54M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS0 = 10;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS0))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS0;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11n MCS0,MCS8,MCS16,MCS24.\r\nDefault value is 10dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11N_MCS0", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS1 = 10;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS1))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS1;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11n MCS1,MCS9,MCS17,MCS25.\r\nDefault value is 10dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11N_MCS1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS2 = 10;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS2))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS2;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11n MCS2,MCS10,MCS18,MCS26.\r\nDefault value is 10dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11N_MCS2", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS3 = 10;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS3))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS3;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11n MCS3,MCS11,MCS19,MCS27.\r\nDefault value is 10dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11N_MCS3", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS4 = 10;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS4))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS4;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11n MCS4,MCS12,MCS20,MCS28.\r\nDefault value is 10dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11N_MCS4", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS5 = 10;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS5))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS5;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11n MCS5,MCS13,MCS21,MCS29.\r\nDefault value is 10dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11N_MCS5", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS6 = 10;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS6))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS6;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11n MCS6,MCS14,MCS22,MCS30.\r\nDefault value is 10dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11N_MCS6", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS7 = 10;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS7))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11N_MCS7;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11n MCS7,MCS15,MCS23,MCS31.\r\nDefault value is 10dB";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11N_MCS7", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


	// PER test
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PREFIX_11B, MAX_BUFFER_SIZE, "");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PREFIX_11B))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PREFIX_11B;
        setting.unit  = "";
        setting.helpText = "Prefix of 802.11b waveform name. Default is non- value, use default waveform name defined in program.";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_WAVEFORM_PREFIX_11B", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PREFIX_11AG, MAX_BUFFER_SIZE, "");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PREFIX_11AG))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PREFIX_11AG;
        setting.unit  = "";
        setting.helpText = "Prefix of 802.11ag waveform name. Default is non- value, use default waveform name defined in program.";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_WAVEFORM_PREFIX_11AG", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PREFIX_11N, MAX_BUFFER_SIZE, "");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PREFIX_11N))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PREFIX_11N;
        setting.unit  = "";
        setting.helpText = "Prefix of 802.11n waveform name. Default is non- value, use default waveform name defined in program.";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_WAVEFORM_PREFIX_11N", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PREFIX_11AC, MAX_BUFFER_SIZE, "");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PREFIX_11AC))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PREFIX_11AC;
        setting.unit  = "";
        setting.helpText = "Prefix of 802.11ac waveform name. Default is non- value, use default waveform name defined in program.";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_WAVEFORM_PREFIX_11AC", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.PER_FRAME_COUNT_11B = 500;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_FRAME_COUNT_11B))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_FRAME_COUNT_11B;
        setting.unit  = "dB";
        setting.helpText = "Number of frame count to verify 802.11b PER. Default is 500";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_FRAME_COUNT_11B", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.PER_FRAME_COUNT_11AG = 500;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_FRAME_COUNT_11AG))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_FRAME_COUNT_11AG;
        setting.unit  = "dB";
        setting.helpText = "Number of frame count to verify 802.11ag PER. Default is 500";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_FRAME_COUNT_11AG", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.PER_FRAME_COUNT_11N = 500;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_FRAME_COUNT_11N))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_FRAME_COUNT_11N;
        setting.unit  = "dB";
        setting.helpText = "Number of frame count to verify 802.11n PER. Default is 500";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_FRAME_COUNT_11N", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
	setting.type = WIFI_SETTING_TYPE_INTEGER;

	g_WiFi11ACGlobalSettingParam.PER_FRAME_COUNT_11AC = 500;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_FRAME_COUNT_11AC))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_FRAME_COUNT_11AC;
        setting.unit  = "dB";
        setting.helpText = "Number of frame count to verify 802.11ac PER. Default is 500";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_FRAME_COUNT_11AC", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PATH, MAX_BUFFER_SIZE, "../iqvsg");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PATH))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PATH;
        setting.unit  = "";
        setting.helpText = "Path where all waveform files are stored for RX PER testing.\r\nDefault setting is the Mod folder under IQlite";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_WAVEFORM_PATH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_DESTINATION_MAC, MAX_BUFFER_SIZE, "000000C0FFEE");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_DESTINATION_MAC))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_DESTINATION_MAC;
        setting.unit  = "";
        setting.helpText = "Destination MAC address used in all waveforms. All four MAC addresses could be set to the same one.\r\nDefault value is 000000C0FFEE";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_WAVEFORM_DESTINATION_MAC", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    /*setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11B_1_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11B_1_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11B_1_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of 1 Mbps sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_1_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11B_1_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_DSSS-1.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11B_1_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11B_1_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 1 Mbps data rate.\r\nDefault value is WiFi_DSSS-1.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_1_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11B_2_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11B_2_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11B_2_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of 2 Mbps sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_2_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11B_2L_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_DSSS-2L.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11B_2L_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11B_2L_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 2 Mbps data rate(Long Preamble).\r\nDefault value is WiFi_DSSS-2L.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_2L_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11B_2S_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_DSSS-2S.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11B_2S_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11B_2S_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 2 Mbps data rate(Short Preamble).\r\nDefault value is WiFi_DSSS-2S.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_2S_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11B_5_5_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11B_5_5_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11B_5_5_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of 5_5 Mbps sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_5_5_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11B_5_5L_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_CCK-5_5L.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11B_5_5L_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11B_5_5L_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 5_5 Mbps data rate(Long Preamble).\r\nDefault value is WiFi_CCK-5_5L.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_5_5L_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11B_5_5S_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_CCK-5_5S.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11B_5_5S_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11B_5_5S_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 5.5 Mbps data rate(Short Preamble).\r\nDefault value is WiFi_CCK-5_5S.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_5_5S_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11B_11_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11B_11_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11B_11_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of 11 Mbps sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_11_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11B_11L_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_CCK-11L.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11B_11L_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11B_11L_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 11 Mbps data rate(Long Preamble).\r\nDefault value is WiFi_CCK-11L.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_11L_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11B_11S_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_CCK-11S.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11B_11S_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11B_11S_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 11 Mbps data rate(Short Preamble).\r\nDefault value is WiFi_CCK-11S.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_11S_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11AG_6_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11AG_6_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11AG_6_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of OFDM-6 sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_6_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11AG_6_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_OFDM-6.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11AG_6_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11AG_6_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for OFDM-6 data rate.\r\nDefault value is WiFi_OFDM-6.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_6_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11AG_9_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11AG_9_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11AG_9_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of OFDM-9 sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_9_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11AG_9_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_OFDM-9.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11AG_9_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11AG_9_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for OFDM-9 data rate.\r\nDefault value is WiFi_OFDM-9.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_9_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11AG_12_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11AG_12_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11AG_12_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of OFDM-12 sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_12_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11AG_12_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_OFDM-12.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11AG_12_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11AG_12_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for OFDM-12 data rate.\r\nDefault value is WiFi_OFDM-12.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_12_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11AG_18_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11AG_18_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11AG_18_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of OFDM-18 sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_18_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11AG_18_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_OFDM-18.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11AG_18_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11AG_18_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for OFDM-18 data rate.\r\nDefault value is WiFi_OFDM-18.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_18_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11AG_24_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11AG_24_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11AG_24_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of OFDM-24 sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_24_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11AG_24_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_OFDM-24.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11AG_24_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11AG_24_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for OFDM-24 data rate.\r\nDefault value is WiFi_OFDM-24.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_24_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11AG_36_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11AG_36_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11AG_36_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of OFDM-36 sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_36_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11AG_36_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_OFDM-36.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11AG_36_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11AG_36_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for OFDM-36 data rate.\r\nDefault value is WiFi_OFDM-36.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_36_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11AG_48_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11AG_48_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11AG_48_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of OFDM-48 sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_48_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11AG_48_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_OFDM-48.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11AG_48_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11AG_48_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for OFDM-48 data rate.\r\nDefault value is WiFi_OFDM-48.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_48_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11AG_54_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11AG_54_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11AG_54_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of OFDM-54 sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_54_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11AG_54_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_OFDM-54.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11AG_54_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11AG_54_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for OFDM-54 data rate.\r\nDefault value is WiFi_OFDM-54.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_54_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS0_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS0_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS0_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS0 (mixed format) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS0_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS0_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS0.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS0_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS0_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS0 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS0.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS0_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS1_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS1_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS1_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS1 (mixed format) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS1_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS1_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS1.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS1_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS1_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS1 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS1.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS1_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS2_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS2_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS2_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS2 (mixed format) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS2_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS2_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS2.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS2_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS2_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS2 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS2.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS2_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS3_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS3_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS3_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS3 (mixed format) sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS3_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS3_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS3.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS3_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS3_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS3 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS3.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS3_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS4_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS4_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS4_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS4 (mixed format) sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS4_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS4_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS4.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS4_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS4_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS4 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS4.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS4_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS5_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS5_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS5_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS5 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS5_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS5_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS5.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS5_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS5_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS5 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS5.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS5_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS6_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS6_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS6_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS6 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS6_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS6_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS6.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS6_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS6_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS6 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS6.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS6_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS7_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS7_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS7_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS7 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS7_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS7_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS7.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS7_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS7_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS7 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS7.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS7_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS8_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS8_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS8_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS8 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS8_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS8_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS8.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS8_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS8_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS8 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS8.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS8_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS9_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS9_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS9_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS9 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS9_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS9_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS9.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS9_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS9_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS9 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS9.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS9_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS10_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS10_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS10_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS10 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS10_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS10_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS10.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS10_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS10_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS10 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS10.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS10_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS11_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS11_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS11_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS11 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS11_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS11_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS11.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS11_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS11_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS11 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS11.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS11_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS12_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS12_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS12_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS12 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS12_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS12_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS12.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS12_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS12_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS12 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS12.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS12_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS13_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS13_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS13_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS13 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS13_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS13_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS13.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS13_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS13_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS13 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS13.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS13_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS14_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS14_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS14_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS14 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS14_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS14_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS14.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS14_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS14_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS14 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS14.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS14_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS15_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS15_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS15_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS15 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS15_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS15_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS15.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS15_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS15_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS15 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS15.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS15_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS16_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS16_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS16_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS16 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS16_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS16_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS16.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS16_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS16_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS16 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS16.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS16_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS17_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS17_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS17_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS17 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS17_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS17_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS17.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS17_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS17_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS17 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS17.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS17_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS18_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS18_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS18_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS18 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS18_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS18_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS18.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS18_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS18_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS18 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS18.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS18_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS19_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS19_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS19_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS19 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS19_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS19_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS19.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS19_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS19_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS19 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS19.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS19_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS20_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS20_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS20_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS20 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS20_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS20_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS20.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS20_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS20_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS20 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS20.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS20_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS21_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS21_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS21_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS21 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS21_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS21_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS21.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS21_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS21_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS21 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS21.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS21_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS22_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS22_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS22_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS22 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS22_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS22_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS22.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS22_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS22_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS22 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS22.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS22_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS23_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS23_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS23_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS23 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS23_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS23_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS23.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS23_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS23_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS23 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS23.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS23_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS24_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS24_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS24_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS24 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS24_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS24_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS24.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS24_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS24_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS24 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS24.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS24_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS25_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS25_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS25_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS25 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS25_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS25_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS25.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS25_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS25_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS25 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS25.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS25_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS26_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS26_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS26_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS26 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS26_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS26_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS26.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS26_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS26_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS26 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS26.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS26_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS27_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS27_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS27_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS27 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS27_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS27_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS27.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS27_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS27_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS27 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS27.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS27_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS28_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS28_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS28_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS28 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS28_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS28_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS28.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS28_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS28_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS28 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS28.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS28_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS29_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS29_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS29_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS29 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS29_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS29_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS29.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS29_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS29_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS29 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS29.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS29_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS30_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS30_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS30_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS30 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS30_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS30_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS30.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS30_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS30_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS30 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS30.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS30_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS31_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS31_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS31_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS31 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS31_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS31_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS31.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS31_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT20_MIXED_MCS31_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS31 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS31.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS31_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS0_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS0_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS0_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS0 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS0_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS0_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS0.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS0_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS0_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS0 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS0.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS0_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS1_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS1_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS1_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS1 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS1_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS1_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS1.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS1_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS1_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS1 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS1.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS1_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS2_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS2_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS2_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS2 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS2_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS2_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS2.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS2_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS2_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS2 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS2.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS2_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS3_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS3_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS3_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS3 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS3_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS3_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS3.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS3_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS3_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS3 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS3.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS3_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS4_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS4_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS4_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS4 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS4_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS4_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS4.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS4_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS4_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS4 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS4.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS4_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS5_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS5_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS5_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS5 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS5_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS5_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS5.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS5_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS5_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS5 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS5.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS5_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS6_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS6_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS6_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS6 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS6_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS6_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS6.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS6_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS6_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS6 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS6.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS6_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS7_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS7_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS7_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS7 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS7_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS7_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS7.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS7_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS7_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS7 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS7.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS7_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS8_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS8_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS8_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS8 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS8_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS8_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS8.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS8_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS8_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS8 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS8.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS8_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS9_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS9_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS9_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS9 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS9_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS9_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS9.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS9_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS9_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS9 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS9.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS9_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS10_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS10_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS10_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS10 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS10_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS10_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS10.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS10_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS10_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS10 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS10.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS10_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS11_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS11_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS11_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS11 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS11_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS11_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS11.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS11_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS11_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS11 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS11.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS11_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS12_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS12_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS12_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS12 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS12_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS12_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS12.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS12_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS12_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS12 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS12.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS12_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS13_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS13_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS13_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS13 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS13_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS13_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS13.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS13_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS13_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS13 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS13.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS13_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS14_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS14_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS14_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS14 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS14_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS14_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS14.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS14_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS14_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS14 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS14.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS14_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS15_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS15_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS15_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS15 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS15_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS15_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS15.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS15_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS15_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS15 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS15.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS15_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS16_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS16_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS16_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS16 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS16_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS16_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS16.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS16_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS16_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS16 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS16.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS16_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS17_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS17_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS17_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS17 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS17_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS17_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS17.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS17_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS17_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS17 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS17.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS17_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS18_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS18_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS18_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS18 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS18_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS18_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS18.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS18_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS18_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS18 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS18.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS18_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS19_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS19_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS19_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS19 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS19_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS19_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS19.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS19_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS19_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS19 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS19.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS19_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS20_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS20_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS20_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS20 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS20_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS20_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS20.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS20_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS20_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS20 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS20.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS20_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS21_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS21_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS21_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS21 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS21_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS21_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS21.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS21_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS21_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS21 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS21.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS21_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS22_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS22_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS22_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS22 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS22_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS22_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS22.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS22_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS22_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS22 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS22.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS22_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS23_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS23_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS23_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS23 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS23_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS23_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS23.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS23_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS23_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS23 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS23.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS23_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS24_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS24_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS24_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS24 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS24_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS24_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS24.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS24_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS24_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS24 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS24.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS24_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS25_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS25_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS25_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS25 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS25_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS25_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS25.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS25_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS25_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS25 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS25.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS25_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS26_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS26_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS26_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS26 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS26_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS26_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS26.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS26_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS26_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS26 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS26.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS26_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS27_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS27_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS27_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS27 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS27_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS27_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS27.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS27_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS27_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS27 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS27.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS27_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS28_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS28_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS28_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS28 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS28_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS28_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS28.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS28_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS28_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS28 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS28.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS28_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS29_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS29_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS29_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS29 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS29_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS29_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS29.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS29_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS29_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS29 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS29.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS29_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS30_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS30_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS30_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS30 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS30_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS30_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS30.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS30_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS30_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS30 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS30.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS30_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS31_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS31_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS31_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS31 (mixed format) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS31_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS31_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS31.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS31_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFi11ACGlobalSettingParam.PER_11N_HT40_MIXED_MCS31_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS31 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS31.mod";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS31_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }*/

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PM_AVERAGE = 3;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PM_AVERAGE))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PM_AVERAGE;
        setting.unit  = "";
        setting.helpText = "Averaging times used for Power measurement.\r\nDefault value is 1";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PM_AVERAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.VSA_TRIGGER_TYPE = 6;      // 6: IQV_TRIG_TYPE_IF2_NO_CAL
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.VSA_TRIGGER_TYPE))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.VSA_TRIGGER_TYPE;
        setting.unit  = "";
        setting.helpText = "Trigger types used for capturing.\r\nFree Run: 0\r\nExternal Trigger: 1\r\nSignal Trigger: 6\r\nSignal Trigger for IQ2010: 13";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_TRIGGER_TYPE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    g_WiFi11ACGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB = 3;
    if (sizeof(double)==sizeof(g_WiFi11ACGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB;
        setting.unit  = "dB";
        setting.helpText  = "IQTester VSA amplitude setting tolerance. -3 < Default < +3 dB.";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_AMPLITUDE_TOLERANCE_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    g_WiFi11ACGlobalSettingParam.VSA_TRIGGER_LEVEL_DB = -25;
    if (sizeof(double)==sizeof(g_WiFi11ACGlobalSettingParam.VSA_TRIGGER_LEVEL_DB))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.VSA_TRIGGER_LEVEL_DB;
        setting.unit  = "dB";
        setting.helpText  = "Signal trigger level";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_TRIGGER_LEVEL_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_DOUBLE;
	g_WiFi11ACGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US = 3;
    if (sizeof(double)==sizeof(g_WiFi11ACGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US;
        setting.unit  = "us";
        setting.helpText  = "IQTester VSA signal pre-trigger time setting used for signal capture.";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_PRE_TRIGGER_TIME_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.VSA_PORT = PORT_LEFT;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.VSA_PORT))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.VSA_PORT;
        setting.unit  = "";
        setting.helpText  = "VSA RF port\r\n2 for RF1(LEFT) and 3 for RF2(RIGHT)";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_PORT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.VSG_PORT = PORT_LEFT;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.VSG_PORT))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.VSG_PORT;
        setting.unit  = "";
        setting.helpText  = "VSG RF port\r\n2 for RF1(LEFT) and 3 for RF2(RIGHT)";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSG_PORT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.VSA_SAVE_CAPTURE_ON_FAILED = 1;	
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.VSA_SAVE_CAPTURE_ON_FAILED))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.VSA_SAVE_CAPTURE_ON_FAILED;
        setting.unit  = "";
        setting.helpText  = "A flag that to save sig file when capture failed, 0: OFF, 1: ON, Default is 1.";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_SAVE_CAPTURE_ON_FAILED", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS = 0;	
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS;
        setting.unit  = "";
        setting.helpText  = "A flag that to save sig file, always, 0: OFF, 1: ON, Default is 0.";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_SAVE_CAPTURE_ALWAYS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
    
    setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.DUT_KEEP_TRANSMIT = 1;	
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.DUT_KEEP_TRANSMIT))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.DUT_KEEP_TRANSMIT;
        setting.unit  = "";
        setting.helpText  = "A flag that to let Dut keep Tx until the configuration changed, 0: OFF, 1: ON, Default=1.";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("DUT_KEEP_TRANSMIT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.DUT_TX_SETTLE_TIME_MS = 0;	
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.DUT_TX_SETTLE_TIME_MS))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.DUT_TX_SETTLE_TIME_MS;
        setting.unit  = "ms";
        setting.helpText  = "A delay time for DUT (TX) settle, Default = 0(ms).";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("DUT_TX_SETTLE_TIME_MS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.DUT_RX_SETTLE_TIME_MS = 0;	
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.DUT_RX_SETTLE_TIME_MS))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.DUT_RX_SETTLE_TIME_MS;
        setting.unit  = "ms";
        setting.helpText  = "A delay time for DUT (RX) settle, Default = 0(ms).";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("DUT_RX_SETTLE_TIME_MS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    // [802.11b] Parameters
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_EQ_TAPS = 1;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_EQ_TAPS))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_EQ_TAPS;
        setting.unit  = "";
        setting.helpText = "Number of equalizer taps for 802.11b analysis\r\nOptions are 1,5,7,9\r\nDefault value is 1";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11B_EQ_TAPS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_DC_REMOVE_FLAG = 0;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_DC_REMOVE_FLAG))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_DC_REMOVE_FLAG;
        setting.unit  = "";
        setting.helpText = "DC removal for 802.11b analysis\r\nOptions are 0,1\r\nDefault value is 0";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11B_DC_REMOVE_FLAG", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_METHOD_11B = 1;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_METHOD_11B))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_METHOD_11B;
        setting.unit  = "";
        setting.helpText = "802.11b method\r\n1: Use 11b standard TX accuracy; 2:Use 11b RMS error vector\r\nDefault value is 1";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11B_METHOD_11B", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
	
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_FIXED_01_DATA_SEQUENCE = 0;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_FIXED_01_DATA_SEQUENCE))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_FIXED_01_DATA_SEQUENCE;
        setting.unit  = "";
        setting.helpText = "Enable or disable 802.11b (0101) fixed data sequence.\r\n0: Disable; 1: Enable\r\nDefault value is 0";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11B_FIXED_01_DATA_SEQUENCE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    // [802.11a/g] Parameters
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_PH_CORR_MODE = 2;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_PH_CORR_MODE))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_PH_CORR_MODE;
        setting.unit  = "";
        setting.helpText = "Phase Correction Mode for 802.11 a/g analysis\r\n1:Phase Correction Off; 2:Symbol-by-symbol Correction; 3:Moving Average(10 symbols) Correction\r\nDefault value is 2";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11AG_PH_CORR_MODE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_CH_ESTIMATE = 1;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_CH_ESTIMATE))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_CH_ESTIMATE;
        setting.unit  = "";
        setting.helpText = "Channel Estimate for 802.11 a/g analysis\r\n1:Long Training Sequence; 2:2nd Order Polyfit; 3:Full Packet\r\nDefault value is 1";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11AG_CH_ESTIMATE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR = 2;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR;
        setting.unit  = "";
        setting.helpText = "Symbol Timing Correction for 802.11 a/g analysis\r\n1:Off; 2:On\r\nDefault value is 2";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11AG_SYM_TIM_CORR", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_FREQ_SYNC = 2;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_FREQ_SYNC))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_FREQ_SYNC;
        setting.unit  = "";
        setting.helpText = "Frequency Sync. Mode for 802.11 a/g analysis\r\n1:Short Training Symbol; 2:Long Training Symbol; 3:Full Data Packet\r\nDefault value is 2";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11AG_FREQ_SYNC", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_AMPL_TRACK = 1;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_AMPL_TRACK))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_AMPL_TRACK;
        setting.unit  = "";
        setting.helpText = "Amplitude Tracking for 802.11 a/g analysis\r\n1:Off; 2:On\r\nDefault value is 1";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11AG_AMPL_TRACK", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


    // [MIMO] Parameters
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_PHASE_CORR = 1;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_PHASE_CORR))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_PHASE_CORR;
        setting.unit  = "";
        setting.helpText = "Phase Correction for 802.11n analysis\r\n0:Off; 1:On\r\nDefault value is 1";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11N_PHASE_CORR", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_SYM_TIMING_CORR = 1;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_SYM_TIMING_CORR))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_SYM_TIMING_CORR;
        setting.unit  = "";
        setting.helpText = "Symbol Timing Correction for 802.11n analysis\r\n0:Off; 1:On\r\nDefault value is 1";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11N_SYM_TIMING_CORR", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_AMPLITUDE_TRACKING = 0;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_AMPLITUDE_TRACKING))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_AMPLITUDE_TRACKING;
        setting.unit  = "";
        setting.helpText = "Amplitude Tracking for 802.11n analysis\r\n0:Off; 1:On\r\nDefault value is 0";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11N_AMPLITUDE_TRACKING", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_DECODE_PSDU = 0;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_DECODE_PSDU))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_DECODE_PSDU;
        setting.unit  = "";
        setting.helpText = "Decode PSDU for 802.11n analysis\r\n0:Off; 1:On\r\nDefault value is 0";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11N_DECODE_PSDU", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST = 0;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST;
        setting.unit  = "";
        setting.helpText = "Channel Estimate over Full Packet for 802.11n analysis\r\n0:Off; 1:On\r\nDefault value is 0";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11N_FULL_PACKET_CHANNEL_EST", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_FREQUENCY_CORRELATION = 2;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_FREQUENCY_CORRELATION))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_FREQUENCY_CORRELATION;
        setting.unit  = "";
		setting.helpText = "frequency correction on short and long legacy training fields for 802.11n analysis\r\n2=Long Training Field;3:LTF+SIG Field;4:Full Packet;\r\nDefault value is 2";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11N_FREQUENCY_CORRELATION", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


	 // [80.11AC] Parameters
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_PHASE_CORR = 1;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_PHASE_CORR))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_PHASE_CORR;
        setting.unit  = "";
        setting.helpText = "Phase Correction for 802.11ac analysis\r\n0:Off; 1:On\r\nDefault value is 1";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11AC_PHASE_CORR", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_SYM_TIMING_CORR = 1;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_SYM_TIMING_CORR))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_SYM_TIMING_CORR;
        setting.unit  = "";
        setting.helpText = "Symbol Timing Correction for 802.11ac analysis\r\n0:Off; 1:On\r\nDefault value is 1";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11AC_SYM_TIMING_CORR", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_AMPLITUDE_TRACKING = 0;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_AMPLITUDE_TRACKING))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_AMPLITUDE_TRACKING;
        setting.unit  = "";
        setting.helpText = "Amplitude Tracking for 802.11ac analysis\r\n0:Off; 1:On\r\nDefault value is 0";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11AC_AMPLITUDE_TRACKING", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_DECODE_PSDU = 0;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_DECODE_PSDU))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_DECODE_PSDU;
        setting.unit  = "";
        setting.helpText = "Decode PSDU for 802.11ac analysis\r\n0:Off; 1:On\r\nDefault value is 0";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11AC_DECODE_PSDU", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_FULL_PACKET_CHANNEL_EST = 0;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_FULL_PACKET_CHANNEL_EST))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_FULL_PACKET_CHANNEL_EST;
        setting.unit  = "";
        setting.helpText = "Channel Estimate over Full Packet for 802.11ac analysis\r\n0:Off; 1:On\r\nDefault value is 0";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11AC_FULL_PACKET_CHANNEL_EST", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_FREQUENCY_CORRELATION = 2;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_FREQUENCY_CORRELATION))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_FREQUENCY_CORRELATION;
        setting.unit  = "";
		setting.helpText = "frequency correction on short and long legacy training fields for 802.11ac analysis\r\n2=Long Training Field;3:LTF+SIG Field;4:Full Packet;\r\nDefault value is 2";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11AC_FREQUENCY_CORRELATION", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


    //setting.type = WIFI_SETTING_TYPE_INTEGER;
    //g_WiFi11ACGlobalSettingParam.IQ_PM_METHOD = 0;
    //if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_PM_METHOD))    // Type_Checking
    //{
    //    setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_PM_METHOD;
    //    setting.unit  = "";
    //    setting.helpText = "";
    //    g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_PM_METHOD", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PM_IF_FREQ_SHIFT_MHZ = 0;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PM_IF_FREQ_SHIFT_MHZ))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PM_IF_FREQ_SHIFT_MHZ;
        setting.unit  = "MHz";
        setting.helpText = "VSA center frequency shift in MHz in the case where DUT has a strong LO leakage.\r\nDefault value is 0";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PM_IF_FREQ_SHIFT_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PM_DSSS_SAMPLE_INTERVAL_US = 100;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PM_DSSS_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PM_DSSS_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Required capture time when measuring DSSS power in trigger mode.\r\nDefault value is 100 us.";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PM_DSSS_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.PM_OFDM_SAMPLE_INTERVAL_US = 20;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PM_OFDM_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PM_OFDM_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Required capture time when measuring OFDM power in trigger mode.\r\nDefault value is 20 us.";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PM_OFDM_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.SPECTRUM_DSSS_SAMPLE_INTERVAL_US = 286;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.SPECTRUM_DSSS_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.SPECTRUM_DSSS_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Required capture time when measuring DSSS spectrum in trigger mode.\r\nDefault value is 286 us.";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("SPECTRUM_DSSS_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.SPECTRUM_OFDM_SAMPLE_INTERVAL_US = 95;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.SPECTRUM_OFDM_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.SPECTRUM_OFDM_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Required capture time when measuring OFDM spectrum in trigger mode.\r\nDefault value is 95 us.";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("SPECTRUM_OFDM_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.MASK_DSSS_SAMPLE_INTERVAL_US = 286;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.MASK_DSSS_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.MASK_DSSS_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Required capture time when measuring DSSS MASK in trigger mode.\r\nDefault value is 286 us.";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("MASK_DSSS_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.MASK_OFDM_SAMPLE_INTERVAL_US = 95;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.MASK_OFDM_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.MASK_OFDM_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Required capture time when measuring OFDM MASK in trigger mode.\r\nDefault value is 95 us.";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("MASK_OFDM_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.EVM_AVERAGE = 3;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.EVM_AVERAGE))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.EVM_AVERAGE;
        setting.unit  = "";
        setting.helpText = "Required minumum number of EVM measurements.\r\nDefalut value is 3";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_AVERAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.EVM_SYMBOL_NUM = 18;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.EVM_SYMBOL_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.EVM_SYMBOL_NUM;
        setting.unit  = "";
        setting.helpText = "Number of symbols used for EVM measurement (not used)";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_SYMBOL_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.EVM_PRE_TRIG_TIME_US = 3;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.EVM_PRE_TRIG_TIME_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.EVM_PRE_TRIG_TIME_US;
        setting.unit  = "us";
        setting.helpText = "Pre-trigger time for EVM measurement.\r\nDefault value is 3 us";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_PRE_TRIG_TIME_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11B_L_US = 286;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11B_L_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11B_L_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring 11b (long preamble) EVM.\r\nDefault value is 286 us";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_CAPTURE_TIME_11B_L_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11B_S_US = 190;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11B_S_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11B_S_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring 11b (short preamble) EVM.\r\nDefault value is 190 us";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_CAPTURE_TIME_11B_S_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11AG_US = 95;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11AG_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11AG_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring 11ag EVM.\r\nDefault value is 95 us";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_CAPTURE_TIME_11AG_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11N_MIXED_US = 123;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11N_MIXED_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11N_MIXED_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring 11n mixed format EVM.\r\nDefault value is 123 us";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_CAPTURE_TIME_11N_MIXED_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11N_GREENFIELD_US = 115;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11N_GREENFIELD_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11N_GREENFIELD_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring 11n greenfield format EVM.\r\nDefault value is 115 us";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_CAPTURE_TIME_11N_GREENFIELD_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


	// 802.11ac sampling time. precise value will be decided later.
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11AC_HT_GF_US = 300;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11AC_HT_GF_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11AC_HT_GF_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring 11ac greenfield format HT EVM.\r\nDefault value is 300 us";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_CAPTURE_TIME_11AC_HT_GF_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11AC_HT_MF_US = 300;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11AC_HT_MF_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11AC_HT_MF_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring 11ac mixed format HT EVM.\r\nDefault value is 300 us";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_CAPTURE_TIME_11AC_HT_MF_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11AC_NON_HT_US = 300;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11AC_NON_HT_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11AC_NON_HT_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring 11ac non-HT EVM.\r\nDefault value is 300 us";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_CAPTURE_TIME_11AC_NON_HT_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11AC_VHT_US = 300;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11AC_VHT_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11AC_VHT_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring 11ac VHT EVM.\r\nDefault value is 300 us";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_CAPTURE_TIME_11AC_VHT_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	//setting.type = WIFI_SETTING_TYPE_INTEGER;
	//g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_PSDU_BYTE = 1000;
	//if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_PSDU_BYTE))    // Type_Checking
	//{
	//	setting.value = (void*)&g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_PSDU_BYTE;
	//	setting.unit  = "";
	//	setting.helpText = "No. of PSDU byte to be captured for EVM measurement.\r\nDefault value is 1000 bytes";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_CAPTURE_PSDU_BYTE_NUM", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS0_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS0.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS0_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS0_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT20 MCS0 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS0.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS0_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS1_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS1.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS1_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS1_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT20 MCS1 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS1.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS1_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS2_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS2.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS2_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS2_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT20 MCS2 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS2.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS2_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS3_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS3.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS3_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS3_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT20 MCS3 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS3.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS3_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS4_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS4.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS4_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS4_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT20 MCS4 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS4.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS4_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS5_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS5.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS5_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS5_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT20 MCS5 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS5.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS5_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS6_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS6.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS6_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS6_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT20 MCS6 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS6.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS6_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS7_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS7.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS7_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS7_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT20 MCS7 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS7.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS7_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

 //   setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS8_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS8.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS8_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS8_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS8 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS8.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS8_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS9_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS9.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS9_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS9_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS9 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS9.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS9_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS10_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS10.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS10_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS10_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS10 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS10.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS10_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS11_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS11.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS11_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS11_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS11 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS11.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS11_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS12_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS12.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS12_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS12_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS12 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS12.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS12_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS13_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS13.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS13_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS13_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS13 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS13.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS13_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }


 //   setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS14_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS14.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS14_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS14_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS14 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS14.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS14_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS15_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS15.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS15_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS15_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS15 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS15.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS15_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS16_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS16.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS16_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS16_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT20 MCS16 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS16.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS16_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}
	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS17_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS17.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS17_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS17_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT20 MCS17 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS17.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS17_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}
	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS18_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS18.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS18_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS18_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT20 MCS18 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS18.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS18_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}
	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS19_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS19.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS19_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS19_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT20 MCS19 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS19.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS19_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}
	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS20_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS20.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS20_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS20_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT20 MCS20 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS20.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS20_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}
	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS21_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS21.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS21_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS21_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT20 MCS21 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS21.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS21_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}
	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS22_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS22.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS22_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS22_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT20 MCS22 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS22.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS22_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}
	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS23_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS23.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS23_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_MIXED_MCS23_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT20 MCS23 (mixed format) data rate.\r\nDefault value is WiFi_HT20_MCS23.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS23_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS0_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS0.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS0_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS0_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT40 MCS0 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS0.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS0_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS1_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS1.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS1_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS1_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT40 MCS1 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS1.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS1_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS2_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS2.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS2_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS2_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT40 MCS2 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS2.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS2_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS3_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS3.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS3_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS3_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT40 MCS3 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS3.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS3_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS4_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS4.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS4_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS4_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT40 MCS4 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS4.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS4_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS5_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS5.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS5_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS5_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT40 MCS5 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS5.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS5_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS6_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS6.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS6_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS6_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT40 MCS6 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS6.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS6_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS7_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS7.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS7_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS7_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT40 MCS7 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS7.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS7_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

 //   setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS8_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS8.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS8_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS8_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS8 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS8.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS8_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS9_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS9.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS9_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS9_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS9 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS9.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS9_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS10_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS10.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS10_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS10_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS10 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS10.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS10_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS11_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS11.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS11_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS11_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS11 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS11.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS11_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS12_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS12.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS12_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS12_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS12 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS12.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS12_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS13_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS13.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS13_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS13_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS13 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS13.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS13_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS14_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS14.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS14_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS14_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS14 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS14.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS14_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS15_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS15.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS15_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS15_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS15 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS15.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS15_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS16_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS16.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS16_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS16_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT40 MCS16 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS16.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS16_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}
	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS17_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS17.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS17_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS17_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT40 MCS17 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS17.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS17_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}
	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS18_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS18.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS18_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS18_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT40 MCS18 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS18.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS18_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}
	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS19_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS19.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS19_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS19_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT40 MCS19 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS19.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS19_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}
	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS20_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS20.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS20_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS20_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT40 MCS20 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS20.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS20_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}
	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS21_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS21.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS21_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS21_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT40 MCS21 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS21.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS21_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}
	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS22_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS22.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS22_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS22_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT40 MCS22 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS22.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS22_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}
	//setting.type = WIFI_SETTING_TYPE_STRING;
	//strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS23_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS23.ref");
	//if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS23_REFERENCE_FILE_NAME))    // Type_Checking
	//{
	//	setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_MIXED_MCS23_REFERENCE_FILE_NAME;
	//	setting.unit  = "";
	//	setting.helpText = "Reference file name for HT40 MCS23 (mixed format) data rate.\r\nDefault value is WiFi_HT40_MCS23.ref";
	//	g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS23_REFERENCE_FILE_NAME", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS8_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS8.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS8_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS8_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS8 (greenfield format) data rate.\r\nDefault value is WiFi_HT20_MCS8.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_MCS8_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS9_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS9.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS9_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS9_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS9 (greenfield format) data rate.\r\nDefault value is WiFi_HT20_MCS9.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_MCS9_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS10_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS10.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS10_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS10_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS10 (greenfield format) data rate.\r\nDefault value is WiFi_HT20_MCS10.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_MCS10_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS11_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS11.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS11_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS11_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS11 (greenfield format) data rate.\r\nDefault value is WiFi_HT20_MCS11.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_MCS11_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS12_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS12.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS12_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS12_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS12 (greenfield format) data rate.\r\nDefault value is WiFi_HT20_MCS12.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_MCS12_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS13_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS13.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS13_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS13_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS13 (greenfield format) data rate.\r\nDefault value is WiFi_HT20_MCS13.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_MCS13_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }


 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS14_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS14.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS14_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS14_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS14 (greenfield format) data rate.\r\nDefault value is WiFi_HT20_MCS14.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_MCS14_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS15_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS15.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS15_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS15_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS15 (greenfield format) data rate.\r\nDefault value is WiFi_HT20_MCS15.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_MCS15_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS16_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS16.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS16_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS16_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS16 (greenfield format) data rate.\r\nDefault value is WiFi_HT20_MCS16.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_MCS16_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }
 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS17_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS17.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS17_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS17_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS17 (greenfield format) data rate.\r\nDefault value is WiFi_HT20_MCS17.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_MCS17_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }
 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS18_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS18.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS18_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS18_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS18 (greenfield format) data rate.\r\nDefault value is WiFi_HT20_MCS18.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_MCS18_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }
 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS19_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS19.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS19_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS19_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS19 (greenfield format) data rate.\r\nDefault value is WiFi_HT20_MCS19.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_MCS19_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }
 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS20_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS20.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS20_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS20_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS20 (greenfield format) data rate.\r\nDefault value is WiFi_HT20_MCS20.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_MCS20_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }
 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS21_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS21.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS21_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS21_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS21 (greenfield format) data rate.\r\nDefault value is WiFi_HT20_MCS21.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_MCS21_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }
 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS22_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS22.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS22_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS22_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS22 (greenfield format) data rate.\r\nDefault value is WiFi_HT20_MCS22.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_MCS22_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }
 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS23_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS23.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS23_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT20_GREENFIELD_MCS23_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT20 MCS23 (greenfield format) data rate.\r\nDefault value is WiFi_HT20_MCS23.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_MCS23_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }
 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS8_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS8.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS8_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS8_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS8 (greenfield format) data rate.\r\nDefault value is WiFi_HT40_MCS8.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_MCS8_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS9_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS9.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS9_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS9_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS9 (greenfield format) data rate.\r\nDefault value is WiFi_HT40_MCS9.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_MCS9_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS10_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS10.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS10_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS10_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS10 (greenfield format) data rate.\r\nDefault value is WiFi_HT40_MCS10.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_MCS10_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS11_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS11.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS11_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS11_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS11 (greenfield format) data rate.\r\nDefault value is WiFi_HT40_MCS11.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_MCS11_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS12_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS12.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS12_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS12_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS12 (greenfield format) data rate.\r\nDefault value is WiFi_HT40_MCS12.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_MCS12_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS13_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS13.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS13_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS13_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS13 (greenfield format) data rate.\r\nDefault value is WiFi_HT40_MCS13.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_MCS13_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS14_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS14.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS14_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS14_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS14 (greenfield format) data rate.\r\nDefault value is WiFi_HT40_MCS14.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_MCS14_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS15_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS15.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS15_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS15_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS15 (greenfield format) data rate.\r\nDefault value is WiFi_HT40_MCS15.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_MCS15_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS16_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS16.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS16_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS16_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS16 (greenfield format) data rate.\r\nDefault value is WiFi_HT40_MCS16.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_MCS16_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }
 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS17_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS17.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS17_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS17_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS17 (greenfield format) data rate.\r\nDefault value is WiFi_HT40_MCS17.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_MCS17_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }
 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS18_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS18.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS18_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS18_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS18 (greenfield format) data rate.\r\nDefault value is WiFi_HT40_MCS18.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_MCS18_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }
 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS19_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS19.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS19_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS19_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS19 (greenfield format) data rate.\r\nDefault value is WiFi_HT40_MCS19.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_MCS19_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }
 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS20_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS20.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS20_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS20_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS20 (greenfield format) data rate.\r\nDefault value is WiFi_HT40_MCS20.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_MCS20_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }
 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS21_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS21.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS21_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS21_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS21 (greenfield format) data rate.\r\nDefault value is WiFi_HT40_MCS21.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_MCS21_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }
 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS22_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS22.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS22_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS22_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS22 (greenfield format) data rate.\r\nDefault value is WiFi_HT40_MCS22.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_MCS22_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }
 //   setting.type = WIFI_SETTING_TYPE_STRING;
 //   strcpy_s(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS23_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS23.ref");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS23_REFERENCE_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WiFi11ACGlobalSettingParam.EVM_11N_HT40_GREENFIELD_MCS23_REFERENCE_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "Reference file name for HT40 MCS23 (greenfield format) data rate.\r\nDefault value is WiFi_HT40_MCS23.ref";
 //       g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_MCS23_REFERENCE_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }


	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11B = 0;
	if (sizeof(double)==sizeof(g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11B))    // Type_Checking
	{
		setting.value = (void*)&g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11B;
		setting.unit  = "";
		setting.helpText  = "VSG RF Max output power limit for 11 B signal \r\nDefault value is 0 dBm";
		g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSG_MAX_POWER_11B", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11G = -5;
	if (sizeof(double)==sizeof(g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11G))    // Type_Checking
	{
		setting.value = (void*)&g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11G;
		setting.unit  = "";
		setting.helpText  = "VSG RF Max output power limit for 11 G signal \r\nDefault value is -5 dBm";
		g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSG_MAX_POWER_11G", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11N = -5;
	if (sizeof(double)==sizeof(g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11N))    // Type_Checking
	{
		setting.value = (void*)&g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11N;
		setting.unit  = "";
		setting.helpText  = "VSG RF Max output power limit for 11 N signal \r\nDefault value is -5 dBm";
		g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSG_MAX_POWER_11N", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11AC = -5.0;
	if (sizeof(double)==sizeof(g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11AC))    // Type_Checking
	{
		setting.value = (void*)&g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11AC;
		setting.unit  = "";
		setting.helpText  = "VSG RF Max output power limit for 11 AC signal \r\nDefault value is -5 dBm";
		g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSG_MAX_POWER_11AC", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    //setting.type = WIFI_SETTING_TYPE_INTEGER;
    //g_WiFi11ACGlobalSettingParam.IQ_PER_PACKET_NUM = 0;
    //if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_PER_PACKET_NUM))    // Type_Checking
    //{
    //    setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_PER_PACKET_NUM;
    //    setting.unit  = "";
    //    setting.helpText = "";
    //    g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_PER_PACKET_NUM", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

    //setting.type = WIFI_SETTING_TYPE_INTEGER;
    //g_WiFi11ACGlobalSettingParam.IQ_PER_RX_SENS_PACKET_NUM = 0;
    //if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.IQ_PER_RX_SENS_PACKET_NUM))    // Type_Checking
    //{
    //    setting.value = (void*)&g_WiFi11ACGlobalSettingParam.IQ_PER_RX_SENS_PACKET_NUM;
    //    setting.unit  = "";
    //    setting.helpText = "";
    //    g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_PER_RX_SENS_PACKET_NUM", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

    setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.PER_VSG_TIMEOUT_SEC = 20;
    if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.PER_VSG_TIMEOUT_SEC))    // Type_Checking
    {
        setting.value = (void*)&g_WiFi11ACGlobalSettingParam.PER_VSG_TIMEOUT_SEC;
        setting.unit  = "sec";
        setting.helpText = "Timeout for waiting for VSG TxDone().\r\n Default=20 sec.";
        g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_VSG_TIMEOUT_SEC", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.retryTestItem = 0;
	if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.retryTestItem))    // Type_Checking
	{
		setting.value = (void*)&g_WiFi11ACGlobalSettingParam.retryTestItem;
		setting.unit  = "";
		setting.helpText = "Specify no. of retry on test item if results are failed.";
		g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("RETRY_TEST_ITEM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.retryTestItemDuringRetry = 0;
	if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.retryTestItemDuringRetry))    // Type_Checking
	{
		setting.value = (void*)&g_WiFi11ACGlobalSettingParam.retryTestItemDuringRetry;
		setting.unit  = "";
		setting.helpText = "Specify whether a DUT reset function will be called when a retry occurs,\ncalls the \"RESET_TEST_ITEM\" test function.";
		g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("RESET_TEST_ITEM_DURING_RETRY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.retryErrorItems = 0;
	if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.retryErrorItems))    // Type_Checking
	{
		setting.value = (void*)&g_WiFi11ACGlobalSettingParam.retryErrorItems;
		setting.unit  = "";
		setting.helpText = "Specify whether a retry an item if an error is returned,\nthis will not affect whether an item is retried for a limit failure";
		g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("RETRY_ERROR_ITEMS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_MASK_ACCORDING_CBW = 0;
	if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_MASK_ACCORDING_CBW))    // Type_Checking
	{
		setting.value = (void*)&g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_MASK_ACCORDING_CBW;
		setting.unit  = "";
		setting.helpText = "Indicate for 11AC, standard mask is according to CBW or bssBW. 0: not use CBW 1: use CBW. Default is 0, using bssBW as stardard mask.";
		g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11AC_MASK_ACCORDING_CBW", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFi11ACGlobalSettingParam.AUTO_READING_LIMIT = 0;
	if (sizeof(int)==sizeof(g_WiFi11ACGlobalSettingParam.AUTO_READING_LIMIT))    // Type_Checking
	{
		setting.value = (void*)&g_WiFi11ACGlobalSettingParam.AUTO_READING_LIMIT;
		setting.unit  = "";
		setting.helpText = "Read limit automatically or not.";
		g_WiFi11ACGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("AUTO_READING_LIMIT", setting) );
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
    l_WiFi11ACGlobalSettingReturnMap.clear();

    g_WiFi11ACGlobalSettingReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(g_WiFi11ACGlobalSettingReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)g_WiFi11ACGlobalSettingReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_WiFi11ACGlobalSettingReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}
