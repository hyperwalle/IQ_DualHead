#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_Test.h"
#include "WiFi_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

using namespace std;

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> g_WiFiGlobalSettingParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_WiFiGlobalSettingReturnMap;

WIFI_GLOBAL_SETTING g_WiFiGlobalSettingParam;

struct tagReturn
{
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} g_WiFiGlobalSettingReturn;

void ClearGlobalSettingReturn(void)
{
	g_WiFiGlobalSettingParamMap.clear();
	l_WiFiGlobalSettingReturnMap.clear();
}

#ifndef WIN32 
int initGlobalSettingContainers = InitializeGlobalSettingContainers();
#endif

WIFI_TEST_API int WiFi_Global_Setting(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_WiFiGlobalSettingReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(g_WiFiGlobalSettingParamMap);
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
        RespondToQueryReturn(l_WiFiGlobalSettingReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
	   /*---------------------------------------*
		* g_WiFi_Test_ID need to be valid (>=0) *
		*---------------------------------------*/
		if( g_WiFi_Test_ID<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] WiFi_Test_ID not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] WiFi_Test_ID = %d.\n", g_WiFi_Test_ID);
		}

		TM_ClearReturns(g_WiFi_Test_ID);

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		GetInputParameters(g_WiFiGlobalSettingParamMap);
		err = ERR_OK;	// Global Setting are optinal parameters, thus err always = ERR_OK
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Get input parameters return OK.\n");
    
        g_WiFiGlobalSettingParam.RESET_OTA_ATTN_TABLE = 1;
		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(g_WiFiGlobalSettingReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_WiFiGlobalSettingReturnMap);
		}
		else
		{
			// do nothing
		}
	}
    catch(char *msg)
    {
        ReturnErrorMessage(g_WiFiGlobalSettingReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(g_WiFiGlobalSettingReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
		err = -1;
    }

    return err;
}

int InitializeGlobalSettingContainers(void)
{
    /*------------------*
     * Input Parameters: *
     *------------------*/
    g_WiFiGlobalSettingParamMap.clear();

    WIFI_SETTING_STRUCT setting;

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE = 0;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE;
        setting.unit  = "";
        setting.helpText = "Auto-detect (by CONNECT_IQ_TESTER function) and enable the IQ2010_EXT mode. Default=OFF";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ2010_EXT_ENABLE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	//Auto Gain Control
	
	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.AUTO_GAIN_CONTROL_ENABLE = 0;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.AUTO_GAIN_CONTROL_ENABLE))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.AUTO_GAIN_CONTROL_ENABLE;
        setting.unit  = "";
        setting.helpText = "Auto detect the input power amplitude. Default = OFF. Currently Implemented while IQ2010Ext is enabled.";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("AUTO_GAIN_CONTROL_ENABLE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


    //Peak to Average Settings
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_1M = 3;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_1M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_1M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11b 1Mbps.\r\nDefault value is 3dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11B_1M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_2M = 3;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_2M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_2M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11b 2Mbps.\r\nDefault value is 3dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11B_2M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_5_5M = 3;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_5_5M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_5_5M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11b 5.5Mbps.\r\nDefault value is 3dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11B_5_5M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_11M = 3;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_11M))    // Type_Checking
    {
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11b 11Mbps.\r\nDefault value is 3dB";
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_11M;
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11B_11M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_CH14 = 5;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_CH14))    // Type_Checking
    {
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11b at channel 14.\r\nDefault value is 5dB";
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_CH14;
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11B_CH14", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_6M = 10;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_6M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_6M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11a/g OFDM-6.\r\nDefault value is 10dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11AG_6M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_9M = 10;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_9M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_9M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11a/g OFDM-9.\r\nDefault value is 10dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11AG_9M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_12M = 10;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_12M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_12M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11a/g OFDM-12.\r\nDefault value is 10dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11AG_12M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_18M = 10;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_18M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_18M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11a/g OFDM-18.\r\nDefault value is 10dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11AG_18M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_24M = 10;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_24M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_24M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11a/g OFDM-24.\r\nDefault value is 10dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11AG_24M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_36M = 10;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_36M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_36M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11a/g OFDM-36.\r\nDefault value is 10dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11AG_36M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_48M = 10;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_48M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_48M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11a/g OFDM-48.\r\nDefault value is 10dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11AG_48M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_54M = 10;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_54M))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_54M;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11a/g OFDM-54.\r\nDefault value is 10dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11AG_54M", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS0 = 10;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS0))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS0;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11n MCS0,MCS8,MCS16,MCS24.\r\nDefault value is 10dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11N_MCS0", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS1 = 10;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS1))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS1;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11n MCS1,MCS9,MCS17,MCS25.\r\nDefault value is 10dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11N_MCS1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS2 = 10;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS2))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS2;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11n MCS2,MCS10,MCS18,MCS26.\r\nDefault value is 10dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11N_MCS2", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS3 = 10;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS3))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS3;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11n MCS3,MCS11,MCS19,MCS27.\r\nDefault value is 10dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11N_MCS3", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS4 = 10;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS4))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS4;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11n MCS4,MCS12,MCS20,MCS28.\r\nDefault value is 10dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11N_MCS4", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS5 = 10;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS5))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS5;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11n MCS5,MCS13,MCS21,MCS29.\r\nDefault value is 10dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11N_MCS5", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS6 = 10;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS6))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS6;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11n MCS6,MCS14,MCS22,MCS30.\r\nDefault value is 10dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11N_MCS6", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS7 = 10;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS7))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_P_TO_A_11N_MCS7;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for 802.11n MCS7,MCS15,MCS23,MCS31.\r\nDefault value is 10dB";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_P_TO_A_11N_MCS7", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_WAVEFORM_PATH, MAX_BUFFER_SIZE, "../mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_WAVEFORM_PATH))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_WAVEFORM_PATH;
        setting.unit  = "";
        setting.helpText = "Path where all waveform files are stored for RX PER testing.\r\nDefault setting is the Mod folder under IQlite";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_WAVEFORM_PATH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_WAVEFORM_DESTINATION_MAC, MAX_BUFFER_SIZE, "010000C0FFEE");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_WAVEFORM_DESTINATION_MAC))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_WAVEFORM_DESTINATION_MAC;
        setting.unit  = "";
        setting.helpText = "Destination MAC address used in all waveforms. All four MAC addresses could be set to the same one.\r\nDefault value is 010000C0FFEE";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_WAVEFORM_DESTINATION_MAC", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11B_1_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11B_1_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11B_1_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of 1 Mbps sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_1_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11B_1_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_DSSS-1.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11B_1_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11B_1_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 1 Mbps data rate.\r\nDefault value is WiFi_DSSS-1.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_1_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11B_2_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11B_2_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11B_2_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of 2 Mbps sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_2_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11B_2L_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_DSSS-2L.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11B_2L_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11B_2L_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 2 Mbps data rate(Long Preamble).\r\nDefault value is WiFi_DSSS-2L.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_2L_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11B_2S_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_DSSS-2S.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11B_2S_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11B_2S_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 2 Mbps data rate(Short Preamble).\r\nDefault value is WiFi_DSSS-2S.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_2S_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11B_5_5_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11B_5_5_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11B_5_5_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of 5_5 Mbps sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_5_5_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11B_5_5L_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_CCK-5_5L.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11B_5_5L_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11B_5_5L_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 5_5 Mbps data rate(Long Preamble).\r\nDefault value is WiFi_CCK-5_5L.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_5_5L_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11B_5_5S_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_CCK-5_5S.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11B_5_5S_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11B_5_5S_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 5.5 Mbps data rate(Short Preamble).\r\nDefault value is WiFi_CCK-5_5S.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_5_5S_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11B_11_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11B_11_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11B_11_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of 11 Mbps sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_11_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11B_11L_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_CCK-11L.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11B_11L_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11B_11L_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 11 Mbps data rate(Long Preamble).\r\nDefault value is WiFi_CCK-11L.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_11L_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11B_11S_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_CCK-11S.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11B_11S_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11B_11S_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 11 Mbps data rate(Short Preamble).\r\nDefault value is WiFi_CCK-11S.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11B_11S_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11AG_6_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11AG_6_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11AG_6_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of OFDM-6 sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_6_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11AG_6_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_OFDM-6.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11AG_6_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11AG_6_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for OFDM-6 data rate.\r\nDefault value is WiFi_OFDM-6.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_6_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11AG_9_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11AG_9_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11AG_9_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of OFDM-9 sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_9_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11AG_9_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_OFDM-9.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11AG_9_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11AG_9_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for OFDM-9 data rate.\r\nDefault value is WiFi_OFDM-9.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_9_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11AG_12_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11AG_12_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11AG_12_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of OFDM-12 sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_12_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11AG_12_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_OFDM-12.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11AG_12_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11AG_12_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for OFDM-12 data rate.\r\nDefault value is WiFi_OFDM-12.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_12_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11AG_18_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11AG_18_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11AG_18_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of OFDM-18 sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_18_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11AG_18_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_OFDM-18.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11AG_18_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11AG_18_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for OFDM-18 data rate.\r\nDefault value is WiFi_OFDM-18.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_18_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11AG_24_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11AG_24_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11AG_24_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of OFDM-24 sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_24_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11AG_24_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_OFDM-24.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11AG_24_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11AG_24_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for OFDM-24 data rate.\r\nDefault value is WiFi_OFDM-24.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_24_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11AG_36_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11AG_36_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11AG_36_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of OFDM-36 sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_36_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11AG_36_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_OFDM-36.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11AG_36_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11AG_36_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for OFDM-36 data rate.\r\nDefault value is WiFi_OFDM-36.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_36_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11AG_48_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11AG_48_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11AG_48_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of OFDM-48 sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_48_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11AG_48_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_OFDM-48.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11AG_48_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11AG_48_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for OFDM-48 data rate.\r\nDefault value is WiFi_OFDM-48.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_48_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11AG_54_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11AG_54_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11AG_54_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of OFDM-54 sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_54_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11AG_54_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_OFDM-54.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11AG_54_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11AG_54_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for OFDM-54 data rate.\r\nDefault value is WiFi_OFDM-54.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11AG_54_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
	}

	// 11p HALF
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PER_11P_HALF3_PACKETS_NUM = 200;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11P_HALF3_PACKETS_NUM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11P_HALF3_PACKETS_NUM;
		setting.unit  = "";
		setting.helpText = "Packet number of HALF-3 sent to DUT for PER testing.\r\nDefault value is 200";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_HALF-3_PACKETS_NUM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.PER_11P_HALF3_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HALF-3.mod");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11P_HALF3_WAVEFORM_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.PER_11P_HALF3_WAVEFORM_NAME;
		setting.unit  = "";
		setting.helpText = "Waveform name for HALF-3 data rate.\r\nDefault value is WiFi_HALF-3.mod";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_HALF-3_WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PER_11P_HALF4_5_PACKETS_NUM = 500;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11P_HALF4_5_PACKETS_NUM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11P_HALF4_5_PACKETS_NUM;
		setting.unit  = "";
		setting.helpText = "Packet number of HALF-4_5 sent to DUT for PER testing.\r\nDefault value is 500";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_HALF-4_5_PACKETS_NUM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.PER_11P_HALF4_5_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HALF-4_5.mod");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11P_HALF4_5_WAVEFORM_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.PER_11P_HALF4_5_WAVEFORM_NAME;
		setting.unit  = "";
		setting.helpText = "Waveform name for HALF-4_5 data rate.\r\nDefault value is WiFi_HALF-4_5.mod";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_HALF-4_5_WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PER_11P_HALF6_PACKETS_NUM = 500;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11P_HALF6_PACKETS_NUM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11P_HALF6_PACKETS_NUM;
		setting.unit  = "";
		setting.helpText = "Packet number of HALF-6 sent to DUT for PER testing.\r\nDefault value is 500";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_HALF-6_PACKETS_NUM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.PER_11P_HALF6_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HALF-6.mod");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11P_HALF6_WAVEFORM_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.PER_11P_HALF6_WAVEFORM_NAME;
		setting.unit  = "";
		setting.helpText = "Waveform name for HALF-6 data rate.\r\nDefault value is WiFi_HALF-6.mod";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_HALF-6_WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PER_11P_HALF9_PACKETS_NUM = 500;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11P_HALF9_PACKETS_NUM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11P_HALF9_PACKETS_NUM;
		setting.unit  = "";
		setting.helpText = "Packet number of HALF-9 sent to DUT for PER testing.\r\nDefault value is 500";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_HALF-9_PACKETS_NUM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.PER_11P_HALF9_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HALF-9.mod");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11P_HALF9_WAVEFORM_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.PER_11P_HALF9_WAVEFORM_NAME;
		setting.unit  = "";
		setting.helpText = "Waveform name for HALF-9 data rate.\r\nDefault value is WiFi_HALF-9.mod";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_HALF-9_WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PER_11P_HALF12_PACKETS_NUM = 1000;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11P_HALF12_PACKETS_NUM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11P_HALF12_PACKETS_NUM;
		setting.unit  = "";
		setting.helpText = "Packet number of HALF-12 sent to DUT for PER testing.\r\nDefault value is 1000";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_HALF-12_PACKETS_NUM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.PER_11P_HALF12_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HALF-12.mod");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11P_HALF12_WAVEFORM_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.PER_11P_HALF12_WAVEFORM_NAME;
		setting.unit  = "";
		setting.helpText = "Waveform name for HALF-12 data rate.\r\nDefault value is WiFi_HALF-12.mod";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_HALF-12_WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PER_11P_HALF18_PACKETS_NUM = 1000;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11P_HALF18_PACKETS_NUM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11P_HALF18_PACKETS_NUM;
		setting.unit  = "";
		setting.helpText = "Packet number of HALF-18 sent to DUT for PER testing.\r\nDefault value is 1000";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_HALF-18_PACKETS_NUM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.PER_11P_HALF18_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HALF-18.mod");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11P_HALF18_WAVEFORM_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.PER_11P_HALF18_WAVEFORM_NAME;
		setting.unit  = "";
		setting.helpText = "Waveform name for HALF-18 data rate.\r\nDefault value is WiFi_HALF-18.mod";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_HALF-18_WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PER_11P_HALF24_PACKETS_NUM = 1000;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11P_HALF24_PACKETS_NUM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11P_HALF24_PACKETS_NUM;
		setting.unit  = "";
		setting.helpText = "Packet number of HALF-24 sent to DUT for PER testing.\r\nDefault value is 1000";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_HALF-24_PACKETS_NUM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.PER_11P_HALF24_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HALF-24.mod");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11P_HALF24_WAVEFORM_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.PER_11P_HALF24_WAVEFORM_NAME;
		setting.unit  = "";
		setting.helpText = "Waveform name for HALF-24 data rate.\r\nDefault value is WiFi_HALF-24.mod";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_HALF-24_WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PER_11P_HALF27_PACKETS_NUM = 1000;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11P_HALF27_PACKETS_NUM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11P_HALF27_PACKETS_NUM;
		setting.unit  = "";
		setting.helpText = "Packet number of HALF-27 sent to DUT for PER testing.\r\nDefault value is 1000";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_HALF-27_PACKETS_NUM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.PER_11P_HALF27_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HALF-27.mod");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11P_HALF27_WAVEFORM_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.PER_11P_HALF27_WAVEFORM_NAME;
		setting.unit  = "";
		setting.helpText = "Waveform name for HALF-27 data rate.\r\nDefault value is WiFi_HALF-27.mod";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_HALF-27_WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// 11p QUAR
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PER_11P_QUAR1_5_PACKETS_NUM = 200;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11P_QUAR1_5_PACKETS_NUM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11P_QUAR1_5_PACKETS_NUM;
		setting.unit  = "";
		setting.helpText = "Packet number of QUAR-1_5 sent to DUT for PER testing.\r\nDefault value is 200";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_QUAR-1_5_PACKETS_NUM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.PER_11P_QUAR1_5_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_QUAR-1_5.mod");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11P_QUAR1_5_WAVEFORM_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.PER_11P_QUAR1_5_WAVEFORM_NAME;
		setting.unit  = "";
		setting.helpText = "Waveform name for QUAR-1_5 data rate.\r\nDefault value is WiFi_QUAR-1_5.mod";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_QUAR-1_5_WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PER_11P_QUAR2_25_PACKETS_NUM = 500;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11P_QUAR2_25_PACKETS_NUM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11P_QUAR2_25_PACKETS_NUM;
		setting.unit  = "";
		setting.helpText = "Packet number of QUAR-2_25 sent to DUT for PER testing.\r\nDefault value is 500";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_QUAR-2_25_PACKETS_NUM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.PER_11P_QUAR2_25_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_QUAR-2_25.mod");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11P_QUAR2_25_WAVEFORM_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.PER_11P_QUAR2_25_WAVEFORM_NAME;
		setting.unit  = "";
		setting.helpText = "Waveform name for QUAR-2_25 data rate.\r\nDefault value is WiFi_QUAR-2_25.mod";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_QUAR-2_25_WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PER_11P_QUAR3_PACKETS_NUM = 500;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11P_QUAR3_PACKETS_NUM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11P_QUAR3_PACKETS_NUM;
		setting.unit  = "";
		setting.helpText = "Packet number of QUAR-3 sent to DUT for PER testing.\r\nDefault value is 500";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_QUAR-3_PACKETS_NUM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.PER_11P_QUAR3_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_QUAR-3.mod");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11P_QUAR3_WAVEFORM_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.PER_11P_QUAR3_WAVEFORM_NAME;
		setting.unit  = "";
		setting.helpText = "Waveform name for QUAR-3 data rate.\r\nDefault value is WiFi_QUAR-3.mod";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_QUAR-3_WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PER_11P_QUAR4_5_PACKETS_NUM = 500;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11P_QUAR4_5_PACKETS_NUM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11P_QUAR4_5_PACKETS_NUM;
		setting.unit  = "";
		setting.helpText = "Packet number of QUAR-4_5 sent to DUT for PER testing.\r\nDefault value is 500";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_QUAR-4_5_PACKETS_NUM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.PER_11P_QUAR4_5_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_QUAR-4_5.mod");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11P_QUAR4_5_WAVEFORM_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.PER_11P_QUAR4_5_WAVEFORM_NAME;
		setting.unit  = "";
		setting.helpText = "Waveform name for QUAR-4_5 data rate.\r\nDefault value is WiFi_QUAR-4_5.mod";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_QUAR-4_5_WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PER_11P_QUAR6_PACKETS_NUM = 1000;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11P_QUAR6_PACKETS_NUM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11P_QUAR6_PACKETS_NUM;
		setting.unit  = "";
		setting.helpText = "Packet number of QUAR-6 sent to DUT for PER testing.\r\nDefault value is 1000";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_QUAR-6_PACKETS_NUM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.PER_11P_QUAR6_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_QUAR-6.mod");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11P_QUAR6_WAVEFORM_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.PER_11P_QUAR6_WAVEFORM_NAME;
		setting.unit  = "";
		setting.helpText = "Waveform name for QUAR-6 data rate.\r\nDefault value is WiFi_QUAR-6.mod";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_QUAR-6_WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PER_11P_QUAR9_PACKETS_NUM = 1000;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11P_QUAR9_PACKETS_NUM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11P_QUAR9_PACKETS_NUM;
		setting.unit  = "";
		setting.helpText = "Packet number of QUAR-9 sent to DUT for PER testing.\r\nDefault value is 1000";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_QUAR-9_PACKETS_NUM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.PER_11P_QUAR9_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_QUAR-9.mod");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11P_QUAR9_WAVEFORM_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.PER_11P_QUAR9_WAVEFORM_NAME;
		setting.unit  = "";
		setting.helpText = "Waveform name for QUAR-9 data rate.\r\nDefault value is WiFi_QUAR-9.mod";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_QUAR-9_WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PER_11P_QUAR12_PACKETS_NUM = 1000;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11P_QUAR12_PACKETS_NUM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11P_QUAR12_PACKETS_NUM;
		setting.unit  = "";
		setting.helpText = "Packet number of QUAR-12 sent to DUT for PER testing.\r\nDefault value is 1000";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_QUAR-12_PACKETS_NUM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.PER_11P_QUAR12_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_QUAR-12.mod");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11P_QUAR12_WAVEFORM_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.PER_11P_QUAR12_WAVEFORM_NAME;
		setting.unit  = "";
		setting.helpText = "Waveform name for QUAR-12 data rate.\r\nDefault value is WiFi_QUAR-12.mod";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_QUAR-12_WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PER_11P_QUAR13_5_PACKETS_NUM = 1000;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11P_QUAR13_5_PACKETS_NUM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11P_QUAR13_5_PACKETS_NUM;
		setting.unit  = "";
		setting.helpText = "Packet number of QUAR-13_5 sent to DUT for PER testing.\r\nDefault value is 1000";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_QUAR-13_5_PACKETS_NUM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.PER_11P_QUAR13_5_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_QUAR-13_5.mod");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11P_QUAR13_5_WAVEFORM_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.PER_11P_QUAR13_5_WAVEFORM_NAME;
		setting.unit  = "";
		setting.helpText = "Waveform name for QUAR-13_5 data rate.\r\nDefault value is WiFi_QUAR-13_5.mod";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11P_QUAR-13_5_WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

