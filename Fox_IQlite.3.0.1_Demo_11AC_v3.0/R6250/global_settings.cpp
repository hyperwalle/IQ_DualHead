
#include "stdafx.h"
#include "Windows.h"
#include "TestManager.h"
#include "global_settings.h"
#include <string>
#include <map>
#include <fstream>
using namespace std;
extern TS_Global_Setting g_tsGlobalSetting;
extern map<string, TX_PARAM_IN> g_TxParamMap;
extern map<string, RX_PARAM_IN> g_RxParamMap;
extern STRING_TESTITEM_VECTOR strTestItemVector;
int read_Global_Settings(char* pszGlobalSettingFileName)
{
	TCHAR szDirPath[BUFFER_SIZE]=_T("");
	TCHAR szFilenameGlobalSetting[BUFFER_SIZE]=_T("");
	GetCurrentDirectory(BUFFER_SIZE,szDirPath);
	TCHAR tszFilename[BUFFER_SIZE]=_T("");
	MultiByteToWideChar(CP_ACP,0,pszGlobalSettingFileName,strlen(pszGlobalSettingFileName),tszFilename,BUFFER_SIZE);
	swprintf_s(szFilenameGlobalSetting,BUFFER_SIZE,L"%s\\%s",szDirPath,tszFilename);

	TCHAR szTemp[128]=_T("");
	g_tsGlobalSetting.ANALYSIS_11AC_AMPLITUDE_TRACKING=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11AC_AMPLITUDE_TRACKING"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11AC_DECODE_PSDU=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11AC_DECODE_PSDU"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11AC_FREQUENCY_CORRELATION=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11AC_FREQUENCY_CORRELATION"),2,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11AC_FULL_PACKET_CHANNEL_EST=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11AC_FULL_PACKET_CHANNEL_EST"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11AC_MASK_ACCORDING_CBW=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11AC_MASK_ACCORDING_CBW"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11AC_PHASE_CORR=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11AC_PHASE_CORR"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11AC_SYM_TIMING_CORR=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11AC_SYM_TIMING_CORR"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11AG_AMPL_TRACK=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11AG_AMPL_TRACK"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11AG_CH_ESTIMATE=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11AG_CH_ESTIMATE"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11AG_FREQ_SYNC=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11AG_FREQ_SYNC"),2,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11AG_PH_CORR_MODE=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11AG_PH_CORR_MODE"),2,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11AG_SYM_TIM_CORR=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11AG_SYM_TIM_CORR"),2,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11B_DC_REMOVE_FLAG=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11B_DC_REMOVE_FLAG"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11B_EQ_TAPS=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11B_EQ_TAPS"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11B_FIXED_01_DATA_SEQUENCE=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11B_FIXED_01_DATA_SEQUENCE"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11B_METHOD_11B=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11B_METHOD_11B"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11N_AMPLITUDE_TRACKING=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11N_AMPLITUDE_TRACKING"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11N_DECODE_PSDU=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11N_DECODE_PSDU"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11N_FREQUENCY_CORRELATION=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11N_FREQUENCY_CORRELATION"),2,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11N_FULL_PACKET_CHANNEL_EST"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11N_PHASE_CORR=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11N_PHASE_CORR"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11N_SYM_TIMING_CORR=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11N_SYM_TIMING_CORR"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.AUTO_READING_LIMIT=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("AUTO_READING_LIMIT"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.DUT_KEEP_TRANSMIT=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("DUT_KEEP_TRANSMIT"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.DUT_RX_SETTLE_TIME_MS=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("DUT_RX_SETTLE_TIME_MS"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.DUT_TX_SETTLE_TIME_MS=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("DUT_TX_SETTLE_TIME_MS"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_AVERAGE=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_AVERAGE"),3,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_HT_GF_US=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_CAPTURE_TIME_11AC_HT_GF_US"),300,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_HT_MF_US=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_CAPTURE_TIME_11AC_HT_MF_US"),300,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_NON_HT_US=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_CAPTURE_TIME_11AC_NON_HT_US"),300,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_VHT_US=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_CAPTURE_TIME_11AC_VHT_US"),300,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_CAPTURE_TIME_11AG_US=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_CAPTURE_TIME_11AG_US"),95,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_CAPTURE_TIME_11B_L_US=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_CAPTURE_TIME_11B_L_US"),286,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_CAPTURE_TIME_11B_S_US=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_CAPTURE_TIME_11B_S_US"),190,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_CAPTURE_TIME_11N_GREENFIELD_US=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_CAPTURE_TIME_11N_GREENFIELD_US"),115,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_CAPTURE_TIME_11N_MIXED_US=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_CAPTURE_TIME_11N_MIXED_US"),123,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_PRE_TRIG_TIME_US=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_PRE_TRIG_TIME_US"),3,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_SYMBOL_NUM=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_SYMBOL_NUM"),18,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11AG_12M=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11AG_12M"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11AG_18M=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11AG_18M"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11AG_24M=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11AG_24M"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11AG_36M=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11AG_36M"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11AG_48M=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11AG_48M"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11AG_54M=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11AG_54M"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11AG_6M=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11AG_6M"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11AG_9M=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11AG_9M"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11B_11M=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11B_11M"),2,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11B_1M=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11B_1M"),2,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11B_2M=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11B_2M"),2,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11B_5_5M=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11B_5_5M"),2,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11N_MCS0=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11N_MCS0"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11N_MCS1=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11N_MCS1"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11N_MCS2=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11N_MCS2"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11N_MCS3=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11N_MCS3"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11N_MCS4=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11N_MCS4"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11N_MCS5=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11N_MCS5"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11N_MCS6=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11N_MCS6"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11N_MCS7=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11N_MCS7"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.MASK_DSSS_SAMPLE_INTERVAL_US=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("MASK_DSSS_SAMPLE_INTERVAL_US"),286,szFilenameGlobalSetting);
	g_tsGlobalSetting.MASK_OFDM_SAMPLE_INTERVAL_US=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("MASK_OFDM_SAMPLE_INTERVAL_US"),95,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_FRAME_COUNT_11AC=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_FRAME_COUNT_11AC"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_FRAME_COUNT_11AG=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_FRAME_COUNT_11AG"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_FRAME_COUNT_11B=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_FRAME_COUNT_11B"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_FRAME_COUNT_11N=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_FRAME_COUNT_11N"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_VSG_TIMEOUT_SEC=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_VSG_TIMEOUT_SEC"),20,szFilenameGlobalSetting);
	g_tsGlobalSetting.PM_AVERAGE=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PM_AVERAGE"),3,szFilenameGlobalSetting);
	g_tsGlobalSetting.PM_DSSS_SAMPLE_INTERVAL_US=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PM_DSSS_SAMPLE_INTERVAL_US"),100,szFilenameGlobalSetting);
	g_tsGlobalSetting.PM_IF_FREQ_SHIFT_MHZ=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PM_IF_FREQ_SHIFT_MHZ"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.PM_OFDM_SAMPLE_INTERVAL_US=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PM_OFDM_SAMPLE_INTERVAL_US"),20,szFilenameGlobalSetting);
	g_tsGlobalSetting.RESET_TEST_ITEM_DURING_RETRY=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("RESET_TEST_ITEM_DURING_RETRY"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.RETRY_ERROR_ITEMS=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("RETRY_ERROR_ITEMS"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.RETRY_TEST_ITEM=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("RETRY_TEST_ITEM"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.SPECTRUM_DSSS_SAMPLE_INTERVAL_US=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("SPECTRUM_DSSS_SAMPLE_INTERVAL_US"),286,szFilenameGlobalSetting);
	g_tsGlobalSetting.SPECTRUM_OFDM_SAMPLE_INTERVAL_US=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("SPECTRUM_OFDM_SAMPLE_INTERVAL_US"),95,szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_PORT=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("VSA_PORT"),2,szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_SAVE_CAPTURE_ALWAYS=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("VSA_SAVE_CAPTURE_ALWAYS"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_SAVE_CAPTURE_ON_FAILED=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("VSA_SAVE_CAPTURE_ON_FAILED"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_TRIGGER_TYPE=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("VSA_TRIGGER_TYPE"),6,szFilenameGlobalSetting);
	g_tsGlobalSetting.VSG_PORT=GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("VSG_PORT"),2,szFilenameGlobalSetting);

	// double type
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSA_AMPLITUDE_TOLERANCE_DB"),_T("3"),szTemp,sizeof(szTemp),szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_AMPLITUDE_TOLERANCE_DB = _wtof(szTemp);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSA_PRE_TRIGGER_TIME_US"),_T("3"),szTemp,sizeof(szTemp),szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_PRE_TRIGGER_TIME_US = _wtof(szTemp);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSA_TRIGGER_LEVEL_DB"),_T("-25"),szTemp,sizeof(szTemp),szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_TRIGGER_LEVEL_DB = _wtof(szTemp);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSG_MAX_POWER_11AC"),_T("-5"),szTemp,sizeof(szTemp),szFilenameGlobalSetting);
	g_tsGlobalSetting.VSG_MAX_POWER_11AC = _wtof(szTemp);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSG_MAX_POWER_11B"),_T("0"),szTemp,sizeof(szTemp),szFilenameGlobalSetting);
	g_tsGlobalSetting.VSG_MAX_POWER_11B = _wtof(szTemp);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSG_MAX_POWER_11G"),_T("-5"),szTemp,sizeof(szTemp),szFilenameGlobalSetting);
	g_tsGlobalSetting.VSG_MAX_POWER_11G = _wtof(szTemp);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSG_MAX_POWER_11N"),_T("-5"),szTemp,sizeof(szTemp),szFilenameGlobalSetting);
	g_tsGlobalSetting.VSG_MAX_POWER_11N = _wtof(szTemp);


	// string type
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_WAVEFORM_DESTINATION_MAC"),_T("000000C0FFEE"), g_tsGlobalSetting.PER_WAVEFORM_DESTINATION_MAC,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_WAVEFORM_PATH"),_T("../iqvsg"), g_tsGlobalSetting.PER_WAVEFORM_PATH,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_WAVEFORM_PREFIX_11AC"),_T(""), g_tsGlobalSetting.PER_WAVEFORM_PREFIX_11AC,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_WAVEFORM_PREFIX_11AG"),_T(""), g_tsGlobalSetting.PER_WAVEFORM_PREFIX_11AG,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_WAVEFORM_PREFIX_11B"),_T(""), g_tsGlobalSetting.PER_WAVEFORM_PREFIX_11B,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_WAVEFORM_PREFIX_11N"),_T(""), g_tsGlobalSetting.PER_WAVEFORM_PREFIX_11N,BUFFER_SIZE,szFilenameGlobalSetting);

	// test control settings
	g_tsGlobalSetting.TestCtrl.RunMode=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("RunMode"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.RepeatTimes=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("RepeatTimes"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.ExitWhenDone=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("ExitWhenDone"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.RetryMode=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("RetryMode"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.ShowFailInfo=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("ShowFailInfo"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.nRetryTimes=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("RetryTimes"),3,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.ReloadDutDLL=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("ReloadDutDLL"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.IQ_TESTER_RECONNECT=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("IQ_RECONNECT"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.IQTESTER_TYPE=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("IQTESTER_TYPE"),1,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("IQ_TESTER_IP1"),_T("192.168.100.254"),g_tsGlobalSetting.TestCtrl.IQ_TESTER_IP1,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("IQ_TESTER_IP2"),_T(""),g_tsGlobalSetting.TestCtrl.IQ_TESTER_IP2,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("IQ_TESTER_IP3"),_T(""),g_tsGlobalSetting.TestCtrl.IQ_TESTER_IP3,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("IQ_TESTER_IP4"),_T(""),g_tsGlobalSetting.TestCtrl.IQ_TESTER_IP4,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("DUT_DLL_FILENAME"),_T("LP_DUT.DLL"),g_tsGlobalSetting.TestCtrl.szDUT_DLL_FILENAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("ConnectionString"),_T(""),g_tsGlobalSetting.TestCtrl.szConnectionString,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("TX_PATHLOSS_FILE"),_T("path_loss.csv"),g_tsGlobalSetting.TestCtrl.szTxPathLossFile,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("RX_PATHLOSS_FILE"),_T("path_loss.csv"),g_tsGlobalSetting.TestCtrl.szRxPathLossFile,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("PACKET_FORMAT"),_T("MIXED"),g_tsGlobalSetting.TestCtrl.PACKET_FORMAT,BUFFER_SIZE,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.DH_ENABLE=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("DH_ENABLE"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.DH_OBTAIN_CONTROL_TIMEOUT_MS=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("DH_OBTAIN_CONTROL_TIMEOUT_MS"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.DH_PROBE_TIME_MS=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("DH_PROBE_TIME_MS"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.DH_TOKEN_ID=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("DH_TOKEN_ID"),0,szFilenameGlobalSetting);


	return 0;
}

int run_Global_Settings_AddParam(TM_ID TM_Test)
{
	// Read parameters
	read_Global_Settings("global_settings.ini");
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11AC_AMPLITUDE_TRACKING",g_tsGlobalSetting.ANALYSIS_11AC_AMPLITUDE_TRACKING );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11AC_DECODE_PSDU",g_tsGlobalSetting.ANALYSIS_11AC_DECODE_PSDU );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11AC_FREQUENCY_CORRELATION",g_tsGlobalSetting.ANALYSIS_11AC_FREQUENCY_CORRELATION );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11AC_FULL_PACKET_CHANNEL_EST",g_tsGlobalSetting.ANALYSIS_11AC_FULL_PACKET_CHANNEL_EST );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11AC_MASK_ACCORDING_CBW",g_tsGlobalSetting.ANALYSIS_11AC_MASK_ACCORDING_CBW );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11AC_PHASE_CORR",g_tsGlobalSetting.ANALYSIS_11AC_PHASE_CORR );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11AC_SYM_TIMING_CORR",g_tsGlobalSetting.ANALYSIS_11AC_SYM_TIMING_CORR );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11AG_AMPL_TRACK",g_tsGlobalSetting.ANALYSIS_11AG_AMPL_TRACK );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11AG_CH_ESTIMATE",g_tsGlobalSetting.ANALYSIS_11AG_CH_ESTIMATE );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11AG_FREQ_SYNC",g_tsGlobalSetting.ANALYSIS_11AG_FREQ_SYNC );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11AG_PH_CORR_MODE",g_tsGlobalSetting.ANALYSIS_11AG_PH_CORR_MODE );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11AG_SYM_TIM_CORR",g_tsGlobalSetting.ANALYSIS_11AG_SYM_TIM_CORR );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11B_DC_REMOVE_FLAG",g_tsGlobalSetting.ANALYSIS_11B_DC_REMOVE_FLAG );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11B_EQ_TAPS",g_tsGlobalSetting.ANALYSIS_11B_EQ_TAPS );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11B_FIXED_01_DATA_SEQUENCE",g_tsGlobalSetting.ANALYSIS_11B_FIXED_01_DATA_SEQUENCE );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11B_METHOD_11B",g_tsGlobalSetting.ANALYSIS_11B_METHOD_11B );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11N_AMPLITUDE_TRACKING",g_tsGlobalSetting.ANALYSIS_11N_AMPLITUDE_TRACKING );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11N_DECODE_PSDU",g_tsGlobalSetting.ANALYSIS_11N_DECODE_PSDU );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11N_FREQUENCY_CORRELATION",g_tsGlobalSetting.ANALYSIS_11N_FREQUENCY_CORRELATION );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11N_FULL_PACKET_CHANNEL_EST",g_tsGlobalSetting.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11N_PHASE_CORR",g_tsGlobalSetting.ANALYSIS_11N_PHASE_CORR );
	TM_AddIntegerParameter(TM_Test,"ANALYSIS_11N_SYM_TIMING_CORR",g_tsGlobalSetting.ANALYSIS_11N_SYM_TIMING_CORR );
	TM_AddIntegerParameter(TM_Test,"AUTO_READING_LIMIT",g_tsGlobalSetting.AUTO_READING_LIMIT );
	TM_AddIntegerParameter(TM_Test,"DUT_KEEP_TRANSMIT",g_tsGlobalSetting.DUT_KEEP_TRANSMIT );
	TM_AddIntegerParameter(TM_Test,"DUT_RX_SETTLE_TIME_MS",g_tsGlobalSetting.DUT_RX_SETTLE_TIME_MS );
	TM_AddIntegerParameter(TM_Test,"DUT_TX_SETTLE_TIME_MS",g_tsGlobalSetting.DUT_TX_SETTLE_TIME_MS );
	TM_AddIntegerParameter(TM_Test,"EVM_AVERAGE",g_tsGlobalSetting.EVM_AVERAGE );
	TM_AddIntegerParameter(TM_Test,"EVM_CAPTURE_TIME_11AC_HT_GF_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_HT_GF_US );
	TM_AddIntegerParameter(TM_Test,"EVM_CAPTURE_TIME_11AC_HT_MF_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_HT_MF_US );
	TM_AddIntegerParameter(TM_Test,"EVM_CAPTURE_TIME_11AC_NON_HT_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_NON_HT_US );
	TM_AddIntegerParameter(TM_Test,"EVM_CAPTURE_TIME_11AC_VHT_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_VHT_US );
	TM_AddIntegerParameter(TM_Test,"EVM_CAPTURE_TIME_11AG_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11AG_US );
	TM_AddIntegerParameter(TM_Test,"EVM_CAPTURE_TIME_11B_L_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11B_L_US );
	TM_AddIntegerParameter(TM_Test,"EVM_CAPTURE_TIME_11B_S_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11B_S_US );
	TM_AddIntegerParameter(TM_Test,"EVM_CAPTURE_TIME_11N_GREENFIELD_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11N_GREENFIELD_US );
	TM_AddIntegerParameter(TM_Test,"EVM_CAPTURE_TIME_11N_MIXED_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11N_MIXED_US );
	TM_AddIntegerParameter(TM_Test,"EVM_PRE_TRIG_TIME_US",g_tsGlobalSetting.EVM_PRE_TRIG_TIME_US );
	TM_AddIntegerParameter(TM_Test,"EVM_SYMBOL_NUM",g_tsGlobalSetting.EVM_SYMBOL_NUM );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11AG_12M",g_tsGlobalSetting.IQ_P_TO_A_11AG_12M );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11AG_18M",g_tsGlobalSetting.IQ_P_TO_A_11AG_18M );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11AG_24M",g_tsGlobalSetting.IQ_P_TO_A_11AG_24M );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11AG_36M",g_tsGlobalSetting.IQ_P_TO_A_11AG_36M );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11AG_48M",g_tsGlobalSetting.IQ_P_TO_A_11AG_48M );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11AG_54M",g_tsGlobalSetting.IQ_P_TO_A_11AG_54M );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11AG_6M",g_tsGlobalSetting.IQ_P_TO_A_11AG_6M );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11AG_9M",g_tsGlobalSetting.IQ_P_TO_A_11AG_9M );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11B_11M",g_tsGlobalSetting.IQ_P_TO_A_11B_11M );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11B_1M",g_tsGlobalSetting.IQ_P_TO_A_11B_1M );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11B_2M",g_tsGlobalSetting.IQ_P_TO_A_11B_2M );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11B_5_5M",g_tsGlobalSetting.IQ_P_TO_A_11B_5_5M );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11N_MCS0",g_tsGlobalSetting.IQ_P_TO_A_11N_MCS0 );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11N_MCS1",g_tsGlobalSetting.IQ_P_TO_A_11N_MCS1 );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11N_MCS2",g_tsGlobalSetting.IQ_P_TO_A_11N_MCS2 );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11N_MCS3",g_tsGlobalSetting.IQ_P_TO_A_11N_MCS3 );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11N_MCS4",g_tsGlobalSetting.IQ_P_TO_A_11N_MCS4 );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11N_MCS5",g_tsGlobalSetting.IQ_P_TO_A_11N_MCS5 );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11N_MCS6",g_tsGlobalSetting.IQ_P_TO_A_11N_MCS6 );
	TM_AddIntegerParameter(TM_Test,"IQ_P_TO_A_11N_MCS7",g_tsGlobalSetting.IQ_P_TO_A_11N_MCS7 );
	TM_AddIntegerParameter(TM_Test,"MASK_DSSS_SAMPLE_INTERVAL_US",g_tsGlobalSetting.MASK_DSSS_SAMPLE_INTERVAL_US );
	TM_AddIntegerParameter(TM_Test,"MASK_OFDM_SAMPLE_INTERVAL_US",g_tsGlobalSetting.MASK_OFDM_SAMPLE_INTERVAL_US );
	TM_AddIntegerParameter(TM_Test,"PER_FRAME_COUNT_11AC",g_tsGlobalSetting.PER_FRAME_COUNT_11AC );
	TM_AddIntegerParameter(TM_Test,"PER_FRAME_COUNT_11AG",g_tsGlobalSetting.PER_FRAME_COUNT_11AG );
	TM_AddIntegerParameter(TM_Test,"PER_FRAME_COUNT_11B",g_tsGlobalSetting.PER_FRAME_COUNT_11B );
	TM_AddIntegerParameter(TM_Test,"PER_FRAME_COUNT_11N",g_tsGlobalSetting.PER_FRAME_COUNT_11N );
	TM_AddIntegerParameter(TM_Test,"PER_VSG_TIMEOUT_SEC",g_tsGlobalSetting.PER_VSG_TIMEOUT_SEC );
	TM_AddIntegerParameter(TM_Test,"PM_AVERAGE",g_tsGlobalSetting.PM_AVERAGE );
	TM_AddIntegerParameter(TM_Test,"PM_DSSS_SAMPLE_INTERVAL_US",g_tsGlobalSetting.PM_DSSS_SAMPLE_INTERVAL_US );
	TM_AddIntegerParameter(TM_Test,"PM_IF_FREQ_SHIFT_MHZ",g_tsGlobalSetting.PM_IF_FREQ_SHIFT_MHZ );
	TM_AddIntegerParameter(TM_Test,"PM_OFDM_SAMPLE_INTERVAL_US",g_tsGlobalSetting.PM_OFDM_SAMPLE_INTERVAL_US );
	TM_AddIntegerParameter(TM_Test,"RESET_TEST_ITEM_DURING_RETRY",g_tsGlobalSetting.RESET_TEST_ITEM_DURING_RETRY );
	TM_AddIntegerParameter(TM_Test,"RETRY_ERROR_ITEMS",g_tsGlobalSetting.RETRY_ERROR_ITEMS );
	TM_AddIntegerParameter(TM_Test,"RETRY_TEST_ITEM",g_tsGlobalSetting.RETRY_TEST_ITEM );
	TM_AddIntegerParameter(TM_Test,"SPECTRUM_DSSS_SAMPLE_INTERVAL_US",g_tsGlobalSetting.SPECTRUM_DSSS_SAMPLE_INTERVAL_US );
	TM_AddIntegerParameter(TM_Test,"SPECTRUM_OFDM_SAMPLE_INTERVAL_US",g_tsGlobalSetting.SPECTRUM_OFDM_SAMPLE_INTERVAL_US );
	TM_AddIntegerParameter(TM_Test,"VSA_PORT",g_tsGlobalSetting.VSA_PORT );
	TM_AddIntegerParameter(TM_Test,"VSA_SAVE_CAPTURE_ALWAYS",g_tsGlobalSetting.VSA_SAVE_CAPTURE_ALWAYS );
	TM_AddIntegerParameter(TM_Test,"VSA_SAVE_CAPTURE_ON_FAILED",g_tsGlobalSetting.VSA_SAVE_CAPTURE_ON_FAILED );
	TM_AddIntegerParameter(TM_Test,"VSA_TRIGGER_TYPE",g_tsGlobalSetting.VSA_TRIGGER_TYPE );
	TM_AddIntegerParameter(TM_Test,"VSG_PORT",g_tsGlobalSetting.VSG_PORT );
	// new add end

	// add double param
	
	TM_AddDoubleParameter(TM_Test, "VSA_TRIGGER_LEVEL_DB",g_tsGlobalSetting.VSA_TRIGGER_LEVEL_DB  );
	TM_AddDoubleParameter(TM_Test, "VSA_AMPLITUDE_TOLERANCE_DB",g_tsGlobalSetting.VSA_AMPLITUDE_TOLERANCE_DB  );
	TM_AddDoubleParameter(TM_Test, "VSA_PRE_TRIGGER_TIME_US",g_tsGlobalSetting.VSA_PRE_TRIGGER_TIME_US  );
	TM_AddDoubleParameter(TM_Test, "VSG_MAX_POWER_11B",g_tsGlobalSetting.VSG_MAX_POWER_11B  );
	TM_AddDoubleParameter(TM_Test, "VSG_MAX_POWER_11G",g_tsGlobalSetting.VSG_MAX_POWER_11G  );
	TM_AddDoubleParameter(TM_Test, "VSG_MAX_POWER_11N",g_tsGlobalSetting.VSG_MAX_POWER_11N  );
	TM_AddDoubleParameter(TM_Test, "VSG_MAX_POWER_11AC",g_tsGlobalSetting.VSG_MAX_POWER_11AC  );

	// add string param
	char szAnsiStr[BUFFER_SIZE]="";

	WChar2Char(g_tsGlobalSetting.PER_WAVEFORM_DESTINATION_MAC,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_WAVEFORM_DESTINATION_MAC",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.PER_WAVEFORM_PATH,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_WAVEFORM_PATH",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.PER_WAVEFORM_PREFIX_11AC,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_WAVEFORM_PREFIX_11AC",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.PER_WAVEFORM_PREFIX_11AG,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_WAVEFORM_PREFIX_11AG",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.PER_WAVEFORM_PREFIX_11B,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_WAVEFORM_PREFIX_11B",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.PER_WAVEFORM_PREFIX_11N,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_WAVEFORM_PREFIX_11N",szAnsiStr );

	// new add end
	return 0;
}

