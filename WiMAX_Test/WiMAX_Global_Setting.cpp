#include "stdafx.h"
#include "TestManager.h"
#include "WIMAX_Test.h"
#include "WIMAX_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

// This variable is declared in WIMAX_Test_Internal.cpp
extern vDUT_ID      g_WIMAX_Dut;
extern TM_ID        g_WIMAX_Test_ID;

using namespace std;

// Input Parameter Container
map<string, WIMAX_SETTING_STRUCT> g_WIMAXGlobalSettingParamMap;

// Return Value Container 
map<string, WIMAX_SETTING_STRUCT> l_WIMAXGlobalSettingReturnMap;

WIMAX_GLOBAL_SETTING g_WIMAXGlobalSettingParam;

void ClearGlobalSettingReturn(void)
{
	g_WIMAXGlobalSettingParamMap.clear();
	l_WIMAXGlobalSettingReturnMap.clear();
}


struct tagReturn
{
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} g_WIMAXGlobalSettingReturn;

//! Global_Setting (IQview/flex, IQmax, IQnxn, IQmimo, etc.)
/*!
 * Input Parameters
 *
 *  - None
 *
 * Return Values
 *      -# ERROR_MESSAGE (char):			A string for error message 
 *
 * \return 0 No error occurred
 * \return -1 DUT failed to insert.  Please see the returned error message for details
 */