#pragma region 	PER: HT20, mixed format, longGI

	/*****************************************************************
	* For backward compatibility,									 *			  
	*we use old waveform names for mixed, long guard interval format *
	******************************************************************/

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS0_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS0_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS0_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS0 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS0_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	
    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS0_WAVEFORM_NAME, MAX_BUFFER_SIZE,"WiFi_HT20_MCS0.mod");           
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS0_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS0_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS0 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS0.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS0_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS1_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS1_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS1_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS1 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS1_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS1_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS1.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS1_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS1_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS1 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS1.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS1_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS2_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS2_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS2_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS2 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS2_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS2_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS2.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS2_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS2_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS2 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS2.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS2_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS3_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS3_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS3_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS3 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS3_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS3_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS3.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS3_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS3_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS3 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS3.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS3_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS4_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS4_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS4_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS4 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS4_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS4_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS4.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS4_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS4_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS4 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS4.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS4_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS5_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS5_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS5_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS5 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS5_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS5_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS5.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS5_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS5_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS5 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS5.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS5_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS6_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS6_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS6_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS6 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS6_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS6_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS6.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS6_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS6_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS6 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS6.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS6_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS7_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS7_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS7_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS7 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS7_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS7_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS7.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS7_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS7_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS7 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS7.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS7_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS8_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS8_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS8_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS8 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS8_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS8_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS8.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS8_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS8_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS8 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS8.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS8_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS9_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS9_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS9_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS9 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS9_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS9_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS9.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS9_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS9_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS9 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS9.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS9_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS10_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS10_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS10_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS10 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS10_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS10_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS10.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS10_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS10_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS10 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS10.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS10_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS11_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS11_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS11_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS11 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS11_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS11_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS11.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS11_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS11_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS11 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS11.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS11_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS12_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS12_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS12_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS12 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS12_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS12_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS12.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS12_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS12_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS12 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS12.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS12_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS13_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS13_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS13_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS13 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS13_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS13_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS13.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS13_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS13_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS13 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS13.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS13_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS14_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS14_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS14_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS14 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS14_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS14_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS14.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS14_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS14_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS14 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS14.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS14_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS15_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS15_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS15_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS15 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS15_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS15_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS15.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS15_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS15_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS15 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS15.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS15_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS16_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS16_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS16_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS16 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS16_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS16_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS16.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS16_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS16_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS16 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS16.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS16_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS17_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS17_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS17_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS17 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS17_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS17_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS17.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS17_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS17_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS17 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS17.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS17_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS18_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS18_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS18_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS18 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS18_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS18_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS18.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS18_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS18_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS18 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS18.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS18_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS19_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS19_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS19_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS19 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS19_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS19_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS19.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS19_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS19_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS19 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS19.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS19_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS20_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS20_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS20_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS20 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS20_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS20_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS20.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS20_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS20_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS20 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS20.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS20_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS21_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS21_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS21_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS21 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS21_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS21_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS21.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS21_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS21_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS21 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS21.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS21_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS22_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS22_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS22_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS22 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS22_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS22_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS22.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS22_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS22_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS22 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS22.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS22_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS23_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS23_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS23_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS23 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS23_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS23_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS23.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS23_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS23_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS23 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS23.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS23_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS24_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS24_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS24_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS24 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS24_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS24_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS24.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS24_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS24_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS24 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS24.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS24_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS25_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS25_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS25_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS25 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS25_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS25_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS25.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS25_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS25_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS25 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS25.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS25_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS26_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS26_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS26_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS26 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS26_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS26_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS26.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS26_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS26_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS26 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS26.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS26_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS27_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS27_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS27_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS27 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS27_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS27_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS27.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS27_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS27_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS27 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS27.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS27_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS28_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS28_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS28_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS28 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS28_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS28_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS28.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS28_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS28_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS28 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS28.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS28_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS29_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS29_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS29_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS29 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS29_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS29_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS29.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS29_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS29_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS29 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS29.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS29_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS30_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS30_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS30_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS30 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS30_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS30_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS30.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS30_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS30_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS30 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS30.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS30_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS31_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS31_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS31_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS31 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS31_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS31_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS31.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS31_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_MCS31_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS31 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS31.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_MCS31_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
#pragma endregion

