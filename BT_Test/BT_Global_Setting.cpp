#include "stdafx.h"
#include "TestManager.h"
#include "BT_Test.h"
#include "BT_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

// This variable is declared in BT_Test_Internal.cpp
extern vDUT_ID      g_BT_Dut;
extern TM_ID        g_BT_Test_ID;

using namespace std;

// Input Parameter Container
map<string, BT_SETTING_STRUCT> g_BTGlobalSettingParamMap;

// Return Value Container 
map<string, BT_SETTING_STRUCT> l_BTGlobalSettingReturnMap;

BT_GLOBAL_SETTING g_BTGlobalSettingParam;

struct tagReturn
{
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} g_BTGlobalSettingReturn;

#ifndef WIN32
int initGlobalSettingContainers = InitializeGlobalSettingContainers();
#endif

int ClearGlobalSettingReturn(void)
{
	g_BTGlobalSettingParamMap.clear();
	l_BTGlobalSettingReturnMap.clear();
	return 0;
}


BT_TEST_API int BT_Global_Setting(void)
{
    int		err = ERR_OK;
    int		dummyValue = 0;
	char	logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_BTGlobalSettingReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(g_BTGlobalSettingParamMap);
        return err;
    }
	else
	{
		// do nothing
	}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_BTGlobalSettingReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
	   /*---------------------------------------*
		* g_BT_Test_ID need to be valid (>=0) *
		*---------------------------------------*/
		TM_ClearReturns(g_BT_Test_ID);
		if( g_BT_Test_ID<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] BT_Test_ID not valid.\n");
			throw logMessage;
		}
		else
		{
			// do nothing
		}

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		GetInputParameters(g_BTGlobalSettingParamMap);
		err = ERR_OK;	// Global Setting are optinal parameters, thus err always = ERR_OK
	    
		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(g_BTGlobalSettingReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_BTGlobalSettingReturnMap);
		}
		else
		{
			// do nothing
		}
	}
    catch(char *msg)
    {
        ReturnErrorMessage(g_BTGlobalSettingReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(g_BTGlobalSettingReturn.ERROR_MESSAGE, "[BT] Unknown Error!\n");
		err = -1;
    }

    return err;
}