WIMAX_TEST_API int WIMAX_Global_Setting(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_WIMAXGlobalSettingReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WIMAX_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(g_WIMAXGlobalSettingParamMap);
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
        RespondToQueryReturn(l_WIMAXGlobalSettingReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
	   /*----------------------------------------*
		* g_WIMAX_Test_ID need to be valid (>=0) *
		*----------------------------------------*/
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
		GetInputParameters(g_WIMAXGlobalSettingParamMap);
		err = ERR_OK;	// Global Setting are optinal parameters, thus err always = ERR_OK
	    
		/*----------------------*
		 *  Function Completed  *
		 *----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(g_WIMAXGlobalSettingReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
		}
		else
		{
			// do nothing
		}
	}
    catch(char *msg)
    {
        ReturnErrorMessage(g_WIMAXGlobalSettingReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(g_WIMAXGlobalSettingReturn.ERROR_MESSAGE, "[Error] Unknown Error!\n");
    }

	/*-----------------------*
	 *  Return Test Results  *
	 *-----------------------*/
	ReturnTestResults(l_WIMAXGlobalSettingReturnMap);


    return err;
}

void InitializeGlobalSettingContainers(void)
{
    /*------------------*
     * Input Paramters: *
     *------------------*/
    g_WIMAXGlobalSettingParamMap.clear();

    WIMAX_SETTING_STRUCT setting;


    setting.type = WIMAX_SETTING_TYPE_STRING;
    strcpy_s(g_WIMAXGlobalSettingParam.WIMAX_TEST_MODE, MAX_BUFFER_SIZE, "NON-LINK");
    if (MAX_BUFFER_SIZE==sizeof(g_WIMAXGlobalSettingParam.WIMAX_TEST_MODE))    // Type_Checking
    {
        setting.value = (void*)g_WIMAXGlobalSettingParam.WIMAX_TEST_MODE;
        setting.unit  = "";
        setting.helpText = "Indicate the LINK or NON-LINK test mode. Default=NON-LINK.";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("WIMAX_TEST_MODE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    // Peak to Average Settings
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.IQ_P_TO_A_WIMAX = 10;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.IQ_P_TO_A_WIMAX))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.IQ_P_TO_A_WIMAX;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for WIMAX.\r\nDefault value is 10 dB";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("IQ_P_TO_A_WIMAX", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    // Peak to Average Settings of CW signal
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.IQ_P_TO_A_CW = 3;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.IQ_P_TO_A_CW))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.IQ_P_TO_A_CW;
        setting.unit  = "dB";
        setting.helpText = "Peak to average ratio for CW signal.\r\nDefault value is 3 dB";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("IQ_P_TO_A_CW", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_STRING;
    strcpy_s(g_WIMAXGlobalSettingParam.WAVEFORM_PATH, MAX_BUFFER_SIZE, "../mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WIMAXGlobalSettingParam.WAVEFORM_PATH))    // Type_Checking
    {
        setting.value = (void*)g_WIMAXGlobalSettingParam.WAVEFORM_PATH;
        setting.unit  = "";
        setting.helpText = "Path where all waveform files are stored.\r\nDefault setting is the Mod folder under IQlite";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("WAVEFORM_PATH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_STRING;
    strcpy_s(g_WIMAXGlobalSettingParam.WAVEFORM_DESTINATION_MAC, MAX_BUFFER_SIZE, "010000C0FFEE");
    if (MAX_BUFFER_SIZE==sizeof(g_WIMAXGlobalSettingParam.WAVEFORM_DESTINATION_MAC))    // Type_Checking
    {
        setting.value = (void*)g_WIMAXGlobalSettingParam.WAVEFORM_DESTINATION_MAC;
        setting.unit  = "";
        setting.helpText = "Destination MAC address used in all waveforms. All four MAC addresses could be set to the same one.\r\nDefault value is 010000C0FFEE";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("WAVEFORM_DESTINATION_MAC", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.PER_WIMAX_PACKETS_NUM = 1000;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.PER_WIMAX_PACKETS_NUM))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.PER_WIMAX_PACKETS_NUM;
        setting.unit  = "";
        setting.helpText = "Packet number sent to DUT for PER testing.\r\nDefault value is 1000";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("PER_WIMAX_PACKETS_NUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_STRING;
    strcpy_s(g_WIMAXGlobalSettingParam.WIMAX_WAVEFORM_NAME, MAX_BUFFER_SIZE, "WIMAX_MOD_FILE.mod");
    if (MAX_BUFFER_SIZE==sizeof(g_WIMAXGlobalSettingParam.WIMAX_WAVEFORM_NAME))    // Type_Checking
    {
        setting.value = (void*)g_WIMAXGlobalSettingParam.WIMAX_WAVEFORM_NAME;
        setting.unit  = "";
        setting.helpText = "Waveform name for WIMAX PER test.\r\nDefault value is WIMAX.mod";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("WIMAX_WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.PM_AVERAGE = 3;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.PM_AVERAGE))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.PM_AVERAGE;
        setting.unit  = "";
        setting.helpText = "Averaging times used for Power measurement.\r\nDefault value is 3";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("PM_AVERAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.VSA_TRIGGER_TYPE = 6;      // 6: IQV_TRIG_TYPE_IF2_NO_CAL
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.VSA_TRIGGER_TYPE))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.VSA_TRIGGER_TYPE;
        setting.unit  = "";
        setting.helpText = "Trigger types used for capturing.\r\nFree Run: 1\r\nExternal Trigger: 2\r\nSignal Trigger: 6\r\nSignal Trigger for IQ2010: 13";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("VSA_TRIGGER_TYPE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    g_WIMAXGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB = 3;
    if (sizeof(double)==sizeof(g_WIMAXGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB;
        setting.unit  = "dB";
        setting.helpText  = "IQTester VSA amplitude setting tolerance. -3 < Default < +3 dB.";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("VSA_AMPLITUDE_TOLERANCE_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    g_WIMAXGlobalSettingParam.VSA_TRIGGER_LEVEL_DB = -25;
    if (sizeof(double)==sizeof(g_WIMAXGlobalSettingParam.VSA_TRIGGER_LEVEL_DB))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.VSA_TRIGGER_LEVEL_DB;
        setting.unit  = "dB";
        setting.helpText  = "Signal trigger level";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("VSA_TRIGGER_LEVEL_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
	g_WIMAXGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US = 3;
    if (sizeof(double)==sizeof(g_WIMAXGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US;
        setting.unit  = "us";
        setting.helpText  = "IQTester VSA signal pre-trigger time setting used for signal capture.";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("VSA_PRE_TRIGGER_TIME_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.VSA_PORT = PORT_LEFT;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.VSA_PORT))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.VSA_PORT;
        setting.unit  = "";
        setting.helpText  = "VSA RF port\r\n2 for RF1(LEFT) and 3 for RF2(RIGHT)";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("VSA_PORT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.VSG_PORT = PORT_RIGHT;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.VSG_PORT))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.VSG_PORT;
        setting.unit  = "";
        setting.helpText  = "VSG RF port\r\n2 for RF1(LEFT) and 3 for RF2(RIGHT)";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("VSG_PORT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
    
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
	g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ON_FAILED = 1;	
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ON_FAILED))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ON_FAILED;
        setting.unit  = "";
        setting.helpText  = "A flag that to save sig file when capture failed, 0: OFF, 1: ON, Default is 1.";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("VSA_SAVE_CAPTURE_ON_FAILED", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
	g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS = 0;	
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS;
        setting.unit  = "";
        setting.helpText  = "A flag that to save sig file, always, 0: OFF, 1: ON, Default is 0.";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("VSA_SAVE_CAPTURE_ALWAYS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
    
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
	g_WIMAXGlobalSettingParam.DUT_KEEP_TRANSMIT = 1;	
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.DUT_KEEP_TRANSMIT))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.DUT_KEEP_TRANSMIT;
        setting.unit  = "";
        setting.helpText  = "A flag that to let Dut keep Tx until the configuration changed, 0: OFF, 1: ON, Default=1.";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("DUT_KEEP_TRANSMIT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
	g_WIMAXGlobalSettingParam.DUT_TX_SETTLE_TIME_MS = 0;	
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.DUT_TX_SETTLE_TIME_MS))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.DUT_TX_SETTLE_TIME_MS;
        setting.unit  = "ms";
        setting.helpText  = "A delay time for DUT (TX) settle, Default = 0(ms).";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("DUT_TX_SETTLE_TIME_MS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
	g_WIMAXGlobalSettingParam.DUT_RX_SETTLE_TIME_MS = 0;	
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.DUT_RX_SETTLE_TIME_MS))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.DUT_RX_SETTLE_TIME_MS;
        setting.unit  = "ms";
        setting.helpText  = "A delay time for DUT (RX) settle, Default = 0(ms).";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("DUT_RX_SETTLE_TIME_MS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    // [WIMAX] Parameters
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_PH_CORR_MODE = 2;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_PH_CORR_MODE))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_PH_CORR_MODE;
        setting.unit  = "";
        setting.helpText = "Specifies the phase tracking mode used for the analysis. 1=Phase tracking off, 2=Symbol by symbol phase tracking (fast), 3=10-symbol moving average (slow) (NOT supported currently).\r\nDefault value is 2.";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("ANALYSIS_WIMAX_PH_CORR_MODE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_CH_ESTIMATE = 1;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_CH_ESTIMATE))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_CH_ESTIMATE;
        setting.unit  = "";
        setting.helpText = "Specifies the channel estimation and correction mode. 1=Channel estimate based on long preamble symbol, 3=Channel estimate based on whole burst.\r\nDefault value is 1.";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("ANALYSIS_WIMAX_CH_ESTIMATE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_FREQ_SYNC = 2;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_FREQ_SYNC))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_FREQ_SYNC;
        setting.unit  = "";
        setting.helpText = "Specifies the frequency offset correction mode. 1=Does not perform frequency correction, 2=performs frequency correction by using both coarse and fine frequency estimates, 3=performs time-domain correction based on full packet measurement.\r\nDefault value is 2.";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("ANALYSIS_WIMAX_FREQ_SYNC", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_SYM_TIM_CORR = 2;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_SYM_TIM_CORR))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_SYM_TIM_CORR;
        setting.unit  = "";
        setting.helpText = "Specifies the timing offset correction mode. 1=Does not perform timing correction, 2=Correction for symbol clock offset\r\nDefault value is 2.";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("ANALYSIS_WIMAX_SYM_TIM_CORR", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_AMPL_TRACK = 1;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_AMPL_TRACK))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_AMPL_TRACK;
        setting.unit  = "";
        setting.helpText = "Indicates whether symbol to symbol amplitude tracking is enabled. 1=Disabled, 2=Enabled\r\nDefault value is 1.";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("ANALYSIS_WIMAX_AMPL_TRACK", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.ANALYSIS_FFT_RBW = 100;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.ANALYSIS_FFT_RBW))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.ANALYSIS_FFT_RBW;
        setting.unit  = "kHz";
        setting.helpText = "This parameter is used to specify the resolution bandwidth in kHz. Default: 100 kHz.";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("ANALYSIS_FFT_RBW", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.PM_IF_FREQ_SHIFT_MHZ = 0;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.PM_IF_FREQ_SHIFT_MHZ))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.PM_IF_FREQ_SHIFT_MHZ;
        setting.unit  = "MHz";
        setting.helpText = "VSA center frequency shift in MHz in the case where DUT has a strong LO leakage.\r\nDefault value is 0";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("PM_IF_FREQ_SHIFT_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.PM_SAMPLE_INTERVAL_US = 100;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.PM_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.PM_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Required capture time when measuring power in trigger mode.\r\nDefault value is 100 us.";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("PM_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.SPECTRUM_SAMPLE_INTERVAL_US = 1000;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.SPECTRUM_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.SPECTRUM_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Required capture time when measuring spectrum in trigger mode.\r\nDefault value is 1000 us.";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("SPECTRUM_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.MASK_FFT_AVERAGE = 1;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.MASK_FFT_AVERAGE))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.MASK_FFT_AVERAGE;
        setting.unit  = "";
        setting.helpText = "Required minumum number of Mask measurements.\r\nDefalut value is 1";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("MASK_FFT_AVERAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.MASK_SAMPLE_INTERVAL_US = 1000;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.MASK_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.MASK_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Required capture time when measuring MASK in trigger mode.\r\nDefault value is 1000 us.";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("MASK_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.EVM_AVERAGE = 3;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.EVM_AVERAGE))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.EVM_AVERAGE;
        setting.unit  = "";
        setting.helpText = "Required minumum number of EVM measurements.\r\nDefalut value is 3";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("EVM_AVERAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    g_WIMAXGlobalSettingParam.EVM_SAMPLE_INTERVAL_US = 1000;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.EVM_SAMPLE_INTERVAL_US))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.EVM_SAMPLE_INTERVAL_US;
        setting.unit  = "us";
        setting.helpText = "Capture time for measuring EVM.\r\nDefault value is 1000 us";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("EVM_SAMPLE_INTERVAL_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

 //   setting.type = WIMAX_SETTING_TYPE_STRING;
	//strcpy_s(g_WIMAXGlobalSettingParam.EVM_MAP_CONFIG_FILE_NAME, MAX_BUFFER_SIZE, "WIMAX_MAP_FILE.mcf");
 //   if (MAX_BUFFER_SIZE==sizeof(g_WIMAXGlobalSettingParam.EVM_MAP_CONFIG_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)g_WIMAXGlobalSettingParam.EVM_MAP_CONFIG_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "For 802.16e analysis, a map configuration file (*.mcf) is required for non-auto-detect operation.\r\nDefault value is WIMAX_MAP_FILE.mcf";
 //       g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("EVM_MAP_CONFIG_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

	setting.type = WIMAX_SETTING_TYPE_DOUBLE;
	g_WIMAXGlobalSettingParam.VSG_MAX_POWER_WIMAX = -5;
	if (sizeof(double)==sizeof(g_WIMAXGlobalSettingParam.VSG_MAX_POWER_WIMAX))    // Type_Checking
	{
		setting.value = (void*)&g_WIMAXGlobalSettingParam.VSG_MAX_POWER_WIMAX;
		setting.unit  = "dBm";
		setting.helpText  = "VSG RF Max output power limit for WIMAX signal \r\nDefault value is -5 dBm";
		g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("VSG_MAX_POWER_WIMAX", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIMAX_SETTING_TYPE_DOUBLE;
	g_WIMAXGlobalSettingParam.VSG_POWER_FOR_SYNCHRONIZATION_DBM = -50;
	if (sizeof(double)==sizeof(g_WIMAXGlobalSettingParam.VSG_POWER_FOR_SYNCHRONIZATION_DBM))    // Type_Checking
	{
		setting.value = (void*)&g_WIMAXGlobalSettingParam.VSG_POWER_FOR_SYNCHRONIZATION_DBM;
		setting.unit  = "dBm";
		setting.helpText  = "The output power of VSG for WiMAX synchronization. \r\nDefault value is -50 dBm";
		g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("VSG_POWER_FOR_SYNCHRONIZATION_DBM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIMAX_SETTING_TYPE_DOUBLE;
	g_WIMAXGlobalSettingParam.DUT_QUICK_POWER_CHECK_TOLERANCE_DB = 5;
	if (sizeof(double)==sizeof(g_WIMAXGlobalSettingParam.DUT_QUICK_POWER_CHECK_TOLERANCE_DB))    // Type_Checking
	{
		setting.value = (void*)&g_WIMAXGlobalSettingParam.DUT_QUICK_POWER_CHECK_TOLERANCE_DB;
		setting.unit  = "dB";
		setting.helpText  = "DUT quick power check amplitude setting tolerance. Default=+-5";
		g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("DUT_QUICK_POWER_CHECK_TOLERANCE_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
	g_WIMAXGlobalSettingParam.DUT_QUICK_POWER_CHECK_SETTLE_TIME_MS = 1000;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.DUT_QUICK_POWER_CHECK_SETTLE_TIME_MS))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.DUT_QUICK_POWER_CHECK_SETTLE_TIME_MS;
        setting.unit  = "ms";
        setting.helpText = "A delay time for DUT (TX) settle, if quick power check is out of range. Default = 1000(ms).";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("DUT_QUICK_POWER_CHECK_SETTLE_TIME_MS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.type = WIMAX_SETTING_TYPE_INTEGER;
	g_WIMAXGlobalSettingParam.PER_VSG_TIMEOUT_SEC = 20;
    if (sizeof(int)==sizeof(g_WIMAXGlobalSettingParam.PER_VSG_TIMEOUT_SEC))    // Type_Checking
    {
        setting.value = (void*)&g_WIMAXGlobalSettingParam.PER_VSG_TIMEOUT_SEC;
        setting.unit  = "sec";
        setting.helpText = "Timeout for waiting for VSG TxDone().\r\n Default=20 sec.";
        g_WIMAXGlobalSettingParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("PER_VSG_TIMEOUT_SEC", setting) );
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
    l_WIMAXGlobalSettingReturnMap.clear();

    g_WIMAXGlobalSettingReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIMAX_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(g_WIMAXGlobalSettingReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)g_WIMAXGlobalSettingReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_WIMAXGlobalSettingReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return;
}