#pragma region PER: HT40, mixed format,longGI

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS0_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS0_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS0_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS0 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS0_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS0_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS0.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS0_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS0_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS0 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS0.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS0_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS1_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS1_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS1_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS1 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS1_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS1_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS1.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS1_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS1_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS1 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS1.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS1_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS2_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS2_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS2_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS2 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS2_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS2_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS2.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS2_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS2_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS2 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS2.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS2_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS3_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS3_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS3_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS3 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS3_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS3_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS3.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS3_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS3_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS3 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS3.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS3_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS4_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS4_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS4_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS4 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS4_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS4_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS4.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS4_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS4_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS4 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS4.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS4_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS5_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS5_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS5_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS5 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS5_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS5_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS5.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS5_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS5_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS5 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS5.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS5_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS6_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS6_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS6_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS6 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS6_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS6_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS6.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS6_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS6_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS6 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS6.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS6_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS7_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS7_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS7_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS7 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS7_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS7_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS7.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS7_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS7_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS7 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS7.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS7_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS8_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS8_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS8_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS8 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS8_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS8_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS8.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS8_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS8_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS8 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS8.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS8_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS9_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS9_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS9_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS9 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS9_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS9_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS9.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS9_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS9_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS9 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS9.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS9_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS10_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS10_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS10_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS10 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS10_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS10_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS10.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS10_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS10_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS10 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS10.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS10_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS11_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS11_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS11_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS11 (MIXED format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS11_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS11_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS11.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS11_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS11_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS11 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS11.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS11_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS12_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS12_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS12_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS12 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS12_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS12_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS12.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS12_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS12_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS12 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS12.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS12_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS13_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS13_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS13_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS13 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS13_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS13_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS13.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS13_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS13_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS13 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS13.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS13_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS14_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS14_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS14_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS14 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS14_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS14_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS14.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS14_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS14_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS14 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS14.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS14_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS15_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS15_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS15_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS15 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS15_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS15_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS15.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS15_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS15_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS15 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS15.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS15_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS16_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS16_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS16_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS16 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS16_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS16_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS16.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS16_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS16_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS16 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS16.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS16_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS17_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS17_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS17_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS17 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS17_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS17_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS17.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS17_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS17_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS17 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS17.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS17_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS18_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS18_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS18_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS18 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS18_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS18_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS18.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS18_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS18_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS18 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS18.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS18_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS19_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS19_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS19_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS19 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS19_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS19_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS19.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS19_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS19_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS19 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS19.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS19_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS20_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS20_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS20_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS20 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS20_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS20_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS20.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS20_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS20_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS20 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS20.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS20_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS21_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS21_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS21_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS21 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS21_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS21_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS21.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS21_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS21_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS21 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS21.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS21_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS22_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS22_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS22_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS22 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS22_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS22_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS22.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS22_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS22_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS22 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS22.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS22_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS23_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS23_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS23_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS23 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS23_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS23_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS23.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS23_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS23_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS23 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS23.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS23_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS24_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS24_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS24_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS24 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS24_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS24_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS24.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS24_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS24_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS24 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS24.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS24_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS25_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS25_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS25_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS25 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS25_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS25_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS25.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS25_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS25_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS25 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS25.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS25_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS26_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS26_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS26_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS26 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS26_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS26_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS26.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS26_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS26_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS26 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS26.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS26_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS27_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS27_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS27_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS27 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS27_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS27_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS27.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS27_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS27_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS27 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS27.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS27_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS28_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS28_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS28_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS28 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS28_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS28_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS28.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS28_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS28_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS28 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS28.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS28_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS29_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS29_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS29_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS29 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS29_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS29_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS29.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS29_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS29_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS29 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS29.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS29_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS30_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS30_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS30_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS30 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS30_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS30_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS30.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS30_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS30_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS30 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS30.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS30_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS31_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS31_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS31_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS31 (mixed format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS31_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS31_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS31.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS31_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_MCS31_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS31 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS31.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_MCS31_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
	}
#pragma endregion

#pragma region PER: HT20, GREENFIELD format, longGI


	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS0_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS0_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS0_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS0 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS0_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS0.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS0 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS0.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS1_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS1_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS1_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS1 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS1_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS1.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS1 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS1.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS2_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS2_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS2_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS2 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS2_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS2.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS2 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS2.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS3_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS3_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS3_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS3 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS3_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS3.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS3 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS3.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS4_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS4_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS4_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS4 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS4_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS4.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS4 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS4.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS5_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS5_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS5_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS5 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS5_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS5.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS5 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS5.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS6_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS6_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS6_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS6 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS6_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS6.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS6 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS6.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS7_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS7_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS7_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS7 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS7_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS7.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS7 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS7.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS8_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS8_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS8_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS8 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS8_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS8.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS8 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS8.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS9_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS9_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS9_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS9 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS9_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS9.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS9 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS9.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS10_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS10_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS10_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS10 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS10_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS10.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS10 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS10.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS11_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS11_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS11_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS11 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS11_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS11.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS11 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS11.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS12_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS12_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS12_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS12 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS12_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS12.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS12 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS12.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS13_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS13_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS13_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS13 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS13_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS13.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS13 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS13.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS14_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS14_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS14_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS14 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS14_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS14.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS14 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS14.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS15_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS15_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS15_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS15 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS15_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS15.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS15 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS15.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS16_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS16_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS16_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS16 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS16_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS16.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS16 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS16.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS17_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS17_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS17_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS17 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS17_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS17.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS17 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS17.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS18_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS18_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS18_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS18 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS18_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS18.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS18 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS18.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS19_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS19_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS19_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS19 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS19_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS19.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS19 (GREENFIELD_L_GI format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS19.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS20_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS20_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS20_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS20 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS20_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS20.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS20 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS20.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS21_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS21_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS21_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS21 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS21_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS21.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS21 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS21.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS22_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS22_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS22_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS22 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS22_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS22.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS22 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS22.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS23_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS23_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS23_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS23 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS23_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS23.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS23 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS23.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS24_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS24_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS24_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS24 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS24_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS24.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS24 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS24.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS25_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS25_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS25_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS25 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS25_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS25.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS25 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS25.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS26_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS26_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS26_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS26 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS26_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS26.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS26 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS26.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS27_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS27_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS27_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS27 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS27_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS27.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS27 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS27.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS28_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS28_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS28_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS28 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS28_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS28.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS28 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS28.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS29_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS29_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS29_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS29 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS29_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS29.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS29 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS29.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS30_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS30_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS30_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS30 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS30_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS30.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS30 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS30.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS31_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS31_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS31_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS31 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS31_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS31.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS31 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS31.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
#pragma endregion

#pragma region  PER:  HT40, GREENFIELD format, longGI
	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS0_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS0_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS0_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS0 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS0_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS0.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS0 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS0.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS1_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS1_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS1_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS1 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS1_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS1.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS1 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS1.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS2_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS2_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS2_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS2 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS2_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS2.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS2 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS2.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS3_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS3_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS3_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS3 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS3_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS3.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS3 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS3.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS4_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS4_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS4_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS4 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS4_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS4.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS4 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS4.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS5_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS5_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS5_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS5 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS5_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS5.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS5 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS5.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS6_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS6_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS6_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS6 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS6_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS6.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS6 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS6.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS7_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS7_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS7_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS7 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS7_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS7.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS7 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS7.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS8_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS8_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS8_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS8 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS8_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS8.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS8 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS8.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS9_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS9_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS9_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS9 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS9_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS9.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS9 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS9.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS10_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS10_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS10_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS10 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS10_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS10.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS10 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS10.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS11_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS11_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS11_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS11 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS11_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS11.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS11 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS11.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS12_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS12_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS12_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS12 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS12_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS12.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS12 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS12.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS13_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS13_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS13_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS13 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS13_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS13.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS13 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS13.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS14_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS14_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS14_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS14 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS14_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS14.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS14 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS14.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS15_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS15_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS15_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS15 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS15_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS15.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS15 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS15.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS16_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS16_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS16_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS16 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS16_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS16.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS16 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS16.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS17_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS17_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS17_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS17 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS17_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS17.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS17 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS17.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS18_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS18_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS18_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS18 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS18_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS18.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS18 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS18.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS19_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS19_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS19_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS19 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS19_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS19.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS19 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS19.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS20_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS20_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS20_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS20 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS20_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS20.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS20 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS20.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS21_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS21_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS21_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS21 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS21_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS21.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS21 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS21.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS22_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS22_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS22_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS22 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS22_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS22.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS22 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS22.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS23_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS23_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS23_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS23 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS23_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS23.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS23 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS23.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS24_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS24_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS24_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS24 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS24_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS24.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS24 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS24.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS25_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS25_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS25_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS25 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS25_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS25.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS25 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS25.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS26_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS26_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS26_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS26 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS26_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS26.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS26 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS26.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS27_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS27_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS27_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS27 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS27_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS27.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS27 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS27.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS28_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS28_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS28_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS28 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS28_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS28.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS28 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS28.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS29_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS29_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS29_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS29 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS29_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS29.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS29 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS29.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS30_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS30_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS30_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS30 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS30_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS30.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS30 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS30.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS31_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS31_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS31_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS31 (greenfield format, long guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS31_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS31.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS31 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS31.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
#pragma endregion

#pragma region	PER:  HT20,mixed format, shortGI

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS0_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS0_PACKETS_NUM))    // Type_Checking
	{
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS0_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS0 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS0_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS0_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS0.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS0_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS0_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS0 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS0.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS0_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS1_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS1_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS1_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS1 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS1_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS1_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS1.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS1_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS1_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS1 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS1.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS1_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS2_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS2_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS2_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS2 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS2_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS2_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS2.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS2_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS2_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS2 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS2.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS2_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS3_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS3_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS3_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS3 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS3_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS3_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS3.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS3_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS3_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS3 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS3.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS3_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS4_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS4_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS4_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS4 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS4_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS4_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS4.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS4_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS4_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS4 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS4.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS4_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS5_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS5_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS5_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS5 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS5_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS5_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS5.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS5_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS5_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS5 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS5.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS5_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS6_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS6_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS6_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS6 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS6_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS6_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS6.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS6_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS6_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS6 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS6.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS6_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS7_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS7_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS7_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS7 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS7_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS7_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS7.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS7_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS7_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS7 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS7.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS7_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS8_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS8_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS8_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS8 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS8_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS8_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS8.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS8_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS8_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS8 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS8.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS8_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS9_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS9_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS9_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS9 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS9_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS9_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS9.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS9_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS9_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS9 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS9.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS9_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS10_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS10_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS10_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS10 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS10_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS10_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS10.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS10_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS10_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS10 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS10.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS10_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS11_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS11_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS11_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS11 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS11_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS11_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS11.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS11_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS11_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS11 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS11.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS11_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS12_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS12_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS12_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS12 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS12_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS12_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS12.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS12_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS12_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS12 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS12.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS12_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS13_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS13_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS13_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS13 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS13_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS13_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS13.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS13_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS13_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS13 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS13.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS13_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS14_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS14_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS14_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS14 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS14_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS14_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS14.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS14_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS14_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS14 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS14.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS14_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS15_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS15_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS15_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS15 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS15_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS15_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS15.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS15_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS15_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS15 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS15.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS15_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS16_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS16_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS16_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS16 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS16_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS16_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS16.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS16_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS16_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS16 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS16.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS16_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS17_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS17_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS17_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS17 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS17_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS17_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS17.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS17_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS17_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS17 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS17.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS17_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS18_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS18_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS18_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS18 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS18_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS18_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS18.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS18_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS18_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS18 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS18.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS18_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS19_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS19_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS19_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS19 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS19_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS19_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS19.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS19_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS19_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS19 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS19.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS19_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS20_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS20_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS20_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS20 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS20_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS20_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS20.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS20_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS20_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS20 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS20.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS20_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS21_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS21_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS21_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS21 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS21_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS21_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS21.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS21_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS21_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS21 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS21.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS21_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS22_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS22_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS22_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS22 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS22_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS22_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS22.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS22_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS22_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS22 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS22.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS22_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS23_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS23_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS23_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS23 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS23_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS23_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS23.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS23_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS23_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS23 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS23.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS23_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS24_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS24_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS24_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS24 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS24_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS24_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS24.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS24_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS24_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS24 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS24.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS24_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS25_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS25_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS25_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS25 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS25_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS25_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS25.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS25_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS25_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS25 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS25.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS25_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS26_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS26_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS26_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS26 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS26_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS26_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS26.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS26_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS26_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS26 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS26.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS26_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS27_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS27_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS27_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS27 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS27_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS27_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS27.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS27_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS27_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS27 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS27.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS27_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS28_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS28_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS28_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS28 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS28_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS28_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS28.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS28_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS28_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS28 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS28.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS28_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS29_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS29_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS29_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS29 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS29_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS29_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS29.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS29_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS29_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS29 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS29.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS29_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS30_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS30_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS30_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS30 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS30_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS30_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS30.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS30_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS30_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS30 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS30.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS30_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS31_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS31_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS31_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS31 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS31_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS31_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS31.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS31_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_MIXED_S_GI_MCS31_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS31 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS31.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_MIXED_S_GI_MCS31_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

#pragma endregion

#pragma region PER:  HT40,mixed format, shortGI

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS0_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS0_PACKETS_NUM))    // Type_Checking
	{
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS0_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS0 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS0_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	
    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS0_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS0.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS0_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS0_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS0 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS0.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS0_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS1_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS1_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS1_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS1 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS1_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS1_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS1.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS1_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS1_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS1 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS1.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS1_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS2_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS2_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS2_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS2 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS2_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS2_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS2.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS2_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS2_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS2 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS2.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS2_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS3_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS3_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS3_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS3 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS3_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS3_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS3.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS3_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS3_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS3 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS3.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS3_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS4_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS4_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS4_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS4 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS4_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS4_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS4.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS4_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS4_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS4 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS4.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS4_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS5_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS5_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS5_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS5 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS5_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS5_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS5.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS5_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS5_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS5 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS5.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS5_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS6_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS6_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS6_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS6 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS6_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS6_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS6.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS6_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS6_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS6 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS6.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS6_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS7_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS7_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS7_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS7 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS7_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS7_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS7.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS7_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS7_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS7 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS7.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS7_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS8_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS8_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS8_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS8 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS8_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS8_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS8.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS8_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS8_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS8 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS8.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS8_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS9_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS9_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS9_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS9 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS9_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS9_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS9.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS9_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS9_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS9 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS9.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS9_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS10_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS10_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS10_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS10 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS10_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS10_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS10.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS10_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS10_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS10 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS10.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS10_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS11_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS11_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS11_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS11 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS11_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS11_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS11.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS11_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS11_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS11 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS11.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS11_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS12_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS12_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS12_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS12 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS12_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS12_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS12.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS12_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS12_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS12 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS12.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS12_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS13_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS13_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS13_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS13 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS13_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS13_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS13.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS13_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS13_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS13 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS13.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS13_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS14_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS14_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS14_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS14 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS14_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS14_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS14.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS14_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS14_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS14 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS14.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS14_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS15_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS15_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS15_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS15 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS15_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS15_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS15.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS15_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS15_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS15 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS15.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS15_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS16_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS16_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS16_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS16 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS16_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS16_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS16.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS16_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS16_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS16 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS16.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS16_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS17_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS17_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS17_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS17 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS17_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS17_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS17.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS17_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS17_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS17 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS17.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS17_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS18_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS18_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS18_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS18 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS18_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS18_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS18.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS18_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS18_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS18 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS18.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS18_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS19_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS19_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS19_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS19 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS19_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS19_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS19.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS19_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS19_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS19 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS19.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS19_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS20_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS20_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS20_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS20 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS20_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS20_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS20.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS20_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS20_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS20 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS20.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS20_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS21_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS21_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS21_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS21 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS21_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS21_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS21.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS21_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS21_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS21 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS21.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS21_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS22_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS22_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS22_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS22 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS22_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS22_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS22.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS22_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS22_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS22 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS22.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS22_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS23_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS23_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS23_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS23 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS23_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS23_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS23.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS23_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS23_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS23 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS23.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS23_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS24_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS24_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS24_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS24 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS24_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS24_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS24.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS24_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS24_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS24 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS24.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS24_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS25_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS25_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS25_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS25 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS25_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS25_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS25.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS25_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS25_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS25 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS25.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS25_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS26_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS26_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS26_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS26 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS26_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS26_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS26.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS26_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS26_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS26 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS26.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS26_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS27_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS27_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS27_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS27 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS27_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS27_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS27.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS27_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS27_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS27 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS27.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS27_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS28_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS28_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS28_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS28 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS28_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS28_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS28.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS28_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS28_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS28 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS28.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS28_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS29_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS29_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS29_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS29 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS29_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS29_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS29.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS29_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS29_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS29 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS29.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS29_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS30_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS30_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS30_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS30 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS30_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS30_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS30.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS30_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS30_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS30 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS30.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS30_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS31_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS31_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS31_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS31 (mixed format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS31_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS31_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS31.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS31_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_MIXED_S_GI_MCS31_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS31 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS31.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_MIXED_S_GI_MCS31_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
#pragma endregion

#pragma region	 PER: HT20, GREENFIELD format, shortGI

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS0_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS0_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS0_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS0 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS0_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS0.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS0 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS0.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS1_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS1_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS1_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS1 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS1_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS1.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS1 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS1.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS2_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS2_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS2_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS2 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS2_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS2.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS2 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS2.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS3_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS3_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS3_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS3 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS3_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS3.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS3 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS3.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS4_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS4_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS4_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS4 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS4_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS4.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS4 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS4.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS5_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS5_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS5_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS5 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS5_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS5.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS5 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS5.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS6_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS6_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS6_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS6 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS6_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS6.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS6 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS6.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS7_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS7_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS7_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS7 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS7_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS7.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS7 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS7.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS8_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS8_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS8_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS8 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS8_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS8.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS8 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS8.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS9_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS9_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS9_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS9 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS9_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS9.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS9 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS9.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS10_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS10_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS10_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS10 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS10_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS10.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS10 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS10.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS11_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS11_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS11_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS11 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS11_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS11.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS11 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS11.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS12_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS12_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS12_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS12 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS12_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS12.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS12 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS12.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS13_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS13_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS13_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS13 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS13_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS13.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS13 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS13.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS14_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS14_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS14_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS14 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS14_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS14.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS14 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS14.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS15_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS15_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS15_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS15 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS15_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS15.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS15 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS15.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS16_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS16_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS16_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS16 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS16_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS16.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS16 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS16.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS17_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS17_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS17_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS17 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS17_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS17.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS17 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS17.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS18_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS18_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS18_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS18 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS18_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS18.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS18 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS18.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS19_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS19_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS19_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS19 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS19_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS19.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS19 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS19.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS20_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS20_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS20_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS20 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS20_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS20.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS20 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS20.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS21_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS21_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS21_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS21 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS21_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS21.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS21 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS21.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS22_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS22_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS22_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS22 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS22_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS22.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS22 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS22.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS23_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS23_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS23_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS23 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS23_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS23.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS23 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS23.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS24_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS24_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS24_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS24 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS24_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS24.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS24 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS24.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS25_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS25_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS25_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS25 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS25_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS25.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS25 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS25.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS26_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS26_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS26_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS26 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS26_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS26.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS26 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS26.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS27_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS27_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS27_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS27 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS27_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS27.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS27 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS27.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS28_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS28_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS28_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS28 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS28_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS28.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS28 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS28.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS29_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS29_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS29_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS29 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS29_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS29.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS29 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS29.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS30_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS30_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS30_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS30 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS30_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS30.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS30 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS30.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS31_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS31_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS31_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT20 MCS31 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS31_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS31.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT20_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT20 MCS31 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS31.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT20_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
	}
#pragma endregion

#pragma region    PER:HT40, GREENFIELD format, shortGI

	setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS0_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS0_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS0_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS0 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS0_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS0.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS0 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS0.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS1_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS1_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS1_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS1 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS1_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS1.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS1 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS1.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS2_PACKETS_NUM = 200;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS2_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS2_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS2 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 200";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS2_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS2I.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS2 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS2.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS3_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS3_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS3_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS3 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS3_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS3.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS3 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS3.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS4_PACKETS_NUM = 500;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS4_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS4_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS4 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 500";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS4_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS4.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS4 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS4.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS5_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS5_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS5_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS5 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS5_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS5.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS5 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS5.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS6_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS6_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS6_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS6 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS6_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS6.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS6 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS6.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS7_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS7_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS7_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS7 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS7_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS7.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS7 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS7.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS8_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS8_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS8_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS8 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS8_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS8.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS8 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS8.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS9_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS9_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS9_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS9 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS9_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS9.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS9 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS9.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS10_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS10_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS10_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS10 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS10_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS10.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS10 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS10.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS11_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS11_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS11_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS11 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS11_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS11.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS11 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS11.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS12_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS12_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS12_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS12 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS12_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS12.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS12 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS12.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS13_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS13_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS13_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS13 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS13_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS13.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS13 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS13.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS14_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS14_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS14_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS14 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS14_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS14.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS14 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS14mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS15_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS15_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS15_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS15 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS15_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS15.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS15 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS15.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS16_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS16_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS16_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS16 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS16_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS16.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS16 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS16.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS17_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS17_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS17_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS17 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS17_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS17.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS17 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS17.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS18_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS18_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS18_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS18 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS18_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS18.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS18 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS18.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS19_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS19_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS19_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS19 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS19_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS19.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS19 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS19.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS20_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS20_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS20_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS20 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS20_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS20.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS20 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS20.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS21_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS21_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS21_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS21 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS21_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS21.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS21 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS21.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS22_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS22_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS22_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS22 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS22_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS22.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS22 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS22.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS23_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS23_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS23_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS23 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS23_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS23.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS23 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS23.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS24_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS24_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS24_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS24 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS24_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS24.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS24 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS24.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS25_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS25_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS25_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS25 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS25_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS25.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS25 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS25.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS26_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS26_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS26_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS26 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS26_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS26.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS26 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS26.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS27_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS27_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS27_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS27 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS27_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS27.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS27 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS27.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS28_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS28_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS28_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS28 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS28_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS28.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS28 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS28.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS29_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS29_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS29_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS29 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS29_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS29.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS29 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS29.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS30_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS30_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS30_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS30 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS30_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS30.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS30 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS30.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS31_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS31_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS31_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number of HT40 MCS31 (greenfield format, short guard interval) sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS31_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS31.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.PER_11N_HT40_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for HT40 MCS31 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS31.mod";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_11N_HT40_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
	}