int ParseTestItem(char *pszScriptFile)
{
	ifstream is;
	is.open(pszScriptFile,ios_base::in);
	if(!is)
	{
		printf("Fail to open script file %s, GetLastError=%d",pszScriptFile,GetLastError());
		return 0;
	}
	while(!is.eof())
	{
		char szReadLine[256]="";
		is.getline(szReadLine,sizeof(szReadLine));
		if(strstr(szReadLine,"_VERIFY_") ==  NULL)
			continue;
		if(szReadLine[0]=='/' || szReadLine[0]==';' || szReadLine[0]=='\\')
			continue;
		if(strstr(szReadLine,"TX_VERIFY_EVM"))
		{
			//TX_VERIFY_EVM 2412 DSSS-1 HT20 #0 $-5 @3
			char szTemp1[24]="",szTemp2[24]="",szTemp3[24]="";
			TX_PARAM_IN txParam;
			sscanf(szReadLine, "%s %d %s %s %s %s %s",
				txParam.szSubItem,
				&txParam.CH_FREQ_MHZ,
				txParam.DATA_RATE,
				txParam.BSS_BANDWIDTH,
				szTemp1,
				szTemp2,
				szTemp3
				);
			// pass value to the following var.
			txParam.BSS_FREQ_MHZ_PRIMARY = txParam.CH_FREQ_MHZ;
			txParam.BSS_FREQ_MHZ_SECONDARY=0;
			txParam.CH_FREQ_MHZ_PRIMARY_20MHz=0;
			
			sprintf_s(txParam.BSS_BANDWIDTH,sizeof(txParam.BSS_BANDWIDTH), "BW-%d",atoi(txParam.BSS_BANDWIDTH+2));
			sprintf_s(txParam.CH_BANDWIDTH,sizeof(txParam.CH_BANDWIDTH), "C%s",txParam.BSS_BANDWIDTH);

			strncpy_s(szTemp1,sizeof(szTemp1)-1,szTemp1+1,strlen(szTemp1+1));
			strncpy_s(szTemp2,sizeof(szTemp2)-1,szTemp2+1,strlen(szTemp2+1));
			strncpy_s(szTemp3,sizeof(szTemp3)-1,szTemp3+1,strlen(szTemp3+1));

			txParam.dTargetPower=atof(szTemp1);
			txParam.dEvmSpec=atof(szTemp2);
			switch (atoi(szTemp3))
			{
			case 1:
				txParam.nTx1=1;
				txParam.nTx2=0;
				txParam.nTx3=0;
				txParam.nTx4=0;
				break;
			case 2:
				txParam.nTx1=0;
				txParam.nTx2=1;
				txParam.nTx3=0;
				txParam.nTx4=0;
				break;
			case 3:
				txParam.nTx1=1;
				txParam.nTx2=1;
				txParam.nTx3=0;
				txParam.nTx4=0;
				break;
			case 4:
				txParam.nTx1=0;
				txParam.nTx2=0;
				txParam.nTx3=1;
				txParam.nTx4=0;
				break;
			case 5:
				txParam.nTx1=1;
				txParam.nTx2=0;
				txParam.nTx3=1;
				txParam.nTx4=0;
				break;
			case 6:
				txParam.nTx1=0;
				txParam.nTx2=1;
				txParam.nTx3=1;
				txParam.nTx4=0;
				break;
			case 7:
				txParam.nTx1=1;
				txParam.nTx2=1;
				txParam.nTx3=1;
				txParam.nTx4=0;
				break;
			default:
				break;

			}
			txParam.NUM_STREAM_11AC=txParam.nTx1+txParam.nTx2+txParam.nTx3+txParam.nTx4;
			g_TxParamMap.insert(Tx_Pair(szReadLine,txParam));
			strTestItemVector.push_back(szReadLine);
		}
		else if(strstr(szReadLine,"TX_VERIFY_POWER"))
		{
			//TX_VERIFY_POWER 2437 OFDM-54 HT20 #10 @2
			char szTemp1[24]="",szTemp2[24]="",szTemp3[24]="";
			TX_PARAM_IN txParam;
			sscanf(szReadLine, "%s %d %s %s %s %s",
				txParam.szSubItem,
				&txParam.CH_FREQ_MHZ,
				txParam.DATA_RATE,
				txParam.BSS_BANDWIDTH,
				szTemp1,
				szTemp3
				);
			// pass value to the following var.
			txParam.BSS_FREQ_MHZ_PRIMARY = txParam.CH_FREQ_MHZ;
			txParam.BSS_FREQ_MHZ_SECONDARY=0;
			txParam.CH_FREQ_MHZ_PRIMARY_20MHz=0;
			sprintf_s(txParam.BSS_BANDWIDTH,sizeof(txParam.BSS_BANDWIDTH), "BW-%d",atoi(txParam.BSS_BANDWIDTH+2));
			sprintf_s(txParam.CH_BANDWIDTH,sizeof(txParam.CH_BANDWIDTH), "C%s",txParam.BSS_BANDWIDTH);

			strncpy_s(szTemp1,sizeof(szTemp1)-1,szTemp1+1,strlen(szTemp1+1));
			strncpy_s(szTemp3,sizeof(szTemp3)-1,szTemp3+1,strlen(szTemp3+1));

			txParam.dTargetPower=atof(szTemp1);
			switch (atoi(szTemp3))
			{
			case 1:
				txParam.nTx1=1;
				txParam.nTx2=0;
				txParam.nTx3=0;
				txParam.nTx4=0;
				break;
			case 2:
				txParam.nTx1=0;
				txParam.nTx2=1;
				txParam.nTx3=0;
				txParam.nTx4=0;
				break;
			case 3:
				txParam.nTx1=1;
				txParam.nTx2=1;
				txParam.nTx3=0;
				txParam.nTx4=0;
				break;
			case 4:
				txParam.nTx1=0;
				txParam.nTx2=0;
				txParam.nTx3=1;
				txParam.nTx4=0;
				break;
			case 5:
				txParam.nTx1=1;
				txParam.nTx2=0;
				txParam.nTx3=1;
				txParam.nTx4=0;
				break;
			case 6:
				txParam.nTx1=0;
				txParam.nTx2=1;
				txParam.nTx3=1;
				txParam.nTx4=0;
				break;
			case 7:
				txParam.nTx1=1;
				txParam.nTx2=1;
				txParam.nTx3=1;
				txParam.nTx4=0;
				break;
			default:
				break;

			}
			txParam.NUM_STREAM_11AC=txParam.nTx1+txParam.nTx2+txParam.nTx3+txParam.nTx4;
			g_TxParamMap.insert(Tx_Pair(szReadLine,txParam));
			strTestItemVector.push_back(szReadLine);
		}
		else if(strstr(szReadLine,"TX_VERIFY_MASK"))
		{
			//TX_VERIFY_MASK 2437 OFDM-54 HT20 #10  @2


			char szTemp1[24]="",szTemp2[24]="",szTemp3[24]="";
			TX_PARAM_IN txParam;
			sscanf(szReadLine, "%s %d %s %s %s %s",
				txParam.szSubItem,
				&txParam.CH_FREQ_MHZ,
				txParam.DATA_RATE,
				txParam.BSS_BANDWIDTH,
				szTemp1,
				szTemp3
				);
			// pass value to the following var.
			txParam.BSS_FREQ_MHZ_PRIMARY = txParam.CH_FREQ_MHZ;
			txParam.BSS_FREQ_MHZ_SECONDARY=0;
			txParam.CH_FREQ_MHZ_PRIMARY_20MHz=0;
			sprintf_s(txParam.BSS_BANDWIDTH,sizeof(txParam.BSS_BANDWIDTH), "BW-%d",atoi(txParam.BSS_BANDWIDTH+2));
			sprintf_s(txParam.CH_BANDWIDTH,sizeof(txParam.CH_BANDWIDTH), "C%s",txParam.BSS_BANDWIDTH);

			strncpy_s(szTemp1,sizeof(szTemp1)-1,szTemp1+1,strlen(szTemp1+1));
			strncpy_s(szTemp3,sizeof(szTemp3)-1,szTemp3+1,strlen(szTemp3+1));

			txParam.dTargetPower=atof(szTemp1);
			switch (atoi(szTemp3))
			{
			case 1:
				txParam.nTx1=1;
				txParam.nTx2=0;
				txParam.nTx3=0;
				txParam.nTx4=0;
				break;
			case 2:
				txParam.nTx1=0;
				txParam.nTx2=1;
				txParam.nTx3=0;
				txParam.nTx4=0;
				break;
			case 3:
				txParam.nTx1=1;
				txParam.nTx2=1;
				txParam.nTx3=0;
				txParam.nTx4=0;
				break;
			case 4:
				txParam.nTx1=0;
				txParam.nTx2=0;
				txParam.nTx3=1;
				txParam.nTx4=0;
				break;
			case 5:
				txParam.nTx1=1;
				txParam.nTx2=0;
				txParam.nTx3=1;
				txParam.nTx4=0;
				break;
			case 6:
				txParam.nTx1=0;
				txParam.nTx2=1;
				txParam.nTx3=1;
				txParam.nTx4=0;
				break;
			case 7:
				txParam.nTx1=1;
				txParam.nTx2=1;
				txParam.nTx3=1;
				txParam.nTx4=0;
				break;
			default:
				break;

			}
			txParam.NUM_STREAM_11AC=txParam.nTx1+txParam.nTx2+txParam.nTx3+txParam.nTx4;
			g_TxParamMap.insert(Tx_Pair(szReadLine,txParam));
			strTestItemVector.push_back(szReadLine);
		}
		else if(strstr(szReadLine,"RX_VERIFY_PER"))
		{
			//RX_VERIFY_PER 2437 OFDM-54 HT20 #10 &200 @2
			char szTemp1[24]="",szTemp2[24]="",szTemp3[24]="";
			RX_PARAM_IN rxParam;
			sscanf(szReadLine, "%s %d %s %s %s %s %s",
				rxParam.szSubItem,
				&rxParam.CH_FREQ_MHZ,
				rxParam.DATA_RATE,
				rxParam.BSS_BANDWIDTH,
				szTemp1,
				szTemp2,
				szTemp3
				);
			rxParam.CH_FREQ_MHZ_PRIMARY_20MHz =0;
			rxParam.BSS_FREQ_MHZ_SECONDARY = 0;
			rxParam.BSS_FREQ_MHZ_PRIMARY = rxParam.CH_FREQ_MHZ;
			sprintf_s(rxParam.BSS_BANDWIDTH,sizeof(rxParam.BSS_BANDWIDTH), "BW-%d",atoi(rxParam.BSS_BANDWIDTH+2));
			sprintf_s(rxParam.CH_BANDWIDTH,sizeof(rxParam.CH_BANDWIDTH), "C%s",rxParam.BSS_BANDWIDTH);

			strncpy_s(szTemp1,sizeof(szTemp1)-1,szTemp1+1,strlen(szTemp1+1));
			strncpy_s(szTemp2,sizeof(szTemp2)-1,szTemp2+1,strlen(szTemp2+1));
			strncpy_s(szTemp3,sizeof(szTemp3)-1,szTemp3+1,strlen(szTemp3+1));

			rxParam.dRxPowerLevel=atof(szTemp1);
			rxParam.FRAME_COUNT = atof(szTemp2);
			switch (atoi(szTemp3))
			{
			case 1:
				rxParam.nRx1=1;
				rxParam.nRx2=0;
				rxParam.nRx3=0;
				rxParam.nRx4=0;
				break;
			case 2:
				rxParam.nRx1=0;
				rxParam.nRx2=1;
				rxParam.nRx3=0;
				rxParam.nRx4=0;
				break;
			case 3:
				rxParam.nRx1=1;
				rxParam.nRx2=1;
				rxParam.nRx3=0;
				rxParam.nRx4=0;
				break;
			case 4:
				rxParam.nRx1=0;
				rxParam.nRx2=0;
				rxParam.nRx3=1;
				rxParam.nRx4=0;
				break;
			case 5:
				rxParam.nRx1=1;
				rxParam.nRx2=0;
				rxParam.nRx3=1;
				rxParam.nRx4=0;
				break;
			case 6:
				rxParam.nRx1=0;
				rxParam.nRx2=1;
				rxParam.nRx3=1;
				rxParam.nRx4=0;
				break;
			case 7:
				rxParam.nRx1=1;
				rxParam.nRx2=1;
				rxParam.nRx3=1;
				rxParam.nRx4=0;
				break;
			default:
				break;

			}
			rxParam.NUM_STREAM_11AC=rxParam.nRx1+rxParam.nRx2+rxParam.nRx3+rxParam.nRx4;
			g_RxParamMap.insert(Rx_Pair(szReadLine,rxParam));
			strTestItemVector.push_back(szReadLine);
		}
		else
		{
			continue;
		}

	}
	is.close();

	// double type
	TCHAR szTemp[BUFFER_SIZE]=_T("");
	TCHAR szFilenameScript[BUFFER_SIZE]=_T("");
	Char2WChar(pszScriptFile,strlen(pszScriptFile),szTemp,BUFFER_SIZE);
	swprintf_s(szFilenameScript,BUFFER_SIZE,L"%s",szTemp);

	GetPrivateProfileString(_T("WIFI_LIMIT"),_T("POWER_LOWER_2G"),_T("2.0"),szTemp,sizeof(szTemp),szFilenameScript);
	g_tsGlobalSetting.WifiLimit.dPowerLower2g = _wtof(szTemp);
	GetPrivateProfileString(_T("WIFI_LIMIT"),_T("POWER_UPPER_2G"),_T("2.0"),szTemp,sizeof(szTemp),szFilenameScript);
	g_tsGlobalSetting.WifiLimit.dPowerUpper2g = _wtof(szTemp);
	GetPrivateProfileString(_T("WIFI_LIMIT"),_T("POWER_LOWER_5G"),_T("2.50"),szTemp,sizeof(szTemp),szFilenameScript);
	g_tsGlobalSetting.WifiLimit.dPowerLower5g = _wtof(szTemp);
	GetPrivateProfileString(_T("WIFI_LIMIT"),_T("POWER_UPPER_5G"),_T("2.50"),szTemp,sizeof(szTemp),szFilenameScript);
	g_tsGlobalSetting.WifiLimit.dPowerUpper5g = _wtof(szTemp);
	GetPrivateProfileString(_T("WIFI_LIMIT"),_T("MASK_PERCENTAGE"),_T("5"),szTemp,sizeof(szTemp),szFilenameScript);
	g_tsGlobalSetting.WifiLimit.dMaskPercent = _wtof(szTemp);
	GetPrivateProfileString(_T("WIFI_LIMIT"),_T("FREQENCY_LIMIT"),_T("20.00"),szTemp,sizeof(szTemp),szFilenameScript);
	g_tsGlobalSetting.WifiLimit.dFreqencyShift = _wtof(szTemp);
	g_tsGlobalSetting.WifiLimit.dEvmLower=-99.00;
	g_tsGlobalSetting.WifiLimit.dPerPercent=8.00;
	g_tsGlobalSetting.WifiLimit.dPerPercentOFDM=10.00;
	
	return 0;
}