int InitializeGlobalSettingContainers(void)
{
    /*------------------*
     * Input Paramters: *
     *------------------*/
    g_BTGlobalSettingParamMap.clear();

    BT_SETTING_STRUCT setting;

    // Peak to Average Settings for BDR
    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.IQ_P_TO_A_BDR = 3;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.IQ_P_TO_A_BDR))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.IQ_P_TO_A_BDR;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for BDR.\r\nDefault value is 3 dB";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("IQ_P_TO_A_BDR", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    // Peak to Average Settings for EDR
    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.IQ_P_TO_A_EDR = 3;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.IQ_P_TO_A_EDR))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.IQ_P_TO_A_EDR;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for EDR.\r\nDefault value is 3 dB";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("IQ_P_TO_A_EDR", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    // Peak to Average Settings for LE
    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.IQ_P_TO_A_LE = 3;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.IQ_P_TO_A_LE))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.IQ_P_TO_A_LE;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for LE.\r\nDefault value is 3 dB";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("IQ_P_TO_A_LE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    // Peak to Average Settings for CW
    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.IQ_P_TO_A_CW = 3;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.IQ_P_TO_A_CW))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.IQ_P_TO_A_CW;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for CW.\r\nDefault value is 3 dB";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("IQ_P_TO_A_CW", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
	// [Dick Walvis] Packet length for packets with 1 timeslot: 625 usec, assume some pretrigger time, use 650 usec capture time
	g_BTGlobalSettingParam.BT_DH1_SAMPLE_INTERVAL_US = 650;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.BT_DH1_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.BT_DH1_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring DH1 packet, default = 650 us.\nPacket length for packets with 1 timeslot: 625 usec, assume some pretrigger time, use 650 usec capture time.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BT_DH1_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
	// [Dick Walvis] Packet length for packets with 3 timeslot: 3x 625 usec, assume some pretrigger time, use 2000 usec capture time
	g_BTGlobalSettingParam.BT_DH3_SAMPLE_INTERVAL_US = 2000;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.BT_DH3_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.BT_DH3_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring DH3 packet, default = 2000 us.\nPacket length for packets with 3 timeslot: 3x 625 usec, assume some pretrigger time, use 2000 usec capture time.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BT_DH3_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
	// [Dick Walvis] Packet length for packets with 5 timeslot: 5x 625 usec, assume some pretrigger time, use 3250 usec capture time
	g_BTGlobalSettingParam.BT_DH5_SAMPLE_INTERVAL_US = 3250;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.BT_DH5_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.BT_DH5_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring DH5 packet, default = 3250 us.\nPacket length for packets with 5 timeslot: 5x 625 usec, assume some pretrigger time, use 3250 usec capture time.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BT_DH5_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    // [Zhiyong Huang] Packet length for packets with 37 Octets is around 400 usec, assume some pre-trigger and redundant time, use 625 usec capture time
    g_BTGlobalSettingParam.BT_1LE_SAMPLE_INTERVAL_US = 625;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.BT_1LE_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.BT_1LE_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring 1LE packet, default =625 us.\nPacket length for packets with 37 Octets is around 400 usec, assume some pre-trigger and redundant time, use 625 usec capture time.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BT_1LE_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


    setting.type = BT_SETTING_TYPE_INTEGER;
	g_BTGlobalSettingParam.TX_BDR_AVERAGE = 1;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.TX_BDR_AVERAGE))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.TX_BDR_AVERAGE;
        setting.unit  = "times";
        setting.helpText = "Required minumum number of TX_BDR measurements.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("TX_BDR_AVERAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
	g_BTGlobalSettingParam.TX_EDR_AVERAGE = 1;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.TX_EDR_AVERAGE))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.TX_EDR_AVERAGE;
        setting.unit  = "times";
        setting.helpText = "Required minumum number of TX_EDR measurements.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("TX_EDR_AVERAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.TX_LE_AVERAGE = 1;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.TX_LE_AVERAGE))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.TX_LE_AVERAGE;
        setting.unit  = "times";
        setting.helpText = "Required minumum number of TX_LE measurements.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("TX_LE_AVERAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
	g_BTGlobalSettingParam.TX_POWER_CONTROL_AVERAGE = 3;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.TX_POWER_CONTROL_AVERAGE))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.TX_POWER_CONTROL_AVERAGE;
        setting.unit  = "times";
        setting.helpText = "Required minumum number of TX_POWER_CONTROL measurements.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("TX_POWER_CONTROL_AVERAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
	g_BTGlobalSettingParam.DUT_TX_SETTLE_TIME_MS = 0;	
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.DUT_TX_SETTLE_TIME_MS))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.DUT_TX_SETTLE_TIME_MS;
        setting.unit  = "ms";
        setting.helpText  = "A delay time for DUT (TX) settle, Default = 0(ms).";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("DUT_TX_SETTLE_TIME_MS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
	g_BTGlobalSettingParam.DUT_RX_SETTLE_TIME_MS = 0;	
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.DUT_RX_SETTLE_TIME_MS))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.DUT_RX_SETTLE_TIME_MS;
        setting.unit  = "ms";
        setting.helpText  = "A delay time for DUT (RX) settle, Default = 0(ms).";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("DUT_RX_SETTLE_TIME_MS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_STRING;
    strcpy_s(g_BTGlobalSettingParam.BER_WAVEFORM_PATH, MAX_BUFFER_SIZE, "../mod");
    if (MAX_BUFFER_SIZE==sizeof(g_BTGlobalSettingParam.BER_WAVEFORM_PATH))    // Type_Checking
    {
        setting.value = (void*)g_BTGlobalSettingParam.BER_WAVEFORM_PATH;
        setting.unit  = "";
        setting.helpText = "Path where all waveform files are stored for RX BER testing.\r\nDefault setting is the Mod folder under IQlite";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_WAVEFORM_PATH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_STRING;
    strcpy_s(g_BTGlobalSettingParam.BER_WAVEFORM_BD_ADDRESS, MAX_BUFFER_SIZE, "000088C0FFEE");
    if (MAX_BUFFER_SIZE==sizeof(g_BTGlobalSettingParam.BER_WAVEFORM_BD_ADDRESS))    // Type_Checking
    {
        setting.value = (void*)g_BTGlobalSettingParam.BER_WAVEFORM_BD_ADDRESS;
        setting.unit  = "";
        setting.helpText = "BD address used in all waveforms.\r\nDefault value is 000088C0FFEE";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_WAVEFORM_BD_ADDRESS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.BER_1DH1_PACKETS_LENGTH = 27;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.BER_1DH1_PACKETS_LENGTH))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.BER_1DH1_PACKETS_LENGTH;
        setting.unit  = "";
        setting.helpText = "Packet LENGTH of 1DH1 sent to DUT for BER testing.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_1DH1_PACKETS_LENGTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_STRING;
    strcpy_s(g_BTGlobalSettingParam.BER_1DH1_WAVEFORM_NAME, MAX_BUFFER_SIZE, "1DH1_000088C0FFEE.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_BTGlobalSettingParam.BER_1DH1_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_BTGlobalSettingParam.BER_1DH1_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 1DH1.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_1DH1_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.BER_1DH3_PACKETS_LENGTH = 183;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.BER_1DH3_PACKETS_LENGTH))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.BER_1DH3_PACKETS_LENGTH;
        setting.unit  = "";
        setting.helpText = "Packet LENGTH of 1DH3 sent to DUT for BER testing.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_1DH3_PACKETS_LENGTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_STRING;
    strcpy_s(g_BTGlobalSettingParam.BER_1DH3_WAVEFORM_NAME, MAX_BUFFER_SIZE, "1DH3_000088C0FFEE.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_BTGlobalSettingParam.BER_1DH3_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_BTGlobalSettingParam.BER_1DH3_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 1DH3.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_1DH3_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.BER_1DH5_PACKETS_LENGTH = 339;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.BER_1DH5_PACKETS_LENGTH))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.BER_1DH5_PACKETS_LENGTH;
        setting.unit  = "";
        setting.helpText = "Packet LENGTH of 1DH5 sent to DUT for BER testing.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_1DH5_PACKETS_LENGTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_STRING;
    strcpy_s(g_BTGlobalSettingParam.BER_1DH5_WAVEFORM_NAME, MAX_BUFFER_SIZE, "1DH5_000088C0FFEE.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_BTGlobalSettingParam.BER_1DH5_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_BTGlobalSettingParam.BER_1DH5_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 1DH5.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_1DH5_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.BER_2DH1_PACKETS_LENGTH = 54;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.BER_2DH1_PACKETS_LENGTH))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.BER_2DH1_PACKETS_LENGTH;
        setting.unit  = "";
        setting.helpText = "Packet LENGTH of 2DH1 sent to DUT for BER testing.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_2DH1_PACKETS_LENGTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_STRING;
    strcpy_s(g_BTGlobalSettingParam.BER_2DH1_WAVEFORM_NAME, MAX_BUFFER_SIZE, "2DH1_000088C0FFEE.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_BTGlobalSettingParam.BER_2DH1_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_BTGlobalSettingParam.BER_2DH1_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 2DH1.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_2DH1_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.BER_2DH3_PACKETS_LENGTH = 367;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.BER_2DH3_PACKETS_LENGTH))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.BER_2DH3_PACKETS_LENGTH;
        setting.unit  = "";
        setting.helpText = "Packet LENGTH of 2DH3 sent to DUT for BER testing.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_2DH3_PACKETS_LENGTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_STRING;
    strcpy_s(g_BTGlobalSettingParam.BER_2DH3_WAVEFORM_NAME, MAX_BUFFER_SIZE, "2DH3_000088C0FFEE.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_BTGlobalSettingParam.BER_2DH3_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_BTGlobalSettingParam.BER_2DH3_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 2DH3.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_2DH3_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.BER_2DH5_PACKETS_LENGTH = 679;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.BER_2DH5_PACKETS_LENGTH))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.BER_2DH5_PACKETS_LENGTH;
        setting.unit  = "";
        setting.helpText = "Packet LENGTH of 2DH5 sent to DUT for BER testing.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_2DH5_PACKETS_LENGTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_STRING;
    strcpy_s(g_BTGlobalSettingParam.BER_2DH5_WAVEFORM_NAME, MAX_BUFFER_SIZE, "2DH5_000088C0FFEE.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_BTGlobalSettingParam.BER_2DH5_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_BTGlobalSettingParam.BER_2DH5_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 2DH5.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_2DH5_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.BER_3DH1_PACKETS_LENGTH = 83;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.BER_3DH1_PACKETS_LENGTH))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.BER_3DH1_PACKETS_LENGTH;
        setting.unit  = "";
        setting.helpText = "Packet LENGTH of 3DH1 sent to DUT for BER testing.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_3DH1_PACKETS_LENGTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_STRING;
    strcpy_s(g_BTGlobalSettingParam.BER_3DH1_WAVEFORM_NAME, MAX_BUFFER_SIZE, "3DH1_000088C0FFEE.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_BTGlobalSettingParam.BER_3DH1_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_BTGlobalSettingParam.BER_3DH1_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 3DH1.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_3DH1_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.BER_3DH3_PACKETS_LENGTH = 552;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.BER_3DH3_PACKETS_LENGTH))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.BER_3DH3_PACKETS_LENGTH;
        setting.unit  = "";
        setting.helpText = "Packet LENGTH of 3DH3 sent to DUT for BER testing.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_3DH3_PACKETS_LENGTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_STRING;
    strcpy_s(g_BTGlobalSettingParam.BER_3DH3_WAVEFORM_NAME, MAX_BUFFER_SIZE, "3DH3_000088C0FFEE.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_BTGlobalSettingParam.BER_3DH3_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_BTGlobalSettingParam.BER_3DH3_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 3DH3.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_3DH3_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.BER_3DH5_PACKETS_LENGTH = 1021;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.BER_3DH5_PACKETS_LENGTH))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.BER_3DH5_PACKETS_LENGTH;
        setting.unit  = "";
        setting.helpText = "Packet LENGTH of 3DH5 sent to DUT for BER testing.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_3DH5_PACKETS_LENGTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_STRING;
    strcpy_s(g_BTGlobalSettingParam.BER_3DH5_WAVEFORM_NAME, MAX_BUFFER_SIZE, "3DH5_000088C0FFEE.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_BTGlobalSettingParam.BER_3DH5_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_BTGlobalSettingParam.BER_3DH5_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 3DH5.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_3DH5_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.PER_1LE_PACKETS_LENGTH = 37;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.PER_1LE_PACKETS_LENGTH))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.PER_1LE_PACKETS_LENGTH;
        setting.unit  = "octets";
        setting.helpText = "Packet LENGTH of 1LE sent to DUT for PER testing.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("PER_1LE_PACKETS_LENGTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_STRING;
    strcpy_s(g_BTGlobalSettingParam.PER_1LE_WAVEFORM_NAME, MAX_BUFFER_SIZE, "1LE.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_BTGlobalSettingParam.PER_1LE_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_BTGlobalSettingParam.PER_1LE_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for 1LE.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("PER_1LE_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
    // [Zhiyong Huang] Waveform length for single frame LE: 625 usec
    g_BTGlobalSettingParam.BT_1LE_PER_WAVEFORM_INTERVAL_US = 625;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.BT_1LE_PER_WAVEFORM_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.BT_1LE_PER_WAVEFORM_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Waveform length for PER test , default = 625 us.\nWaveform length for multiple packet in one waveform will be much longer.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BT_1LE_PER_WAVEFORM_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.VSA_TRIGGER_TYPE = 6;      // 6: IQV_TRIG_TYPE_IF2_NO_CAL
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.VSA_TRIGGER_TYPE))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.VSA_TRIGGER_TYPE;
        setting.unit  = "";
        setting.helpText = "Trigger types used for capturing.\r\nFree Run: 0\r\nExternal Trigger: 1\r\nSignal Trigger: 6\r\nSignal Trigger for IQ2010: 13";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("VSA_TRIGGER_TYPE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_DOUBLE;
    g_BTGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB = 3;
    if (sizeof(double)==sizeof(g_BTGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB;
        setting.unit  = "dB";
        setting.helpText  = "IQTester VSA amplitude setting tolerance. -3 < Default < +3 dB.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("VSA_AMPLITUDE_TOLERANCE_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_DOUBLE;
    g_BTGlobalSettingParam.VSA_TRIGGER_LEVEL_DB = -25;
    if (sizeof(double)==sizeof(g_BTGlobalSettingParam.VSA_TRIGGER_LEVEL_DB))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.VSA_TRIGGER_LEVEL_DB;
        setting.unit  = "dB";
        setting.helpText  = "Signal trigger level";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("VSA_TRIGGER_LEVEL_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_DOUBLE;
	g_BTGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US = 3;
    if (sizeof(double)==sizeof(g_BTGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US;
        setting.unit  = "us";
        setting.helpText  = "IQTester VSA signal pre-trigger time setting used for signal capture.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("VSA_PRE_TRIGGER_TIME_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.VSA_PORT = PORT_LEFT;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.VSA_PORT))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.VSA_PORT;
        setting.unit  = "";
        setting.helpText  = "VSA RF port\r\n2 for RF1(LEFT) and 3 for RF2(RIGHT)";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("VSA_PORT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
    g_BTGlobalSettingParam.VSG_PORT = PORT_LEFT;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.VSG_PORT))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.VSG_PORT;
        setting.unit  = "";
        setting.helpText  = "VSG RF port\r\n2 for RF1(LEFT) and 3 for RF2(RIGHT)";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("VSG_PORT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
    
    setting.type = BT_SETTING_TYPE_INTEGER;
	g_BTGlobalSettingParam.VSA_SAVE_CAPTURE_ON_FAILED = 1;	
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.VSA_SAVE_CAPTURE_ON_FAILED))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.VSA_SAVE_CAPTURE_ON_FAILED;
        setting.unit  = "";
        setting.helpText  = "A flag that to save sig file when capture failed, 0: OFF, 1: ON, Default is 1.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("VSA_SAVE_CAPTURE_ON_FAILED", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
	g_BTGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS = 0;	
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS;
        setting.unit  = "";
        setting.helpText  = "A flag that to save sig file, always, 0: OFF, 1: ON, Default is 0.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("VSA_SAVE_CAPTURE_ALWAYS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = BT_SETTING_TYPE_INTEGER;
	g_BTGlobalSettingParam.BER_VSG_TIMEOUT_SEC = 20;
    if (sizeof(int)==sizeof(g_BTGlobalSettingParam.BER_VSG_TIMEOUT_SEC))    // Type_Checking
    {
        setting.value = (void*)&g_BTGlobalSettingParam.BER_VSG_TIMEOUT_SEC;
        setting.unit  = "sec";
        setting.helpText = "Timeout for waiting for VSG TxDone().\r\n Default=20 sec.";
        g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("BER_VSG_TIMEOUT_SEC", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = BT_SETTING_TYPE_DOUBLE;
	g_BTGlobalSettingParam.VSG_MAX_POWER_BT = -5;
	if (sizeof(double)==sizeof(g_BTGlobalSettingParam.VSG_MAX_POWER_BT))    // Type_Checking
	{
		setting.value = (void*)&g_BTGlobalSettingParam.VSG_MAX_POWER_BT;
		setting.unit  = "dBm";
		setting.helpText  = "VSG RF Max output power limit for BT signal \r\nDefault value is -5 dBm";
		g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("VSG_MAX_POWER_BT", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = BT_SETTING_TYPE_INTEGER;
	g_BTGlobalSettingParam.retryTestItem = 0;
	if (sizeof(int)==sizeof(g_BTGlobalSettingParam.retryTestItem))    // Type_Checking
	{
		setting.value = (void*)&g_BTGlobalSettingParam.retryTestItem;
		setting.unit  = "";
		setting.helpText = "Specify no. of retry on test item if results are failed.";
		g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("RETRY_TEST_ITEM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = BT_SETTING_TYPE_INTEGER;
	g_BTGlobalSettingParam.retryTestItemDuringRetry = 0;
	if (sizeof(int)==sizeof(g_BTGlobalSettingParam.retryTestItemDuringRetry))    // Type_Checking
	{
		setting.value = (void*)&g_BTGlobalSettingParam.retryTestItemDuringRetry;
		setting.unit  = "";
		setting.helpText = "Specify whether a DUT reset function will be called when a retry occurs,\ncalls the \"RESET_TEST_ITEM\" test function.";
		g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("RESET_TEST_ITEM_DURING_RETRY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = BT_SETTING_TYPE_INTEGER;
	g_BTGlobalSettingParam.retryErrorItems = 0;
	if (sizeof(int)==sizeof(g_BTGlobalSettingParam.retryErrorItems))    // Type_Checking
	{
		setting.value = (void*)&g_BTGlobalSettingParam.retryErrorItems;
		setting.unit  = "";
		setting.helpText = "Specify whether a retry an item if an error is returned,\nthis will not affect whether an item is retried for a limit failure";
		g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("RETRY_ERROR_ITEMS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = BT_SETTING_TYPE_INTEGER;
	g_BTGlobalSettingParam.AUTO_READING_LIMIT = 0;
	if (sizeof(int)==sizeof(g_BTGlobalSettingParam.AUTO_READING_LIMIT))    // Type_Checking
	{
		setting.value = (void*)&g_BTGlobalSettingParam.AUTO_READING_LIMIT;
		setting.unit  = "";
		setting.helpText = "Read limit automatically or not.";
		g_BTGlobalSettingParamMap.insert( pair<string, BT_SETTING_STRUCT>("AUTO_READING_LIMIT", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    sprintf_s(g_BTGlobalSettingParam.MP_WAVE_FILE_NAME, MPS_WAVEFILE_NAME);
    g_BTGlobalSettingParam.tab_MP_Wave_DataRate_segment.clear();
    g_BTGlobalSettingParam.tab_MP_Wave_DataRate_segment.push_back("MCS7_HT20");
    g_BTGlobalSettingParam.tab_MP_Wave_DataRate_segment.push_back("MCS4_HT20");
    g_BTGlobalSettingParam.tab_MP_Wave_DataRate_segment.push_back("MCS2_HT20");
    g_BTGlobalSettingParam.tab_MP_Wave_DataRate_segment.push_back("MCS0_HT20");
    g_BTGlobalSettingParam.tab_MP_Wave_DataRate_segment.push_back("OFDM-54");
    g_BTGlobalSettingParam.tab_MP_Wave_DataRate_segment.push_back("OFDM-36");
    g_BTGlobalSettingParam.tab_MP_Wave_DataRate_segment.push_back("OFDM-24");
    g_BTGlobalSettingParam.tab_MP_Wave_DataRate_segment.push_back("OFDM-18");
    g_BTGlobalSettingParam.tab_MP_Wave_DataRate_segment.push_back("OFDM-12");
    g_BTGlobalSettingParam.tab_MP_Wave_DataRate_segment.push_back("OFDM-6");
    g_BTGlobalSettingParam.tab_MP_Wave_DataRate_segment.push_back("CCK-11");
    g_BTGlobalSettingParam.tab_MP_Wave_DataRate_segment.push_back("DSSS-1");
    g_BTGlobalSettingParam.tab_MP_Wave_DataRate_segment.push_back("BT_1DH5");
    g_BTGlobalSettingParam.tab_MP_Wave_DataRate_segment.push_back("BT_3DH5");

    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_BTGlobalSettingReturnMap.clear();

    g_BTGlobalSettingReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(g_BTGlobalSettingReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)g_BTGlobalSettingReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_BTGlobalSettingReturnMap.insert( pair<string,BT_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}