#pragma endregion

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PM_AVERAGE = 3;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PM_AVERAGE))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PM_AVERAGE;
        setting.unit  = "";
        setting.helpText = "Averaging times used for Power measurement.\r\nDefault value is 3";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PM_AVERAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.VSA_TRIGGER_TYPE = 6;      // 6: IQV_TRIG_TYPE_IF2_NO_CAL
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.VSA_TRIGGER_TYPE))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.VSA_TRIGGER_TYPE;
        setting.unit  = "";
        setting.helpText = "Trigger types used for capturing.\r\nFree Run: 1\r\nExternal Trigger: 2\r\nSignal Trigger: 6\r\nSignal Trigger for IQ2010: 13";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_TRIGGER_TYPE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB = 3;
    if (sizeof(double)==sizeof(g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB;
        setting.unit  = "dB";
        setting.helpText  = "IQTester VSA amplitude setting tolerance. -3 < Default < +3 dB.";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_AMPLITUDE_TOLERANCE_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB = -25;
    if (sizeof(double)==sizeof(g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB;
        setting.unit  = "dB";
        setting.helpText  = "Signal trigger level";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_TRIGGER_LEVEL_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_DOUBLE;
	g_WiFiGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US = 3;
    if (sizeof(double)==sizeof(g_WiFiGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US;
        setting.unit  = "us";
        setting.helpText  = "IQTester VSA signal pre-trigger time setting used for signal capture.";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_PRE_TRIGGER_TIME_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.VSA_TRIGGER_TIMEOUT_SEC = 1;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.VSA_TRIGGER_TIMEOUT_SEC))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.VSA_TRIGGER_TIMEOUT_SEC;
        setting.unit  = "sec";
        setting.helpText  = "IQTester VSA signal trigger timeout(sec) setting used for signal capture.";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_TRIGGER_TIMEOUT_SEC", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.VSA_PORT = PORT_LEFT;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.VSA_PORT))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.VSA_PORT;
        setting.unit  = "";
        setting.helpText  = "VSA RF port\r\n2 for RF1(LEFT) and 3 for RF2(RIGHT)";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_PORT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.VSG_PORT = PORT_LEFT;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.VSG_PORT))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.VSG_PORT;
        setting.unit  = "";
        setting.helpText  = "VSG RF port\r\n2 for RF1(LEFT) and 3 for RF2(RIGHT)";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSG_PORT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
    
    setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ON_FAILED = 1;	
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ON_FAILED))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ON_FAILED;
        setting.unit  = "";
        setting.helpText  = "A flag that to save sig file when capture failed, 0: OFF, 1: ON, Default is 1.";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_SAVE_CAPTURE_ON_FAILED", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS = 0;	
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS;
        setting.unit  = "";
        setting.helpText  = "A flag that to save sig file, always, 0: OFF, 1: ON, Default is 0.";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_SAVE_CAPTURE_ALWAYS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
    
    setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.DUT_KEEP_TRANSMIT = 1;	
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.DUT_KEEP_TRANSMIT))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.DUT_KEEP_TRANSMIT;
        setting.unit  = "";
        setting.helpText  = "A flag that to let Dut keep Tx until the configuration changed, 0: OFF, 1: ON, Default=1.";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("DUT_KEEP_TRANSMIT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.DUT_TX_SETTLE_TIME_MS = 0;	
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.DUT_TX_SETTLE_TIME_MS))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.DUT_TX_SETTLE_TIME_MS;
        setting.unit  = "ms";
        setting.helpText  = "A delay time for DUT (TX) settle, Default = 0(ms).";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("DUT_TX_SETTLE_TIME_MS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.DUT_RX_SETTLE_TIME_MS = 0;	
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.DUT_RX_SETTLE_TIME_MS))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.DUT_RX_SETTLE_TIME_MS;
        setting.unit  = "ms";
        setting.helpText  = "A delay time for DUT (RX) settle, Default = 0(ms).";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("DUT_RX_SETTLE_TIME_MS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    // [802.11b] Parameters
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.ANALYSIS_11B_EQ_TAPS = 1;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.ANALYSIS_11B_EQ_TAPS))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.ANALYSIS_11B_EQ_TAPS;
        setting.unit  = "";
        setting.helpText = "Number of equalizer taps for 802.11b analysis\r\nOptions are 1,5,7,9\r\nDefault value is 1";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11B_EQ_TAPS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.ANALYSIS_11B_DC_REMOVE_FLAG = 0;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.ANALYSIS_11B_DC_REMOVE_FLAG))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.ANALYSIS_11B_DC_REMOVE_FLAG;
        setting.unit  = "";
        setting.helpText = "DC removal for 802.11b analysis\r\nOptions are 0,1\r\nDefault value is 0";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11B_DC_REMOVE_FLAG", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.ANALYSIS_11B_METHOD_11B = 1;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.ANALYSIS_11B_METHOD_11B))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.ANALYSIS_11B_METHOD_11B;
        setting.unit  = "";
        setting.helpText = "802.11b method\r\n1: Use 11b standard TX accuracy; 2:Use 11b RMS error vector\r\nDefault value is 1";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11B_METHOD_11B", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
	
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.ANALYSIS_11B_FIXED_01_DATA_SEQUENCE = 0;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.ANALYSIS_11B_FIXED_01_DATA_SEQUENCE))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.ANALYSIS_11B_FIXED_01_DATA_SEQUENCE;
        setting.unit  = "";
        setting.helpText = "Enable or disable 802.11b (0101) fixed data sequence.\r\n0: Disable; 1: Enable\r\nDefault value is 0";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11B_FIXED_01_DATA_SEQUENCE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    // [802.11a/g] Parameters
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.ANALYSIS_11AG_PH_CORR_MODE = 2;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.ANALYSIS_11AG_PH_CORR_MODE))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.ANALYSIS_11AG_PH_CORR_MODE;
        setting.unit  = "";
        setting.helpText = "Phase Correction Mode for 802.11 a/g analysis\r\n1:Phase Correction Off; 2:Symbol-by-symbol Correction; 3:Moving Average(10 symbols) Correction\r\nDefault value is 2";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11AG_PH_CORR_MODE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.ANALYSIS_11AG_CH_ESTIMATE = 1;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.ANALYSIS_11AG_CH_ESTIMATE))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.ANALYSIS_11AG_CH_ESTIMATE;
        setting.unit  = "";
        setting.helpText = "Channel Estimate for 802.11 a/g analysis\r\n1:Long Training Sequence; 2:2nd Order Polyfit; 3:Full Packet\r\nDefault value is 1";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11AG_CH_ESTIMATE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR = 2;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR;
        setting.unit  = "";
        setting.helpText = "Symbol Timing Correction for 802.11 a/g analysis\r\n1:Off; 2:On\r\nDefault value is 2";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11AG_SYM_TIM_CORR", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.ANALYSIS_11AG_FREQ_SYNC = 2;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.ANALYSIS_11AG_FREQ_SYNC))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.ANALYSIS_11AG_FREQ_SYNC;
        setting.unit  = "";
        setting.helpText = "Frequency Sync. Mode for 802.11 a/g analysis\r\n1:Short Training Symbol; 2:Long Training Symbol; 3:Full Data Packet\r\nDefault value is 2";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11AG_FREQ_SYNC", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.ANALYSIS_11AG_AMPL_TRACK = 1;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.ANALYSIS_11AG_AMPL_TRACK))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.ANALYSIS_11AG_AMPL_TRACK;
        setting.unit  = "";
        setting.helpText = "Amplitude Tracking for 802.11 a/g analysis\r\n1:Off; 2:On\r\nDefault value is 1";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11AG_AMPL_TRACK", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


    // [MIMO] Parameters
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.ANALYSIS_11N_PHASE_CORR = 1;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.ANALYSIS_11N_PHASE_CORR))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.ANALYSIS_11N_PHASE_CORR;
        setting.unit  = "";
        setting.helpText = "Phase Correction for 802.11n analysis\r\n0:Off; 1:On\r\nDefault value is 1";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11N_PHASE_CORR", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.ANALYSIS_11N_SYM_TIMING_CORR = 1;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.ANALYSIS_11N_SYM_TIMING_CORR))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.ANALYSIS_11N_SYM_TIMING_CORR;
        setting.unit  = "";
        setting.helpText = "Symbol Timing Correction for 802.11n analysis\r\n0:Off; 1:On\r\nDefault value is 1";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11N_SYM_TIMING_CORR", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.ANALYSIS_11N_AMPLITUDE_TRACKING = 0;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.ANALYSIS_11N_AMPLITUDE_TRACKING))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.ANALYSIS_11N_AMPLITUDE_TRACKING;
        setting.unit  = "";
        setting.helpText = "Amplitude Tracking for 802.11n analysis\r\n0:Off; 1:On\r\nDefault value is 0";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11N_AMPLITUDE_TRACKING", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.ANALYSIS_11N_DECODE_PSDU = 0;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.ANALYSIS_11N_DECODE_PSDU))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.ANALYSIS_11N_DECODE_PSDU;
        setting.unit  = "";
        setting.helpText = "Decode PSDU for 802.11n analysis\r\n0:Off; 1:On\r\nDefault value is 0";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11N_DECODE_PSDU", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST = 0;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST;
        setting.unit  = "";
        setting.helpText = "Channel Estimate over Full Packet for 802.11n analysis\r\n0:Off; 1:On\r\nDefault value is 0";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11N_FULL_PACKET_CHANNEL_EST", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.ANALYSIS_11N_FREQUENCY_CORRELATION = 2;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.ANALYSIS_11N_FREQUENCY_CORRELATION))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.ANALYSIS_11N_FREQUENCY_CORRELATION;
        setting.unit  = "";
		setting.helpText = "Frequency correction on short and long legacy training fields for 802.11n analysis\r\n 2=frequency correction on short and long legacy training fields; 3=frequency correction based on full packet; 4=frequency correction on signal fields (legacy and HT) in addition to short and long training fields;\r\nDefault value is 2";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("ANALYSIS_11N_FREQUENCY_CORRELATION", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


    //setting.type = WIFI_SETTING_TYPE_INTEGER;
    //g_WiFiGlobalSettingParam.IQ_PM_METHOD = 0;
    //if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.IQ_PM_METHOD))    // Type_Checking
    //{
    //    setting.value = (void*)&g_WiFiGlobalSettingParam.IQ_PM_METHOD;
    //    setting.unit  = "";
    //    setting.helpText = "";
    //    g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("IQ_PM_METHOD", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PM_IF_FREQ_SHIFT_MHZ = 0;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PM_IF_FREQ_SHIFT_MHZ))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PM_IF_FREQ_SHIFT_MHZ;
        setting.unit  = "MHz";
        setting.helpText = "VSA center frequency shift in MHz in the case where DUT has a strong LO leakage.\r\nDefault value is 0";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PM_IF_FREQ_SHIFT_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PM_DSSS_SAMPLE_INTERVAL_US = 100;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PM_DSSS_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PM_DSSS_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Required capture time when measuring DSSS power in trigger mode.\r\nDefault value is 100 us.";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PM_DSSS_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.PM_OFDM_SAMPLE_INTERVAL_US = 20;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PM_OFDM_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PM_OFDM_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Required capture time when measuring OFDM power in trigger mode.\r\nDefault value is 20 us.";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PM_OFDM_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PM_HALF_SAMPLE_INTERVAL_US = 40;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PM_HALF_SAMPLE_INTERVAL_US))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PM_HALF_SAMPLE_INTERVAL_US;
		setting.unit  = "us";
		setting.helpText = "Required capture time when measuring 11p HALF power in trigger mode.\r\nDefault value is 40 us.";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PM_HALF_SAMPLE_INTERVAL_US", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PM_QUAR_SAMPLE_INTERVAL_US = 80;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PM_QUAR_SAMPLE_INTERVAL_US))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.PM_QUAR_SAMPLE_INTERVAL_US;
		setting.unit  = "us";
		setting.helpText = "Required capture time when measuring 11p QUAR power in trigger mode.\r\nDefault value is 80 us.";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PM_QUAR_SAMPLE_INTERVAL_US", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.SPECTRUM_DSSS_SAMPLE_INTERVAL_US = 286;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.SPECTRUM_DSSS_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.SPECTRUM_DSSS_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Required capture time when measuring DSSS spectrum in trigger mode.\r\nDefault value is 286 us.";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("SPECTRUM_DSSS_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.SPECTRUM_OFDM_SAMPLE_INTERVAL_US = 95;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.SPECTRUM_OFDM_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.SPECTRUM_OFDM_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Required capture time when measuring OFDM spectrum in trigger mode.\r\nDefault value is 95 us.";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("SPECTRUM_OFDM_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.SPECTRUM_HALF_SAMPLE_INTERVAL_US = 187;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.SPECTRUM_HALF_SAMPLE_INTERVAL_US))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.SPECTRUM_HALF_SAMPLE_INTERVAL_US;
		setting.unit  = "us";
		setting.helpText = "Required capture time when measuring 11p HALF spectrum in trigger mode.\r\nDefault value is 187 us.";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("SPECTRUM_HALF_SAMPLE_INTERVAL_US", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.SPECTRUM_QUAR_SAMPLE_INTERVAL_US = 371;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.SPECTRUM_QUAR_SAMPLE_INTERVAL_US))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.SPECTRUM_QUAR_SAMPLE_INTERVAL_US;
		setting.unit  = "us";
		setting.helpText = "Required capture time when measuring 11p QUAR spectrum in trigger mode.\r\nDefault value is 371 us.";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("SPECTRUM_QUAR_SAMPLE_INTERVAL_US", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.MASK_FFT_AVERAGE = 1;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.MASK_FFT_AVERAGE))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.MASK_FFT_AVERAGE;
        setting.unit  = "";
        setting.helpText = "Required minumum number of Mask measurements.\r\nDefalut value is 1";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("MASK_FFT_AVERAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.MASK_DSSS_SAMPLE_INTERVAL_US = 286;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.MASK_DSSS_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.MASK_DSSS_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Required capture time when measuring DSSS MASK in trigger mode.\r\nDefault value is 286 us.";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("MASK_DSSS_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.MASK_OFDM_SAMPLE_INTERVAL_US = 95;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.MASK_OFDM_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.MASK_OFDM_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Required capture time when measuring OFDM MASK in trigger mode.\r\nDefault value is 95 us.";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("MASK_OFDM_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.MASK_HALF_SAMPLE_INTERVAL_US = 187;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.MASK_HALF_SAMPLE_INTERVAL_US))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.MASK_HALF_SAMPLE_INTERVAL_US;
		setting.unit  = "us";
		setting.helpText = "Required capture time when measuring 11p HALF MASK in trigger mode.\r\nDefault value is 187 us.";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("MASK_HALF_SAMPLE_INTERVAL_US", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.MASK_QUAR_SAMPLE_INTERVAL_US = 371;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.MASK_QUAR_SAMPLE_INTERVAL_US))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.MASK_QUAR_SAMPLE_INTERVAL_US;
		setting.unit  = "us";
		setting.helpText = "Required capture time when measuring 11p QUAR MASK in trigger mode.\r\nDefault value is 371 us.";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("MASK_QUAR_SAMPLE_INTERVAL_US", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.EVM_AVERAGE = 3;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.EVM_AVERAGE))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.EVM_AVERAGE;
        setting.unit  = "";
        setting.helpText = "Required minumum number of EVM measurements.\r\nDefalut value is 3";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_AVERAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.EVM_SYMBOL_NUM = 18;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.EVM_SYMBOL_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.EVM_SYMBOL_NUM;
        setting.unit  = "";
        setting.helpText = "Number of symbols used for EVM measurement (not used)";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_SYMBOL_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11B_L_US = 286;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11B_L_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11B_L_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring 11b (long preamble) EVM.\r\nDefault value is 286 us";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_CAPTURE_TIME_11B_L_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11B_S_US = 190;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11B_S_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11B_S_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring 11b (short preamble) EVM.\r\nDefault value is 190 us";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_CAPTURE_TIME_11B_S_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11AG_US = 95;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11AG_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11AG_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring 11ag EVM.\r\nDefault value is 95 us";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_CAPTURE_TIME_11AG_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11P_HALF_US = 187;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11P_HALF_US))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11P_HALF_US;
		setting.unit  = "us";
		setting.helpText = "Capture time for measuring 11p HALF EVM.\r\nDefault value is 187 us";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_CAPTURE_TIME_11P_HALF_US", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_INTEGER;

	g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11P_QUAR_US = 371;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11P_QUAR_US))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11P_QUAR_US;
		setting.unit  = "us";
		setting.helpText = "Capture time for measuring 11p QUAR EVM.\r\nDefault value is 371 us";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_CAPTURE_TIME_11P_QUAR_US", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11N_MIXED_US = 123;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11N_MIXED_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11N_MIXED_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring 11n mixed format EVM.\r\nDefault value is 123 us";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_CAPTURE_TIME_11N_MIXED_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11N_GREENFIELD_US = 115;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11N_GREENFIELD_US))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11N_GREENFIELD_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring 11n GREENFIELD format EVM.\r\nDefault value is 115 us";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_CAPTURE_TIME_11N_GREENFIELD_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