bool ShowTestResultEVM(TX_PARAM_IN *txParam, TX_PARAM_RETURN *txReturn, int iTestCount)
{
	int nAntCount=txParam->nTx1+txParam->nTx2+txParam->nTx3+txParam->nTx4;

	bool bParseResult=false;
	ofstream os;
	os.open(_T(".\\log\\Log_all.txt"),ios_base::app|ios_base::out);
	switch (nAntCount)
	{
	case 3:
		// Power
		if(txParam->CH_FREQ_MHZ <= 2484) // 2.4g test item
		{
			// EVM average
			if( (txReturn->EVM_AVG_ALL < txParam->dEvmSpec && txReturn->EVM_AVG_ALL > g_tsGlobalSetting.WifiLimit.dEvmLower) &&
				(txReturn->FREQ_ERROR < g_tsGlobalSetting.WifiLimit.dFreqencyShift && txReturn->FREQ_ERROR > -g_tsGlobalSetting.WifiLimit.dFreqencyShift) &&
				(txReturn->POWER_AVG_1 < txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper2g && txReturn->POWER_AVG_1 > txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower2g) &&
				(txReturn->POWER_AVG_2 < txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper2g && txReturn->POWER_AVG_2 > txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower2g) &&
				(txReturn->POWER_AVG_3 < txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper2g && txReturn->POWER_AVG_3 > txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower2g))
			{
				printf("\t[#] EVM_AVG_1                :\t %0.2f dB\n",txReturn->EVM_AVG_ALL);
				printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\n",txReturn->FREQ_ERROR);
				printf("\t[#] POWER_AVG_1              :\t %0.2f dBm\n",txReturn->POWER_AVG_1);
				printf("\t[#] POWER_AVG_2              :\t %0.2f dBm\n",txReturn->POWER_AVG_2);
				printf("\t[#] POWER_AVG_3              :\t %0.2f dBm\n",txReturn->POWER_AVG_3);
				os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB"<<endl;
				os<<"\t[#] FREQ_ERROR_AVG           :\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm"<<endl;
				os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<" dBm"<<endl;
				os<<"\t[#] POWER_AVG_2              :\t"<<setprecision(4)<<txReturn->POWER_AVG_2<<" dBm"<<endl;
				os<<"\t[#] POWER_AVG_3              :\t"<<setprecision(4)<<txReturn->POWER_AVG_3<<" dBm"<<endl;
				bParseResult=true;
			}	
			else 
			{
				if(iTestCount == 0)
				{
					// EVM average
					if(txReturn->EVM_AVG_ALL > txParam->dEvmSpec || txReturn->EVM_AVG_ALL <= g_tsGlobalSetting.WifiLimit.dEvmLower)						
					{
						printf("\t[#] EVM_AVG_1                :\t %0.2f dB\t<-- Failed\n",txReturn->EVM_AVG_ALL);
						os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB\t<-- Failed"<<endl;
					}
					else						
					{
						printf("\t[#] EVM_AVG_1                :\t %0.2f dB\n",txReturn->EVM_AVG_ALL);
						os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB"<<endl;
					}

					// Freq
					if(txReturn->FREQ_ERROR > g_tsGlobalSetting.WifiLimit.dFreqencyShift || txReturn->FREQ_ERROR < -g_tsGlobalSetting.WifiLimit.dFreqencyShift)
					{
						printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\t<-- Failed\n",txReturn->FREQ_ERROR);
						os<<"\t[#] FREQ_ERROR_AVG           :\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm\t<-- Failed"<<endl;
					}
					else						
					{
						printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\n",txReturn->FREQ_ERROR);
						os<<"\t[#] FREQ_ERROR_AVG           :\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm"<<endl;
					}

					if(txReturn->POWER_AVG_1 > txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper2g || txReturn->POWER_AVG_1 < txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower2g)
					{
						printf("\t[#] POWER_AVG_1              :\t %0.2f dBm\t<-- Failed\n",txReturn->POWER_AVG_1);
						os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<" dBm\t<-- Failed"<<endl;
					}
					else
					{
						printf("\t[#] POWER_AVG_1              :\t %0.2f\n",txReturn->POWER_AVG_1);
						os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<endl;
					}

					if(txReturn->POWER_AVG_2 > txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper2g || txReturn->POWER_AVG_2 < txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower2g)
					{
						printf("\t[#] POWER_AVG_2              :\t %0.2f dBm\t<-- Failed\n",txReturn->POWER_AVG_2);
						os<<"\t[#] POWER_AVG_2              :\t"<<setprecision(4)<<txReturn->POWER_AVG_2<<" dBm\t<-- Failed"<<endl;
					}
					else						
					{
						printf("\t[#] POWER_AVG_2              :\t %0.2f dBm\n",txReturn->POWER_AVG_2);
						os<<"\t[#] POWER_AVG_2              :\t"<<setprecision(4)<<txReturn->POWER_AVG_2<<" dBm"<<endl;
					}

					if(txReturn->POWER_AVG_3 > txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper2g || txReturn->POWER_AVG_3 < txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower2g)
					{
						printf("\t[#] POWER_AVG_3              :\t %0.2f dBm\t<-- Failed\n",txReturn->POWER_AVG_3);
						os<<"\t[#] POWER_AVG_3              :\t"<<setprecision(4)<<txReturn->POWER_AVG_3<<" dBm\t<-- Failed"<<endl;
					}
					else
					{
						printf("\t[#] POWER_AVG_3              :\t %0.2f dBm\n",txReturn->POWER_AVG_3);
						os<<"\t[#] POWER_AVG_3              :\t"<<setprecision(4)<<txReturn->POWER_AVG_3<<" dBm\t<-- Failed"<<endl;
					}
				}
			}

		}
		else // 5g test item
		{
			// EVM average
			if( (txReturn->EVM_AVG_ALL < txParam->dEvmSpec && txReturn->EVM_AVG_ALL > g_tsGlobalSetting.WifiLimit.dEvmLower) &&
				(txReturn->FREQ_ERROR < g_tsGlobalSetting.WifiLimit.dFreqencyShift && txReturn->FREQ_ERROR > -g_tsGlobalSetting.WifiLimit.dFreqencyShift) &&
				(txReturn->POWER_AVG_1 < txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper5g && txReturn->POWER_AVG_1 > txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower5g) &&
				(txReturn->POWER_AVG_2 < txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper5g && txReturn->POWER_AVG_2 > txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower5g) &&
				(txReturn->POWER_AVG_3 < txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper5g && txReturn->POWER_AVG_3 > txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower5g))
			{
				printf("\t[#] EVM_AVG_1                :\t %0.2f dB\n",txReturn->EVM_AVG_ALL);
				printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\n",txReturn->FREQ_ERROR);
				printf("\t[#] POWER_AVG_1              :\t %0.2f dBm\n",txReturn->POWER_AVG_1);
				printf("\t[#] POWER_AVG_2              :\t %0.2f dBm\n",txReturn->POWER_AVG_2);
				printf("\t[#] POWER_AVG_3              :\t %0.2f dBm\n",txReturn->POWER_AVG_3);
				os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB"<<endl;
				os<<"\t[#] FREQ_ERROR_AVG           :\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm"<<endl;
				os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<" dBm"<<endl;
				os<<"\t[#] POWER_AVG_2              :\t"<<setprecision(4)<<txReturn->POWER_AVG_2<<" dBm"<<endl;
				os<<"\t[#] POWER_AVG_3              :\t"<<setprecision(4)<<txReturn->POWER_AVG_3<<" dBm"<<endl;
				bParseResult=true;
			}
			else
			{
				if(iTestCount == 0)
				{
					// EVM average
					if(txReturn->EVM_AVG_ALL > txParam->dEvmSpec || txReturn->EVM_AVG_ALL <= g_tsGlobalSetting.WifiLimit.dEvmLower)						
					{
						printf("\t[#] EVM_AVG_1                :\t %0.2f dB\t<-- Failed\n",txReturn->EVM_AVG_ALL);
						os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB\t<-- Failed"<<endl;
					}
					else						
					{
						printf("\t[#] EVM_AVG_1                :\t %0.2f dB\n",txReturn->EVM_AVG_ALL);
						os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB"<<endl;
					}

					// Freq
					if(txReturn->FREQ_ERROR > g_tsGlobalSetting.WifiLimit.dFreqencyShift || txReturn->FREQ_ERROR < -g_tsGlobalSetting.WifiLimit.dFreqencyShift)
					{
						printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\t<-- Failed\n",txReturn->FREQ_ERROR);
						os<<"\t[#] FREQ_ERR_ALL:\t\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm\t<-- Failed"<<endl;
					}
					else						
					{
						printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\n",txReturn->FREQ_ERROR);
						os<<"\t[#] FREQ_ERROR_AVG           :\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm"<<endl;
					}

					if(txReturn->POWER_AVG_1 > txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper5g || txReturn->POWER_AVG_1 < txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower5g)
					{
						printf("\t[#] POWER_AVG_1              :\t %0.2f dBm\t<-- Failed\n",txReturn->POWER_AVG_1);
						os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<" dBm\t<-- Failed"<<endl;
					}
					else
					{
						printf("\t[#] POWER_AVG_1              :\t %0.2f dBm\n",txReturn->POWER_AVG_1);
						os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<" dBm"<<endl;
					}

					if(txReturn->POWER_AVG_2 > txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper5g || txReturn->POWER_AVG_2 < txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower5g)
					{
						printf("\t[#] POWER_AVG_2              :\t %0.2f dBm\t<-- Failed\n",txReturn->POWER_AVG_2);
						os<<"\t[#] POWER_AVG_2              :\t"<<setprecision(4)<<txReturn->POWER_AVG_2<<" dBm\t<-- Failed"<<endl;
					}
					else						
					{
						printf("\t[#] POWER_AVG_2              :\t %0.2f dBm\n",txReturn->POWER_AVG_2);
						os<<"\t[#] POWER_AVG_2              :\t"<<setprecision(4)<<txReturn->POWER_AVG_2<<" dBm"<<endl;
					}

					if(txReturn->POWER_AVG_3 > txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper5g || txReturn->POWER_AVG_3 < txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower5g)
					{
						printf("\t[#] POWER_AVG_3              :\t %0.2f dBm\t<-- Failed\n",txReturn->POWER_AVG_3);
						os<<"\t[#] POWER_AVG_3              :\t"<<setprecision(4)<<txReturn->POWER_AVG_3<<" dBm\t<-- Failed"<<endl;
					}
					else
					{
						printf("\t[#] POWER_AVG_3              :\t %0.2f dBm\n",txReturn->POWER_AVG_3);
						os<<"\t[#] POWER_AVG_3              :\t"<<setprecision(4)<<txReturn->POWER_AVG_3<<" dBm\t<-- Failed"<<endl;
					}
				}
			}
		}
		break;
	case 2:
		// Power
		if(txParam->CH_FREQ_MHZ <= 2484) // 2.4g test item
		{
			// EVM average
			if( (txReturn->EVM_AVG_ALL < txParam->dEvmSpec && txReturn->EVM_AVG_ALL > g_tsGlobalSetting.WifiLimit.dEvmLower) &&
				(txReturn->FREQ_ERROR < g_tsGlobalSetting.WifiLimit.dFreqencyShift && txReturn->FREQ_ERROR > -g_tsGlobalSetting.WifiLimit.dFreqencyShift) &&
				(txReturn->POWER_AVG_1 < txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper2g && txReturn->POWER_AVG_1 > txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower2g) &&
				(txReturn->POWER_AVG_2 < txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper2g && txReturn->POWER_AVG_2 > txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower2g))
			{
				printf("\t[#] EVM_AVG_1                :\t %0.2f dB\n",txReturn->EVM_AVG_ALL);
				printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\n",txReturn->FREQ_ERROR);
				printf("\t[#] POWER_AVG_1              :\t %0.2f dBm\n",txReturn->POWER_AVG_1);
				printf("\t[#] POWER_AVG_2              :\t %0.2f dBm\n",txReturn->POWER_AVG_2);
				os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB"<<endl;
				os<<"\t[#] FREQ_ERROR_AVG           :\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm"<<endl;
				os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<" dBm"<<endl;
				os<<"\t[#] POWER_AVG_2              :\t"<<setprecision(4)<<txReturn->POWER_AVG_2<<" dBm"<<endl;
				bParseResult=true;
			}	
			else 
			{
				if(iTestCount == 0)
				{
					// EVM average
					if(txReturn->EVM_AVG_ALL > txParam->dEvmSpec || txReturn->EVM_AVG_ALL <= g_tsGlobalSetting.WifiLimit.dEvmLower)						
					{
						printf("\t[#] EVM_AVG_1                :\t %0.2f dB\t<-- Failed\n",txReturn->EVM_AVG_ALL);
						os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB\t<-- Failed"<<endl;
					}
					else						
					{
						printf("\t[#] EVM_AVG_1                :\t %0.2f dB\n",txReturn->EVM_AVG_ALL);
						os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB"<<endl;
					}

					// Freq
					if(txReturn->FREQ_ERROR > g_tsGlobalSetting.WifiLimit.dFreqencyShift || txReturn->FREQ_ERROR < -g_tsGlobalSetting.WifiLimit.dFreqencyShift)
					{
						printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\t<-- Failed\n",txReturn->FREQ_ERROR);
						os<<"\t[#] FREQ_ERROR_AVG           :\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm\t<-- Failed"<<endl;
					}
					else						
					{
						printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\n",txReturn->FREQ_ERROR);
						os<<"\t[#] FREQ_ERROR_AVG           :\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm"<<endl;
					}

					if(txReturn->POWER_AVG_1 > txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper2g || txReturn->POWER_AVG_1 < txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower2g)
					{
						printf("\t[#] POWER_AVG_1              :\t %0.2f dBm\t<-- Failed\n",txReturn->POWER_AVG_1);
						os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<" dBm\t<-- Failed"<<endl;
					}
					else
					{
						printf("\t[#] POWER_AVG_1              :\t %0.2f\n",txReturn->POWER_AVG_1);
						os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<endl;
					}

					if(txReturn->POWER_AVG_2 > txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper2g || txReturn->POWER_AVG_2 < txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower2g)
					{
						printf("\t[#] POWER_AVG_2              :\t %0.2f dBm\t<-- Failed\n",txReturn->POWER_AVG_2);
						os<<"\t[#] POWER_AVG_2              :\t"<<setprecision(4)<<txReturn->POWER_AVG_2<<" dBm\t<-- Failed"<<endl;
					}
					else						
					{
						printf("\t[#] POWER_AVG_2              :\t %0.2f dBm\n",txReturn->POWER_AVG_2);
						os<<"\t[#] POWER_AVG_2              :\t"<<setprecision(4)<<txReturn->POWER_AVG_2<<" dBm"<<endl;
					}
				}
			}

		}
		else // 5g test item
		{
			// EVM average
			if( (txReturn->EVM_AVG_ALL < txParam->dEvmSpec && txReturn->EVM_AVG_ALL > g_tsGlobalSetting.WifiLimit.dEvmLower) &&
				(txReturn->FREQ_ERROR < g_tsGlobalSetting.WifiLimit.dFreqencyShift && txReturn->FREQ_ERROR > -g_tsGlobalSetting.WifiLimit.dFreqencyShift) &&
				(txReturn->POWER_AVG_1 < txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper5g && txReturn->POWER_AVG_1 > txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower5g) &&
				(txReturn->POWER_AVG_2 < txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper5g && txReturn->POWER_AVG_2 > txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower5g))
			{
				printf("\t[#] EVM_AVG_1                :\t %0.2f dB\n",txReturn->EVM_AVG_ALL);
				printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\n",txReturn->FREQ_ERROR);
				printf("\t[#] POWER_AVG_1              :\t %0.2f dBm\n",txReturn->POWER_AVG_1);
				printf("\t[#] POWER_AVG_2              :\t %0.2f dBm\n",txReturn->POWER_AVG_2);
				os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB"<<endl;
				os<<"\t[#] FREQ_ERROR_AVG           :\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm"<<endl;
				os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<" dBm"<<endl;
				os<<"\t[#] POWER_AVG_2              :\t"<<setprecision(4)<<txReturn->POWER_AVG_2<<" dBm"<<endl;
				bParseResult=true;
			}
			else
			{
				if(iTestCount == 0)
				{
					// EVM average
					if(txReturn->EVM_AVG_ALL > txParam->dEvmSpec || txReturn->EVM_AVG_ALL <= g_tsGlobalSetting.WifiLimit.dEvmLower)						
					{
						printf("\t[#] EVM_AVG_1                :\t %0.2f dB\t<-- Failed\n",txReturn->EVM_AVG_ALL);
						os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB\t<-- Failed"<<endl;
					}
					else						
					{
						printf("\t[#] EVM_AVG_1                :\t %0.2f dB\n",txReturn->EVM_AVG_ALL);
						os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB"<<endl;
					}

					// Freq
					if(txReturn->FREQ_ERROR > g_tsGlobalSetting.WifiLimit.dFreqencyShift || txReturn->FREQ_ERROR < -g_tsGlobalSetting.WifiLimit.dFreqencyShift)
					{
						printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\t<-- Failed\n",txReturn->FREQ_ERROR);
						os<<"\t[#] FREQ_ERR_ALL:\t\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm\t<-- Failed"<<endl;
					}
					else						
					{
						printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\n",txReturn->FREQ_ERROR);
						os<<"\t[#] FREQ_ERROR_AVG           :\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm"<<endl;
					}

					if(txReturn->POWER_AVG_1 > txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper5g || txReturn->POWER_AVG_1 < txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower5g)
					{
						printf("\t[#] POWER_AVG_1              :\t %0.2f dBm\t<-- Failed\n",txReturn->POWER_AVG_1);
						os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<" dBm\t<-- Failed"<<endl;
					}
					else
					{
						printf("\t[#] POWER_AVG_1              :\t %0.2f dBm\n",txReturn->POWER_AVG_1);
						os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<" dBm"<<endl;
					}

					if(txReturn->POWER_AVG_2 > txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper5g || txReturn->POWER_AVG_2 < txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower5g)
					{
						printf("\t[#] POWER_AVG_2              :\t %0.2f dBm\t<-- Failed\n",txReturn->POWER_AVG_2);
						os<<"\t[#] POWER_AVG_2              :\t"<<setprecision(4)<<txReturn->POWER_AVG_2<<" dBm\t<-- Failed"<<endl;
					}
					else						
					{
						printf("\t[#] POWER_AVG_2              :\t %0.2f dBm\n",txReturn->POWER_AVG_2);
						os<<"\t[#] POWER_AVG_2              :\t"<<setprecision(4)<<txReturn->POWER_AVG_2<<" dBm"<<endl;
					}

				}
			}
		}
		break;
	case 1:
		// Power
		if(txParam->CH_FREQ_MHZ <= 2484) // 2.4g test item
		{
			// EVM average
				
			if(strstr(txParam->DATA_RATE,"CCK-11")) // Special for R6250 CCK-11 issue 
			{
				g_tsGlobalSetting.WifiLimit.dPowerUpper2g = 3;
			}
			else
			{
				g_tsGlobalSetting.WifiLimit.dPowerUpper2g = 2;
			}

			if( (txReturn->EVM_AVG_ALL < txParam->dEvmSpec && txReturn->EVM_AVG_ALL > g_tsGlobalSetting.WifiLimit.dEvmLower) &&
				(txReturn->FREQ_ERROR < g_tsGlobalSetting.WifiLimit.dFreqencyShift && txReturn->FREQ_ERROR > -g_tsGlobalSetting.WifiLimit.dFreqencyShift) &&
				(txReturn->POWER_AVG_1 < txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper2g && txReturn->POWER_AVG_1 > txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower2g))
			{
				printf("\t[#] EVM_AVG_1                :\t %0.2f dB\n",txReturn->EVM_AVG_ALL);
				printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\n",txReturn->FREQ_ERROR);
				printf("\t[#] POWER_AVG_1              :\t %0.2f dBm\n",txReturn->POWER_AVG_1);
				os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB"<<endl;
				os<<"\t[#] FREQ_ERROR_AVG           :\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm"<<endl;
				os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<" dBm"<<endl;
				bParseResult=true;
			}	
			else 
			{
				if(iTestCount == 0)
				{
					// EVM average
					if(txReturn->EVM_AVG_ALL > txParam->dEvmSpec || txReturn->EVM_AVG_ALL <= g_tsGlobalSetting.WifiLimit.dEvmLower)						
					{
						printf("\t[#] EVM_AVG_1                :\t %0.2f dB\t<-- Failed\n",txReturn->EVM_AVG_ALL);
						os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB\t<-- Failed"<<endl;
					}
					else						
					{
						printf("\t[#] EVM_AVG_1                :\t %0.2f dB\n",txReturn->EVM_AVG_ALL);
						os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB"<<endl;
					}

					// Freq
					if(txReturn->FREQ_ERROR > g_tsGlobalSetting.WifiLimit.dFreqencyShift || txReturn->FREQ_ERROR < -g_tsGlobalSetting.WifiLimit.dFreqencyShift)
					{
						printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\t<-- Failed\n",txReturn->FREQ_ERROR);
						os<<"\t[#] FREQ_ERROR_AVG           :\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm\t<-- Failed"<<endl;
					}
					else						
					{
						printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\n",txReturn->FREQ_ERROR);
						os<<"\t[#] FREQ_ERROR_AVG           :\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm"<<endl;
					}

					if(txReturn->POWER_AVG_1 > txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper2g || txReturn->POWER_AVG_1 < txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower2g)
					{
						printf("\t[#] POWER_AVG_1              :\t %0.2f dBm\t<-- Failed\n",txReturn->POWER_AVG_1);
						os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<" dBm\t<-- Failed"<<endl;
					}
					else
					{
						printf("\t[#] POWER_AVG_1              :\t %0.2f\n",txReturn->POWER_AVG_1);
						os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<endl;
					}
				}
			}

		}
		else // 5g test item
		{
			// EVM average
			if( (txReturn->EVM_AVG_ALL < txParam->dEvmSpec && txReturn->EVM_AVG_ALL > g_tsGlobalSetting.WifiLimit.dEvmLower) &&
				(txReturn->FREQ_ERROR < g_tsGlobalSetting.WifiLimit.dFreqencyShift && txReturn->FREQ_ERROR > -g_tsGlobalSetting.WifiLimit.dFreqencyShift) &&
				(txReturn->POWER_AVG_1 < txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper5g && txReturn->POWER_AVG_1 > txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower5g))
			{
				printf("\t[#] EVM_AVG_1                :\t %0.2f dB\n",txReturn->EVM_AVG_ALL);
				printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\n",txReturn->FREQ_ERROR);
				printf("\t[#] POWER_AVG_1              :\t %0.2f dBm\n",txReturn->POWER_AVG_1);
				os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB"<<endl;
				os<<"\t[#] FREQ_ERROR_AVG           :\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm"<<endl;
				os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<" dBm"<<endl;
				bParseResult=true;
			}
			else
			{
				if(iTestCount == 0)
				{
					// EVM average
					if(txReturn->EVM_AVG_ALL > txParam->dEvmSpec || txReturn->EVM_AVG_ALL <= g_tsGlobalSetting.WifiLimit.dEvmLower)						
					{
						printf("\t[#] EVM_AVG_1                :\t %0.2f dB\t<-- Failed\n",txReturn->EVM_AVG_ALL);
						os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB\t<-- Failed"<<endl;
					}
					else						
					{
						printf("\t[#] EVM_AVG_1                :\t %0.2f dB\n",txReturn->EVM_AVG_ALL);
						os<<"\t[#] EVM_AVG_1                :\t"<<setprecision(4)<<txReturn->EVM_AVG_ALL<<" dB"<<endl;
					}

					// Freq
					if(txReturn->FREQ_ERROR > g_tsGlobalSetting.WifiLimit.dFreqencyShift || txReturn->FREQ_ERROR < -g_tsGlobalSetting.WifiLimit.dFreqencyShift)
					{
						printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\t<-- Failed\n",txReturn->FREQ_ERROR);
						os<<"\t[#] FREQ_ERR_ALL:\t\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm\t<-- Failed"<<endl;
					}
					else						
					{
						printf("\t[#] FREQ_ERROR_AVG           :\t %0.2f ppm\n",txReturn->FREQ_ERROR);
						os<<"\t[#] FREQ_ERROR_AVG           :\t"<<setprecision(4)<<txReturn->FREQ_ERROR<<" ppm"<<endl;
					}

					if(txReturn->POWER_AVG_1 > txParam->dTargetPower + g_tsGlobalSetting.WifiLimit.dPowerUpper5g || txReturn->POWER_AVG_1 < txParam->dTargetPower - g_tsGlobalSetting.WifiLimit.dPowerLower5g)
					{
						printf("\t[#] POWER_AVG_1              :\t %0.2f dBm\t<-- Failed\n",txReturn->POWER_AVG_1);
						os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<" dBm\t<-- Failed"<<endl;
					}
					else
					{
						printf("\t[#] POWER_AVG_1              :\t %0.2f dBm\n",txReturn->POWER_AVG_1);
						os<<"\t[#] POWER_AVG_1              :\t"<<setprecision(4)<<txReturn->POWER_AVG_1<<" dBm"<<endl;
					}				
				}
			}
		}
		break;
	default:
		break;

	}
	os.close();
	return bParseResult;
}
bool ShowTestResultMask(TX_PARAM_IN *txParam, TX_PARAM_RETURN *txReturn, int iTestCount)
{
	// Mask average	
	ofstream os;
	os.open(_T(".\\log\\Log_all.txt"),ios_base::app|ios_base::out);
	if( txReturn->MASK_PERCENT <= g_tsGlobalSetting.WifiLimit.dMaskPercent && txReturn->MASK_PERCENT >= 0)
	{
		printf("\t[#] POWER_AVERAGE_DBM:\t\t %0.2f dBm\n",txReturn->POWER_AVG_ALL);
		printf("\t[#] VIOLATION_PERCENT:\t\t %0.2f %\n",txReturn->MASK_PERCENT);
		os<<"\t[#] POWER_AVERAGE_DBM:\t\t"<<setprecision(4)<<txReturn->POWER_AVG_ALL<<" dBm"<<endl;
		os<<"\t[#] VIOLATION_PERCENT:\t\t"<<setprecision(4)<<txReturn->MASK_PERCENT<<" %"<<endl;
		os.close();
		return true;
	}	
	else 
	{
		if(iTestCount == 0)
		{
			// MASK average
			printf("\t[#] POWER_AVERAGE_DBM:\t\t %0.2f dBm\n",txReturn->POWER_AVG_ALL);
			printf("\t[#] VIOLATION_PERCENT:\t\t %0.2f %\t<-- Failed\n",txReturn->MASK_PERCENT);
			os<<"\t[#] POWER_AVERAGE_DBM:\t\t"<<setprecision(4)<<txReturn->POWER_AVG_ALL<<" dBm"<<endl;
			os<<"\t[#] VIOLATION_PERCENT:\t\t"<<setprecision(4)<<txReturn->MASK_PERCENT<<" %\t<-- Failed"<<endl;
		}
	}

	os.close();
	return false;
}
bool ShowTestResultPower(TX_PARAM_IN *txParam, TX_PARAM_RETURN *txReturn, int iTestCount)
{
	// Power	
	ofstream os;
	os.open(_T(".\\log\\Log_all.txt"),ios_base::app|ios_base::out);

	double dOffset=0.0;
	int nAntCount=txParam->nTx1+txParam->nTx2+txParam->nTx3+txParam->nTx4;
	switch (nAntCount)
	{
	case 2:
		dOffset=3.0;
		break;
	case 3:
		dOffset=4.75;
		break;
	default:
		break;
	}
	if(txParam->CH_FREQ_MHZ >=2484)
	{
		if( txReturn->POWER_AVG_ALL <= txParam->dTargetPower+dOffset+g_tsGlobalSetting.WifiLimit.dPowerUpper2g && 
			txReturn->POWER_AVG_ALL >= txParam->dTargetPower+dOffset-g_tsGlobalSetting.WifiLimit.dPowerLower2g)
		{
			printf("\t[#] POWER_AVERAGE_DBM:\t\t %0.2f dBm\n",txReturn->POWER_AVG_ALL);
			os<<"\t[#] POWER_AVERAGE_DBM:\t\t"<<setprecision(4)<<txReturn->POWER_AVG_ALL<<" dBm"<<endl;
			os.close();
			return true;
		}	
		else 
		{
			if(iTestCount == 0)
			{
				printf("\t[#] POWER_AVERAGE_DBM:\t\t %0.2f dBm\t<-- Failed\n",txReturn->POWER_AVG_ALL);
				os<<"\t[#] POWER_AVERAGE_DBM:\t\t"<<setprecision(4)<<txReturn->POWER_AVG_ALL<<" dBm\t<-- Failed"<<endl;
			}
		}	
	}
	else
	{
		if( txReturn->POWER_AVG_ALL <= txParam->dTargetPower+dOffset+g_tsGlobalSetting.WifiLimit.dPowerUpper5g && 
			txReturn->POWER_AVG_ALL >= txParam->dTargetPower+dOffset-g_tsGlobalSetting.WifiLimit.dPowerLower5g)
		{
			printf("\t[#] POWER_AVERAGE_DBM:\t\t %0.2f dBm\n",txReturn->POWER_AVG_ALL);
			os<<"\t[#] POWER_AVERAGE_DBM:\t\t"<<setprecision(4)<<txReturn->POWER_AVG_ALL<<" dBm"<<endl;
			os.close();
			return true;
		}	
		else 
		{
			if(iTestCount == 0)
			{
				printf("\t[#] POWER_AVERAGE_DBM:\t\t %0.2f dBm\t<-- Failed\n",txReturn->POWER_AVG_ALL);
				os<<"\t[#] POWER_AVERAGE_DBM:\t\t"<<setprecision(4)<<txReturn->POWER_AVG_ALL<<" dBm\t<-- Failed"<<endl;			
			}
		}
	}
	os.close ();
	return false;
}