#pragma region REF: HT20, mixed format, longGI

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS8_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS8.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS8_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS8_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS8 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS8.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS8_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS9_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS9.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS9_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS9_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS9 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS9.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS9_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS10_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS10.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS10_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS10_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS10 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS10.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS10_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS11_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS11.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS11_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS11_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS11 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS11.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS11_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS12_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS12.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS12_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS12_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS12 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS12.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS12_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS13_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS13.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS13_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS13_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS13 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS13.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS13_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS14_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS14.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS14_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS14_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS14 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS14.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS14_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS15_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS15.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS15_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS15_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS15 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS15.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS15_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS16_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS16.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS16_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS16_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS16 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS16.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS16_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS17_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS17.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS17_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS17_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS17 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS17.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS17_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS18_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS18.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS18_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS18_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS18 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS18.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS18_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS19_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS19.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS19_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS19_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS19 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS19.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS19_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS20_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS20.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS20_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS20_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS20 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS20.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS20_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS21_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS21.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS21_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS21_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS21 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS21.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS21_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS22_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS22.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS22_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS22_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS22 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS22.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS22_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS23_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS23.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS23_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS23_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS23 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS23.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS23_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS24_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS24.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS24_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS24_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS24 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS24.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS24_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS25_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS25.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS25_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS25_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS25 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS25.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS25_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS26_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS26.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS26_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS26_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS26 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS26.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS26_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS27_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS27.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS27_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS27_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS27 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS27.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS27_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS28_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS28.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS28_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS28_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS28 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS28.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS28_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS29_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS29.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS29_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS29_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS29 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS29.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS29_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS30_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS30.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS30_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS30_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS30 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS30.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS30_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS31_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MCS31.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS31_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_MCS31_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS31 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MCS31.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_MCS31_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
#pragma endregion

#pragma region REF: HT40, mixed format, longGI

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS8_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS8.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS8_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS8_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS8 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS8.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS8_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS9_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS9.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS9_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS9_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS9 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS9.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS9_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS10_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS10.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS10_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS10_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS10 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS10.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS10_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS11_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS11.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS11_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS11_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS11 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS11.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS11_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS12_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS12.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS12_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS12_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS12 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS12.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS12_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS13_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS13.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS13_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS13_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS13 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS13.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS13_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS14_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS14.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS14_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS14_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS14 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS14.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS14_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS15_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS15.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS15_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS15_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS15 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS15.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS15_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS16_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS16.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS16_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS16_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS16 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS16.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS16_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS17_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS17.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS17_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS17_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS17 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS17.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS17_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS18_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS18.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS18_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS18_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS18 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS18.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS18_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS19_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS19.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS19_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS19_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS19 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS19.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS19_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS20_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS20.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS20_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS20_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS20 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS20.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS20_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS21_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS21.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS21_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS21_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS21 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS21.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS21_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS22_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS22.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS22_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS22_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS22 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS22.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS22_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS23_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS23.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS23_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS23_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS23 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS23.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS23_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS24_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS24.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS24_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS24_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS24 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS24.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS24_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS25_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS25.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS25_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS25_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS25 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS25.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS25_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS26_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS26.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS26_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS26_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS26 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS26.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS26_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS27_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS27.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS27_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS27_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS27 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS27.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS27_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS28_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS28.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS28_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS28_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS28 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS28.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS28_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS29_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS29.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS29_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS29_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS29 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS29.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS29_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS30_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS30.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS30_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS30_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS30 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS30.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS30_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS31_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MCS31.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS31_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_MCS31_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS31 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_MCS31.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_MCS31_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
#pragma endregion