bool ShowTestResultPER(RX_PARAM_IN *rxParam, RX_PARAM_RETURN *rxReturn, int iTestCount)
{
	// PER average		
	ofstream os;
	os.open(_T(".\\log\\Log_all.txt"),ios_base::app|ios_base::out);

	if(strstr(rxParam->DATA_RATE,"OFDM") || strstr(rxParam->DATA_RATE,"MCS"))
	{
		if( rxReturn->dPerPercent <= g_tsGlobalSetting.WifiLimit.dPerPercentOFDM && rxReturn->dPerPercent >= 0)
		{
			printf("\t[#] PER           :\t\t %0.2f %%\n",rxReturn->dPerPercent);
			printf("\t[#] RX_POWER_LEVEL:\t\t %0.2f dBm\n",rxReturn->dRxPowerLevel);
			os<<"\t[#] PER           :\t\t"<<setprecision(4)<<rxReturn->dPerPercent<<" %"<<endl;
			os<<"\t[#] RX_POWER_LEVEL:\t\t"<<setprecision(4)<<rxReturn->dRxPowerLevel<<" dBm"<<endl;
			os.close();
			return true;
		}	
		else 
		{
			if(iTestCount == 0)
			{
				printf("\t[#] PER           :\t\t %0.2f %s\t<-- Failed\n",rxReturn->dPerPercent,"%");
				printf("\t[#] RX_POWER_LEVEL:\t\t %0.2f\n",rxReturn->dRxPowerLevel );

				os<<"\t[#] PER           :\t\t"<<setprecision(4)<<rxReturn->dPerPercent<<" %\t<-- Failed"<<endl;
				os<<"\t[#] RX_POWER_LEVEL:\t\t"<<setprecision(4)<<rxReturn->dRxPowerLevel<<" dBm"<<endl;
			}
		}
	}
	else
	{
		if( rxReturn->dPerPercent <= g_tsGlobalSetting.WifiLimit.dPerPercent && rxReturn->dPerPercent >= 0)
		{
			printf("\t[#] PER           :\t\t %0.2f %s\n",rxReturn->dPerPercent,"%");
			printf("\t[#] RX_POWER_LEVEL:\t\t %0.2f dBm\n",rxReturn->dRxPowerLevel);
			os<<"\t[#] PER           :\t\t"<<setprecision(4)<<rxReturn->dPerPercent<<" %"<<endl;
			os<<"\t[#] RX_POWER_LEVEL:\t\t"<<setprecision(4)<<rxReturn->dRxPowerLevel<<" dBm"<<endl;
			os.close();
			return true;
		}	
		else 
		{
			if(iTestCount == 0)
			{
				printf("\t[#] PER           :\t\t %0.2f %s\t <-- Failed\n",rxReturn->dPerPercent,"%");
				printf("\t[#] RX_POWER_LEVEL:\t\t %0.2f dBm\n",rxReturn->dRxPowerLevel );
				os<<"\t[#] PER           :\t\t"<<setprecision(4)<<rxReturn->dPerPercent<<" %\t<-- Failed"<<endl;
				os<<"\t[#] RX_POWER_LEVEL:\t\t"<<setprecision(4)<<rxReturn->dRxPowerLevel<<" dBm"<<endl;
			}
		}	
	}
	os.close();	
	return false;
}

int myprintf(const char* pstr)
{
	ofstream os;
	os.open(_T(".\\log\\Log_all.txt"),ios_base::app|ios_base::out);
	os<<endl;
	os<<pstr<<endl;
	os.close();
	return 0;
}