#pragma region REF: HT20, mixed format, shortGI

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS8_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS8.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS8_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS8_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS8 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS8.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS8_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS9_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS9.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS9_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS9_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS9 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS9.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS9_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS10_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS10.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS10_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS10_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS10 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS10.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS10_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS11_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS11.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS11_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS11_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS11 (mixed format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS11.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS11_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS12_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS12.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS12_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS12_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS12 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS12.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS12_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS13_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS13.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS13_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS13_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS13 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS13.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS13_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS14_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS14.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS14_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS14_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS14 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS14.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS14_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS15_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS15.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS15_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS15_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS15 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS15.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS15_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS16_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS16.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS16_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS16_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS16 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS16.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS16_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS17_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS17.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS17_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS17_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS17 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS17.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS17_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS18_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS18.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS18_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS18_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS18 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS18.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS18_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS19_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS19.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS19_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS19_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS19 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS19.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS19_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS20_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS20.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS20_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS20_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS20 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS20.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS20_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS21_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS21.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS21_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS21_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS21 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS21.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS21_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS22_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS22.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS22_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS22_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS22 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS22.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS22_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS23_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS23.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS23_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS23_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS23 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS23.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS23_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS24_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS24.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS24_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS24_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS24 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS24.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS24_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS25_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS25.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS25_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS25_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS25 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS25.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS25_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS26_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS26.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS26_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS26_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS26 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS26.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS26_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS27_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS27.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS27_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS27_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS27 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS27.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS27_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS28_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS28.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS28_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS28_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS28 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS28.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS28_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS29_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS29.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS29_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS29_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS29 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS29.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS29_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS30_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS30.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS30_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS30_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS30 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS30.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS30_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS31_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_MIXED_S_GI_MCS31.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS31_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_MIXED_S_GI_MCS31_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS31 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_MIXED_S_GI_MCS31.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_MIXED_S_GI_MCS31_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
#pragma endregion
 
#pragma region REF: HT40, mixed format, shortGI


    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS8_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS8.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS8_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS8_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS8 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS8.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS8_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS9_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS9.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS9_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS9_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS9 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS9.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS9_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS10_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS10.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS10_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS10_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS10 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS10.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS10_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS11_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS11.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS11_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS11_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS11 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS11.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS11_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS12_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS12.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS12_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS12_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS12 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS12.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS12_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS13_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS13.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS13_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS13_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS13 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS13.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS13_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS14_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS14.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS14_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS14_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS14 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS14.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS14_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS15_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS15.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS15_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS15_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS15 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS15.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS15_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS16_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS16.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS16_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS16_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS16 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS16.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS16_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS17_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS17.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS17_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS17_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS17 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS17.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS17_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS18_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS18.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS18_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS18_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS18 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS18.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS18_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS19_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS19.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS19_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS19_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS19 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS19.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS19_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS20_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS20.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS20_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS20_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS20 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS20.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS20_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS21_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS21.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS21_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS21_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS21 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS21.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS21_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS22_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS22.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS22_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS22_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS22 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS22.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS22_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS23_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS23.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS23_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS23_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS23 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS23.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS23_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS24_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS24.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS24_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS24_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS24 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS24.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS24_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS25_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS25.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS25_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS25_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS25 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS25.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS25_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS26_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS26.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS26_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS26_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS26 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS26.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS26_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS27_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS27.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS27_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS27_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS27 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS27.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS27_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS28_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS28.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS28_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS28_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS28 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS28.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS28_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS29_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS29.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS29_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS29_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS29 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS29.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS29_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS30_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS30.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS30_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS30_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS30 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS30.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS30_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS31_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_MIXED_S_GI_MCS31.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS31_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_MIXED_S_GI_MCS31_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS31 (mixed format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_MIXED_S_GI_MCS31.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_MIXED_S_GI_MCS31_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
#pragma endregion

#pragma region REF: HT20, greenfield format, longGI

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS8_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS8.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS8_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS8_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS8 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS8.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS8_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS9_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS9.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS9_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS9_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS9 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS9.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS9_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS10_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS10.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS10_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS10_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS10 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS10.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS10_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS11_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS11.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS11_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS11_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS11 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS11.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS11_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS12_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS12.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS12_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS12_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS12 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS12.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS12_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS13_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS13.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS13_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS13_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS13 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS13.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS13_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS14_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS14.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS14_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS14_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS14 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS14.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS14_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS15_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS15.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS15_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS15_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS15 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS15.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS15_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS16_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS16.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS16_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS16_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS16 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS16.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS16_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS17_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS17.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS17_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS17_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS17 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS17.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS17_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS18_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS18.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS18_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS18_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS18 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS18.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS18_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS19_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS19.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS19_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS19_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS19 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS19.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS19_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS20_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS20.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS20_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS20_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS20 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS20.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS20_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS21_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS21.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS21_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS21_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS21 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS21.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS21_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS22_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS22.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS22_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS22_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS22 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS22.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS22_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS23_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS23.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS23_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS23_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS23 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS23.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS23_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS24_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS24.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS24_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS24_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS24 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS24.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS24_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS25_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS25.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS25_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS25_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS25 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS25.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS25_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS26_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS26.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS26_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS26_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS26 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS26.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS26_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS27_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS27.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS27_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS27_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS27 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS27.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS27_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS28_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS28.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS28_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS28_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS28 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS28.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS28_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS29_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS29.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS29_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS29_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS29 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS29.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS29_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS30_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS30.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS30_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS30_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS30 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS30.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS30_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS31_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_L_GI_MCS31.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS31_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_L_GI_MCS31_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS31 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_L_GI_MCS31.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_L_GI_MCS31_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
#pragma endregion

#pragma region REF: HT40, greenfield format, longGI

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS8_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS8.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS8_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS8_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS8 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS8.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS8_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS9_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS9.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS9_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS9_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS9 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS9.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS9_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS10_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS10.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS10_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS10_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS10 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS10.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS10_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS11_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS11.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS11_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS11_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS11 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS11.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS11_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS12_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS12.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS12_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS12_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS12 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS12.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS12_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS13_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS13.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS13_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS13_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS13 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS13.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS13_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS14_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS14.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS14_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS14_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS14 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS14.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS14_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS15_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS15.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS15_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS15_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS15 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS15.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS15_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS16_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS16.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS16_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS16_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS16 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS16.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS16_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS17_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS17.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS17_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS17_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS17 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS17.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS17_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS18_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS18.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS18_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS18_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS18 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS18.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS18_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS19_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS19.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS19_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS19_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS19 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS19.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS19_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS20_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS20.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS20_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS20_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS20 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS20.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS20_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS21_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS21.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS21_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS21_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS21 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS21.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS21_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS22_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS22.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS22_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS22_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS22 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS22.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS22_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS23_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS23.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS23_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS23_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS23 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS23.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS23_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS24_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS24.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS24_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS24_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS24 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS24.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS24_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS25_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS25.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS25_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS25_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS25 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS25.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS25_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS26_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS26.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS26_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS26_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS26 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS26.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS26_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS27_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS27.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS27_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS27_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS27 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS27.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS27_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS28_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS28.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS28_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS28_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS28 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS28.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS28_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS29_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS29.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS29_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS29_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS29 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS29.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS29_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS30_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS30.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS30_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS30_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS30 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS30.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS30_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS31_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_L_GI_MCS31.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS31_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_L_GI_MCS31_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS31 (greenfield format, long guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_L_GI_MCS31.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_L_GI_MCS31_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
#pragma endregion

#pragma region REF: HT20, greenfield format, shortGI

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS8_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS8.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS8_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS8_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS8 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS8.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS8_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS9_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS9.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS9_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS9_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS9 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS9.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS9_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS10_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS10.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS10_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS10_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS10 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS10.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS10_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS11_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS11.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS11_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS11_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS11 (greenfield format) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS11.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS11_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS12_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS12.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS12_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS12_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS12 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS12.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS12_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS13_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS13.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS13_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS13_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS13 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS13.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS13_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS14_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS14.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS14_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS14_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS14 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS14.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS14_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS15_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS15.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS15_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS15_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT20 MCS15 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS15.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS15_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS16_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS16.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS16_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS16_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS16 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS16.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS16_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS17_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS17.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS17_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS17_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS17 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS17.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS17_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS18_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS18.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS18_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS18_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS18 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS18.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS18_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS19_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS19.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS19_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS19_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS19 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS19.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS19_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS20_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS20.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS20_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS20_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS20 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS20.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS20_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS21_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS21.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS21_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS21_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS21 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS21.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS21_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS22_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS22.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS22_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS22_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS22 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS22.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS22_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS23_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS23.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS23_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS23_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS23 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS23.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS23_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS24_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS24.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS24_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS24_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS24 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS24.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS24_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS25_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS25.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS25_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS25_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS25 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS25.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS25_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS26_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS26.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS26_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS26_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS26 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS26.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS26_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS27_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS27.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS27_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS27_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS27 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS27.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS27_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS28_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS28.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS28_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS28_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS28 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS28.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS28_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS29_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS29.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS29_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS29_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS29 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS29.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS29_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS30_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS30.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS30_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS30_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS30 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS30.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS30_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS31_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT20_GREENFIELD_S_GI_MCS31.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS31_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT20_GREENFIELD_S_GI_MCS31_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT20 MCS31 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT20_GREENFIELD_S_GI_MCS31.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT20_GREENFIELD_S_GI_MCS31_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
#pragma endregion
 
#pragma region REF: HT40, greenfield format, shortGI


    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS8_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS8.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS8_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS8_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS8 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS8.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS8_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS9_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS9.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS9_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS9_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS9 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS9.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS9_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS10_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS10.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS10_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS10_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS10 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS10.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS10_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS11_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS11.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS11_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS11_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS11 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS11.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS11_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS12_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS12.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS12_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS12_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS12 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS12.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS12_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS13_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS13.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS13_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS13_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS13 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS13.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS13_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS14_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS14.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS14_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS14_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS14 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS14.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS14_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS15_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS15.ref");
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS15_REFERENCE_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS15_REFERENCE_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "Reference file name for HT40 MCS15 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS15.ref";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS15_REFERENCE_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS16_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS16.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS16_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS16_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS16 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS16.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS16_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS17_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS17.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS17_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS17_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS17 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS17.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS17_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS18_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS18.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS18_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS18_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS18 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS18.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS18_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS19_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS19.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS19_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS19_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS19 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS19.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS19_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS20_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS20.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS20_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS20_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS20 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS20.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS20_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS21_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS21.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS21_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS21_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS21 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS21.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS21_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS22_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS22.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS22_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS22_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS22 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS22.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS22_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS23_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS23.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS23_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS23_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS23 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS23.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS23_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS24_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS24.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS24_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS24_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS24 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS24.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS24_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS25_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS25.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS25_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS25_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS25 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS25.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS25_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS26_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS26.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS26_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS26_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS26 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS26.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS26_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS27_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS27.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS27_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS27_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS27 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS27.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS27_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS28_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS28.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS28_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS28_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS28 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS28.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS28_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS29_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS29.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS29_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS29_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS29 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS29.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS29_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS30_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS30.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS30_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS30_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS30 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS30.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS30_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS31_REFERENCE_FILE_NAME, MAX_BUFFER_SIZE, "WiFi_HT40_GREENFIELD_S_GI_MCS31.ref");
	if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS31_REFERENCE_FILE_NAME))    // Type_Checking
	{
		setting.value = (void*)g_WiFiGlobalSettingParam.EVM_11N_HT40_GREENFIELD_S_GI_MCS31_REFERENCE_FILE_NAME;
		setting.unit  = "";
		setting.helpText = "Reference file name for HT40 MCS31 (greenfield format, short guard interval) data rate.\r\nDefault value is WiFi_HT40_GREENFIELD_S_GI_MCS31.ref";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("EVM_11N_HT40_GREENFIELD_S_GI_MCS31_REFERENCE_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
#pragma endregion

	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	g_WiFiGlobalSettingParam.VSG_MAX_POWER_11B = 0;
	if (sizeof(double)==sizeof(g_WiFiGlobalSettingParam.VSG_MAX_POWER_11B))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.VSG_MAX_POWER_11B;
		setting.unit  = "";
		setting.helpText  = "VSG RF Max output power limit for 11 B signal \r\nDefault value is 0 dBm";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSG_MAX_POWER_11B", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	g_WiFiGlobalSettingParam.VSG_MAX_POWER_11G = -5;
	if (sizeof(double)==sizeof(g_WiFiGlobalSettingParam.VSG_MAX_POWER_11G))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.VSG_MAX_POWER_11G;
		setting.unit  = "";
		setting.helpText  = "VSG RF Max output power limit for 11 G signal \r\nDefault value is -5 dBm";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSG_MAX_POWER_11G", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	g_WiFiGlobalSettingParam.VSG_MAX_POWER_11N = -5;
	if (sizeof(double)==sizeof(g_WiFiGlobalSettingParam.VSG_MAX_POWER_11N))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.VSG_MAX_POWER_11N;
		setting.unit  = "";
		setting.helpText  = "VSG RF Max output power limit for 11 N signal \r\nDefault value is -5 dBm";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSG_MAX_POWER_11N", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// For IQ2010Ext Only
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT = 100;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT;
		setting.unit  = "count";
		setting.helpText  = "(IQ2010EXT ONLY) Skip packet count before capture. Default=100.";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_SKIP_PACKET_COUNT", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// For IQ2010Ext Only
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	g_WiFiGlobalSettingParam.VSA_ACK_POWER_RMS_DBM = 15;
	if (sizeof(double)==sizeof(g_WiFiGlobalSettingParam.VSA_ACK_POWER_RMS_DBM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.VSA_ACK_POWER_RMS_DBM;
		setting.unit  = "dBm";
		setting.helpText  = "(IQ2010EXT ONLY) The DUT ACK RMS Power at the tester VSA port. Default=15.";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_ACK_POWER_RMS_DBM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// For IQ2010Ext Only
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	g_WiFiGlobalSettingParam.VSA_ACK_TRIGGER_LEVEL_DBM = 10;
	if (sizeof(double)==sizeof(g_WiFiGlobalSettingParam.VSA_ACK_TRIGGER_LEVEL_DBM))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.VSA_ACK_TRIGGER_LEVEL_DBM;
		setting.unit  = "dBm";
		setting.helpText  = "(IQ2010EXT ONLY) The trigger level set to do ACK detection. Default=10.";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("VSA_ACK_TRIGGER_LEVEL_DBM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.PER_VSG_TIMEOUT_SEC = 20;
    if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.PER_VSG_TIMEOUT_SEC))    // Type_Checking
    {
        setting.value = (void*)&g_WiFiGlobalSettingParam.PER_VSG_TIMEOUT_SEC;
        setting.unit  = "sec";
        setting.helpText = "Timeout for waiting for VSG TxDone().\r\n Default=20 sec.";
        g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("PER_VSG_TIMEOUT_SEC", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.retryTestItem = 0;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.retryTestItem))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.retryTestItem;
		setting.unit  = "";
		setting.helpText = "Specify no. of retry on test item if results are failed.";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("RETRY_TEST_ITEM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.retryTestItemDuringRetry = 0;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.retryTestItemDuringRetry))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.retryTestItemDuringRetry;
		setting.unit  = "";
		setting.helpText = "Specify whether a DUT reset function will be called when a retry occurs,\ncalls the \"RESET_TEST_ITEM\" test function.";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("RESET_TEST_ITEM_DURING_RETRY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.retryErrorItems = 0;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.retryErrorItems))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.retryErrorItems;
		setting.unit  = "";
		setting.helpText = "Specify whether a retry an item if an error is returned,\nthis will not affect whether an item is retried for a limit failure";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("RETRY_ERROR_ITEMS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.RELATIVE_LIMIT = 0;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.RELATIVE_LIMIT))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.RELATIVE_LIMIT;
		setting.unit  = "";
		setting.helpText = "Use relative power limit";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("RELATIVE_LIMIT", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	g_WiFiGlobalSettingParam.AUTO_READING_LIMIT = 0;
	if (sizeof(int)==sizeof(g_WiFiGlobalSettingParam.AUTO_READING_LIMIT))    // Type_Checking
	{
		setting.value = (void*)&g_WiFiGlobalSettingParam.AUTO_READING_LIMIT;
		setting.unit  = "";
		setting.helpText = "Read limit automatically or not.";
		g_WiFiGlobalSettingParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("AUTO_READING_LIMIT", setting) );
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
    l_WiFiGlobalSettingReturnMap.clear();

    g_WiFiGlobalSettingReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(g_WiFiGlobalSettingReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)g_WiFiGlobalSettingReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_WiFiGlobalSettingReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
	}

    return 0;
}