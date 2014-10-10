
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
	g_tsGlobalSetting.ANALYSIS_11AG_AMPL_TRACK  = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11AG_AMPL_TRACK"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11AG_CH_ESTIMATE  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11AG_CH_ESTIMATE"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11AG_FREQ_SYNC  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11AG_FREQ_SYNC"),2,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11AG_PH_CORR_MODE  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11AG_PH_CORR_MODE"),2,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11AG_SYM_TIM_CORR  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11AG_SYM_TIM_CORR"),2,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11B_DC_REMOVE_FLAG  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11B_DC_REMOVE_FLAG"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11B_EQ_TAPS  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11B_EQ_TAPS"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11B_FIXED_01_DATA_SEQUENCE  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11B_FIXED_01_DATA_SEQUENCE"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11B_METHOD_11B  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11B_METHOD_11B"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11N_AMPLITUDE_TRACKING  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11N_AMPLITUDE_TRACKING"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11N_DECODE_PSDU  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11N_DECODE_PSDU"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11N_FREQUENCY_CORRELATION  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11N_FREQUENCY_CORRELATION"),2,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11N_FULL_PACKET_CHANNEL_EST"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11N_PHASE_CORR  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11N_PHASE_CORR"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.ANALYSIS_11N_SYM_TIMING_CORR  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("ANALYSIS_11N_SYM_TIMING_CORR"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.DUT_KEEP_TRANSMIT  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("DUT_KEEP_TRANSMIT"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.DUT_RX_SETTLE_TIME_MS  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("DUT_RX_SETTLE_TIME_MS"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.DUT_TX_SETTLE_TIME_MS  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("DUT_TX_SETTLE_TIME_MS"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_11AG_SAMPLE_INTERVAL_US  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_11AG_SAMPLE_INTERVAL_US"),95,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_11B_L_SAMPLE_INTERVAL_US  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_11B_L_SAMPLE_INTERVAL_US"),286,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_11B_S_SAMPLE_INTERVAL_US  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_11B_S_SAMPLE_INTERVAL_US"),190,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_11N_GREENFIELD_SAMPLE_INTERVAL_US  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_11N_GREENFIELD_SAMPLE_INTERVAL_US"),115,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_11N_MIXED_SAMPLE_INTERVAL_US  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_11N_MIXED_SAMPLE_INTERVAL_US"),123,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_AVERAGE  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_AVERAGE"),3,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_SYMBOL_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("EVM_SYMBOL_NUM"),18,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ2010_EXT_ENABLE  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ2010_EXT_ENABLE"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11AG_12M  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11AG_12M"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11AG_18M  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11AG_18M"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11AG_24M  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11AG_24M"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11AG_36M  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11AG_36M"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11AG_48M  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11AG_48M"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11AG_54M  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11AG_54M"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11AG_6M  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11AG_6M"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11AG_9M  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11AG_9M"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11B_11M  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11B_11M"),3,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11B_1M  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11B_1M"),3,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11B_2M  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11B_2M"),3,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11B_5_5M  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11B_5_5M"),3,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11B_CH14  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11B_CH14"),5,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11N_MCS0  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11N_MCS0"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11N_MCS1  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11N_MCS1"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11N_MCS2  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11N_MCS2"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11N_MCS3  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11N_MCS3"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11N_MCS4  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11N_MCS4"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11N_MCS5  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11N_MCS5"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11N_MCS6  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11N_MCS6"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.IQ_P_TO_A_11N_MCS7  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("IQ_P_TO_A_11N_MCS7"),10,szFilenameGlobalSetting);
	g_tsGlobalSetting.MASK_DSSS_SAMPLE_INTERVAL_US  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("MASK_DSSS_SAMPLE_INTERVAL_US"),286,szFilenameGlobalSetting);
	g_tsGlobalSetting.MASK_OFDM_SAMPLE_INTERVAL_US  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("MASK_OFDM_SAMPLE_INTERVAL_US"),95,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11AG_12_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11AG_12_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11AG_18_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11AG_18_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11AG_24_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11AG_24_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11AG_36_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11AG_36_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11AG_48_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11AG_48_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11AG_54_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11AG_54_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11AG_6_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11AG_6_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11AG_9_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11AG_9_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11B_11_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11B_11_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11B_1_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11B_1_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11B_2_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11B_2_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11B_5_5_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11B_5_5_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS0_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS0_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS10_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS10_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS11_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS11_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS12_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS12_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS13_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS13_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS14_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS14_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS15_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS15_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS16_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS16_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS17_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS17_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS18_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS18_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS19_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS19_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS1_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS1_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS20_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS20_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS21_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS21_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS22_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS22_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS23_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS23_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS24_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS24_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS25_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS25_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS26_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS26_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS27_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS27_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS28_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS28_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS29_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS29_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS2_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS2_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS30_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS30_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS31_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS31_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS3_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS3_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS4_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS4_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS5_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS5_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS6_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS6_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS7_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS7_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS8_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS8_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS9_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS9_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS0_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS0_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS10_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS10_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS11_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS11_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS12_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS12_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS13_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS13_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS14_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS14_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS15_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS15_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS16_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS16_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS17_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS17_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS18_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS18_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS19_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS19_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS1_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS1_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS20_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS20_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS21_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS21_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS22_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS22_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS23_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS23_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS24_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS24_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS25_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS25_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS26_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS26_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS27_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS27_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS28_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS28_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS29_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS29_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS2_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS2_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS30_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS30_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS31_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS31_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS3_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS3_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS4_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS4_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS5_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS5_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS6_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS6_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS7_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS7_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS8_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS8_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS9_PACKETS_NUM  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS9_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_VSG_TIMEOUT_SEC  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PER_VSG_TIMEOUT_SEC"),20,szFilenameGlobalSetting);
	g_tsGlobalSetting.PM_AVERAGE  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PM_AVERAGE"),3,szFilenameGlobalSetting);
	g_tsGlobalSetting.PM_DSSS_SAMPLE_INTERVAL_US  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PM_DSSS_SAMPLE_INTERVAL_US"),100,szFilenameGlobalSetting);
	g_tsGlobalSetting.PM_IF_FREQ_SHIFT_MHZ  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PM_IF_FREQ_SHIFT_MHZ"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.PM_OFDM_SAMPLE_INTERVAL_US  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("PM_OFDM_SAMPLE_INTERVAL_US"),20,szFilenameGlobalSetting);
	g_tsGlobalSetting.SPECTRUM_DSSS_SAMPLE_INTERVAL_US  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("SPECTRUM_DSSS_SAMPLE_INTERVAL_US"),286,szFilenameGlobalSetting);
	g_tsGlobalSetting.SPECTRUM_OFDM_SAMPLE_INTERVAL_US  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("SPECTRUM_OFDM_SAMPLE_INTERVAL_US"),95,szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_PORT  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("VSA_PORT"),2,szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_SAVE_CAPTURE_ALWAYS  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("VSA_SAVE_CAPTURE_ALWAYS"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_SAVE_CAPTURE_ON_FAILED  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("VSA_SAVE_CAPTURE_ON_FAILED"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_SKIP_PACKET_COUNT  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("VSA_SKIP_PACKET_COUNT"),100,szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_TRIGGER_TIMEOUT_SEC  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("VSA_TRIGGER_TIMEOUT_SEC"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_TRIGGER_TYPE  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("VSA_TRIGGER_TYPE"),6,szFilenameGlobalSetting);
	g_tsGlobalSetting.VSG_PORT  = 	GetPrivateProfileInt(_T("GLOBAL_SETTINGS"),_T("VSG_PORT"),2,szFilenameGlobalSetting);

	//new add int parameters IQLite_Core_3.0.1
	g_tsGlobalSetting.AUTO_GAIN_CONTROL_ENABLE = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("AUTO_GAIN_CONTROL_ENABLE"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.AUTO_READING_LIMIT = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("AUTO_READING_LIMIT"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_CAPTURE_TIME_11AG_US = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("EVM_CAPTURE_TIME_11AG_US"),95,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_CAPTURE_TIME_11B_L_US = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("EVM_CAPTURE_TIME_11B_L_US"),286,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_CAPTURE_TIME_11B_S_US = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("EVM_CAPTURE_TIME_11B_S_US"),190,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_CAPTURE_TIME_11N_GREENFIELD_US = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("EVM_CAPTURE_TIME_11N_GREENFIELD_US"),115,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_CAPTURE_TIME_11N_MIXED_US = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("EVM_CAPTURE_TIME_11N_MIXED_US"),123,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_CAPTURE_TIME_11P_HALF_US = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("EVM_CAPTURE_TIME_11P_HALF_US"),187,szFilenameGlobalSetting);
	g_tsGlobalSetting.EVM_CAPTURE_TIME_11P_QUAR_US = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("EVM_CAPTURE_TIME_11P_QUAR_US"),371,szFilenameGlobalSetting);
	g_tsGlobalSetting.MASK_FFT_AVERAGE = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("MASK_FFT_AVERAGE"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.MASK_HALF_SAMPLE_INTERVAL_US = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("MASK_HALF_SAMPLE_INTERVAL_US"),187,szFilenameGlobalSetting);
	g_tsGlobalSetting.MASK_QUAR_SAMPLE_INTERVAL_US = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("MASK_QUAR_SAMPLE_INTERVAL_US"),371,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS0_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS0_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS10_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS10_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS11_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS11_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS12_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS12_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS13_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS13_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS14_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS14_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS15_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS15_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS16_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS16_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS17_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS17_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS18_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS18_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS19_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS19_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS1_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS1_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS20_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS20_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS21_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS21_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS22_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS22_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS23_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS23_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS24_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS24_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS25_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS25_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS26_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS26_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS27_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS27_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS28_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS28_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS29_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS29_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS2_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS2_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS30_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS30_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS31_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS31_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS3_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS3_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS4_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS4_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS5_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS5_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS6_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS6_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS7_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS7_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS8_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS8_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS9_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_L_GI_MCS9_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS0_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS0_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS10_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS10_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS11_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS11_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS12_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS12_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS13_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS13_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS14_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS14_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS15_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS15_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS16_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS16_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS17_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS17_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS18_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS18_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS19_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS19_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS1_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS1_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS20_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS20_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS21_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS21_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS22_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS22_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS23_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS23_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS24_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS24_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS25_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS25_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS26_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS26_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS27_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS27_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS28_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS28_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS29_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS29_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS2_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS2_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS30_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS30_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS31_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS31_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS3_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS3_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS4_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS4_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS5_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS5_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS6_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS6_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS7_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS7_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS8_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS8_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS9_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_GREENFIELD_S_GI_MCS9_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS0_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS0_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS10_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS10_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS11_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS11_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS12_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS12_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS13_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS13_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS14_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS14_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS15_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS15_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS16_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS16_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS17_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS17_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS18_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS18_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS19_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS19_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS1_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS1_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS20_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS20_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS21_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS21_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS22_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS22_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS23_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS23_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS24_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS24_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS25_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS25_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS26_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS26_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS27_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS27_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS28_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS28_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS29_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS29_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS2_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS2_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS30_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS30_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS31_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS31_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS3_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS3_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS4_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS4_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS5_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS5_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS6_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS6_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS7_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS7_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS8_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS8_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS9_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT20_MIXED_S_GI_MCS9_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS0_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS0_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS10_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS10_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS11_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS11_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS12_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS12_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS13_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS13_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS14_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS14_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS15_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS15_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS16_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS16_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS17_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS17_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS18_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS18_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS19_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS19_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS1_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS1_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS20_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS20_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS21_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS21_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS22_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS22_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS23_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS23_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS24_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS24_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS25_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS25_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS26_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS26_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS27_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS27_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS28_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS28_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS29_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS29_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS2_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS2_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS30_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS30_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS31_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS31_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS3_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS3_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS4_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS4_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS5_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS5_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS6_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS6_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS7_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS7_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS8_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS8_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS9_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_L_GI_MCS9_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS0_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS0_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS10_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS10_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS11_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS11_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS12_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS12_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS13_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS13_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS14_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS14_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS15_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS15_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS16_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS16_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS17_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS17_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS18_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS18_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS19_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS19_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS1_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS1_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS20_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS20_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS21_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS21_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS22_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS22_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS23_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS23_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS24_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS24_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS25_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS25_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS26_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS26_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS27_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS27_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS28_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS28_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS29_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS29_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS2_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS2_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS30_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS30_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS31_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS31_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS3_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS3_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS4_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS4_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS5_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS5_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS6_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS6_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS7_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS7_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS8_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS8_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS9_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_GREENFIELD_S_GI_MCS9_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS0_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS0_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS10_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS10_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS11_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS11_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS12_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS12_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS13_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS13_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS14_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS14_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS15_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS15_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS16_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS16_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS17_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS17_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS18_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS18_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS19_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS19_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS1_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS1_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS20_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS20_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS21_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS21_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS22_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS22_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS23_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS23_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS24_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS24_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS25_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS25_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS26_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS26_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS27_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS27_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS28_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS28_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS29_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS29_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS2_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS2_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS30_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS30_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS31_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS31_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS3_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS3_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS4_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS4_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS5_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS5_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS6_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS6_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS7_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS7_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS8_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS8_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS9_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11N_HT40_MIXED_S_GI_MCS9_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11P_HALF12_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11P_HALF-12_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11P_HALF18_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11P_HALF-18_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11P_HALF24_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11P_HALF-24_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11P_HALF27_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11P_HALF-27_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11P_HALF3_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11P_HALF-3_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11P_HALF4_5_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11P_HALF-4_5_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11P_HALF6_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11P_HALF-6_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11P_HALF9_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11P_HALF-9_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11P_QUAR12_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11P_QUAR-12_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11P_QUAR13_5_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11P_QUAR-13_5_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11P_QUAR1_5_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11P_QUAR-1_5_PACKETS_NUM"),200,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11P_QUAR2_25_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11P_QUAR-2_25_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11P_QUAR3_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11P_QUAR-3_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11P_QUAR4_5_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11P_QUAR-4_5_PACKETS_NUM"),500,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11P_QUAR6_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11P_QUAR-6_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PER_11P_QUAR9_PACKETS_NUM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PER_11P_QUAR-9_PACKETS_NUM"),1000,szFilenameGlobalSetting);
	g_tsGlobalSetting.PM_HALF_SAMPLE_INTERVAL_US = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PM_HALF_SAMPLE_INTERVAL_US"),40,szFilenameGlobalSetting);
	g_tsGlobalSetting.PM_QUAR_SAMPLE_INTERVAL_US = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("PM_QUAR_SAMPLE_INTERVAL_US"),80,szFilenameGlobalSetting);
	g_tsGlobalSetting.RELATIVE_LIMIT = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("RELATIVE_LIMIT"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.RESET_TEST_ITEM_DURING_RETRY = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("RESET_TEST_ITEM_DURING_RETRY"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.RETRY_ERROR_ITEMS = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("RETRY_ERROR_ITEMS"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.RETRY_TEST_ITEM = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("RETRY_TEST_ITEM"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.SPECTRUM_HALF_SAMPLE_INTERVAL_US = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("SPECTRUM_HALF_SAMPLE_INTERVAL_US"),187,szFilenameGlobalSetting);
	g_tsGlobalSetting.SPECTRUM_QUAR_SAMPLE_INTERVAL_US = GetPrivateProfileInt(_T("GLOBAL_SETTINGS"), _T("SPECTRUM_QUAR_SAMPLE_INTERVAL_US"),371,szFilenameGlobalSetting);

	//new add end
	
	// double type
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSA_ACK_POWER_RMS_DBM"),_T("15.00"),szTemp,sizeof(szTemp),szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_ACK_POWER_RMS_DBM = _wtof(szTemp);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSA_ACK_TRIGGER_LEVEL_DBM"),_T("10.00"),szTemp,sizeof(szTemp),szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_ACK_TRIGGER_LEVEL_DBM = _wtof(szTemp);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSA_AMPLITUDE_TOLERANCE_DB"),_T("3.00"),szTemp,sizeof(szTemp),szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_AMPLITUDE_TOLERANCE_DB = _wtof(szTemp);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSA_PRE_TRIGGER_TIME_US"),_T("3.00"),szTemp,sizeof(szTemp),szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_PRE_TRIGGER_TIME_US = _wtof(szTemp);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSA_TRIGGER_LEVEL_DB"),_T("-25.00"),szTemp,sizeof(szTemp),szFilenameGlobalSetting);
	g_tsGlobalSetting.VSA_TRIGGER_LEVEL_DB = _wtof(szTemp);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSG_MAX_POWER_11B"),_T("0.00"),szTemp,sizeof(szTemp),szFilenameGlobalSetting);
	g_tsGlobalSetting.VSG_MAX_POWER_11B = _wtof(szTemp);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSG_MAX_POWER_11G"),_T("-5.00"),szTemp,sizeof(szTemp),szFilenameGlobalSetting);
	g_tsGlobalSetting.VSG_MAX_POWER_11G = _wtof(szTemp);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSG_MAX_POWER_11N"),_T("-5.00"),szTemp,sizeof(szTemp),szFilenameGlobalSetting);
	g_tsGlobalSetting.VSG_MAX_POWER_11N = _wtof(szTemp);

	// string type
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_MCS10_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS10.ref"), g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS10_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_MCS11_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS11.ref"), g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS11_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_MCS12_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS12.ref"), g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS12_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_MCS13_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS13.ref"), g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS13_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_MCS14_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS14.ref"), g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS14_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_MCS15_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS15.ref"), g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS15_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_MCS16_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS16.ref"), g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS16_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_MCS17_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS17.ref"), g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS17_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_MCS18_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS18.ref"), g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS18_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_MCS19_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS19.ref"), g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS19_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_MCS20_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS20.ref"), g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS20_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_MCS21_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS21.ref"), g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS21_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_MCS22_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS22.ref"), g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS22_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_MCS23_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS23.ref"), g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS23_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_MCS8_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS8.ref"), g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS8_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_MCS9_REFERENCE_FILE_NAME"),_T("AP_HT20_MCS9.ref"), g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS9_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_MCS10_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS10.ref"), g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS10_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_MCS11_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS11.ref"), g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS11_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_MCS12_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS12.ref"), g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS12_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_MCS13_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS13.ref"), g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS13_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_MCS14_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS14.ref"), g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS14_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_MCS15_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS15.ref"), g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS15_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_MCS16_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS16.ref"), g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS16_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_MCS17_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS17.ref"), g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS17_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_MCS18_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS18.ref"), g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS18_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_MCS19_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS19.ref"), g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS19_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_MCS20_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS20.ref"), g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS20_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_MCS21_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS21.ref"), g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS21_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_MCS22_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS22.ref"), g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS22_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_MCS23_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS23.ref"), g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS23_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_MCS8_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS8.ref"), g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS8_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_MCS9_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS9.ref"), g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS9_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS10_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS10.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS10_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS11_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS11.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS11_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS12_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS12.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS12_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS13_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS13.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS13_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS14_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS14.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS14_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS15_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS15.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS15_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS16_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS16.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS16_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS17_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS17.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS17_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS18_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS18.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS18_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS19_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS19.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS19_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS20_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS20.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS20_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS21_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS21.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS21_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS22_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS22.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS22_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS23_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS23.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS23_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS8_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS8.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS8_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS9_REFERENCE_FILE_NAME"),_T("WiFi_HT20_MCS9.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS9_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS10_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS10.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS10_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS11_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS11.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS11_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS12_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS12.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS12_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS13_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS13.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS13_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS14_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS14.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS14_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS15_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS15.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS15_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS16_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS16.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS16_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS17_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS17.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS17_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS18_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS18.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS18_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS19_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS19.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS19_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS20_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS20.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS20_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS21_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS21.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS21_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS22_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS22.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS22_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS23_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS23.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS23_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS8_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS8.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS8_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS9_REFERENCE_FILE_NAME"),_T("WiFi_HT40_MCS9.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS9_REFERENCE_FILE_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11AG_12_WAVEFORM_NAME"),_T("WiFi_OFDM-12.mod"),g_tsGlobalSetting.PER_11AG_12_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11AG_18_WAVEFORM_NAME"),_T("WiFi_OFDM-18.mod"),g_tsGlobalSetting.PER_11AG_18_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11AG_24_WAVEFORM_NAME"),_T("WiFi_OFDM-24.mod"),g_tsGlobalSetting.PER_11AG_24_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11AG_36_WAVEFORM_NAME"),_T("WiFi_OFDM-36.mod"),g_tsGlobalSetting.PER_11AG_36_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11AG_48_WAVEFORM_NAME"),_T("WiFi_OFDM-48.mod"),g_tsGlobalSetting.PER_11AG_48_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11AG_54_WAVEFORM_NAME"),_T("WiFi_OFDM-54.mod"),g_tsGlobalSetting.PER_11AG_54_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11AG_6_WAVEFORM_NAME"),_T("WiFi_OFDM-6.mod"),g_tsGlobalSetting.PER_11AG_6_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11AG_9_WAVEFORM_NAME"),_T("WiFi_OFDM-9.mod"),g_tsGlobalSetting.PER_11AG_9_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11B_11L_WAVEFORM_NAME"),_T("WiFi_CCK-11L.mod"),g_tsGlobalSetting.PER_11B_11L_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11B_11S_WAVEFORM_NAME"),_T("WiFi_CCK-11S.mod"),g_tsGlobalSetting.PER_11B_11S_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11B_1_WAVEFORM_NAME"),_T("WiFi_DSSS-1.mod"),g_tsGlobalSetting.PER_11B_1_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11B_2L_WAVEFORM_NAME"),_T("WiFi_DSSS-2L.mod"),g_tsGlobalSetting.PER_11B_2L_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11B_2S_WAVEFORM_NAME"),_T("WiFi_DSSS-2S.mod"),g_tsGlobalSetting.PER_11B_2S_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11B_5_5L_WAVEFORM_NAME"),_T("WiFi_CCK-5_5L.mod"),g_tsGlobalSetting.PER_11B_5_5L_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11B_5_5S_WAVEFORM_NAME"),_T("WiFi_CCK-5_5S.mod"),g_tsGlobalSetting.PER_11B_5_5S_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS0_WAVEFORM_NAME"),_T("WiFi_HT20_MCS0.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS0_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS10_WAVEFORM_NAME"),_T("WiFi_HT20_MCS10.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS10_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS11_WAVEFORM_NAME"),_T("WiFi_HT20_MCS11.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS11_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS12_WAVEFORM_NAME"),_T("WiFi_HT20_MCS12.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS12_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS13_WAVEFORM_NAME"),_T("WiFi_HT20_MCS13.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS13_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS14_WAVEFORM_NAME"),_T("WiFi_HT20_MCS14.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS14_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS15_WAVEFORM_NAME"),_T("WiFi_HT20_MCS15.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS15_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS16_WAVEFORM_NAME"),_T("WiFi_HT20_MCS16.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS16_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS17_WAVEFORM_NAME"),_T("WiFi_HT20_MCS17.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS17_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS18_WAVEFORM_NAME"),_T("WiFi_HT20_MCS18.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS18_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS19_WAVEFORM_NAME"),_T("WiFi_HT20_MCS19.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS19_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS1_WAVEFORM_NAME"),_T("WiFi_HT20_MCS1.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS1_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS20_WAVEFORM_NAME"),_T("WiFi_HT20_MCS20.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS20_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS21_WAVEFORM_NAME"),_T("WiFi_HT20_MCS21.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS21_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS22_WAVEFORM_NAME"),_T("WiFi_HT20_MCS22.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS22_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS23_WAVEFORM_NAME"),_T("WiFi_HT20_MCS23.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS23_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS24_WAVEFORM_NAME"),_T("WiFi_HT20_MCS24.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS24_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS25_WAVEFORM_NAME"),_T("WiFi_HT20_MCS25.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS25_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS26_WAVEFORM_NAME"),_T("WiFi_HT20_MCS26.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS26_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS27_WAVEFORM_NAME"),_T("WiFi_HT20_MCS27.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS27_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS28_WAVEFORM_NAME"),_T("WiFi_HT20_MCS28.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS28_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS29_WAVEFORM_NAME"),_T("WiFi_HT20_MCS29.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS29_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS2_WAVEFORM_NAME"),_T("WiFi_HT20_MCS2.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS2_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS30_WAVEFORM_NAME"),_T("WiFi_HT20_MCS30.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS30_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS31_WAVEFORM_NAME"),_T("WiFi_HT20_MCS31.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS31_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS3_WAVEFORM_NAME"),_T("WiFi_HT20_MCS3.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS3_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS4_WAVEFORM_NAME"),_T("WiFi_HT20_MCS4.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS4_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS5_WAVEFORM_NAME"),_T("WiFi_HT20_MCS5.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS5_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS6_WAVEFORM_NAME"),_T("WiFi_HT20_MCS6.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS6_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS7_WAVEFORM_NAME"),_T("WiFi_HT20_MCS7.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS7_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS8_WAVEFORM_NAME"),_T("WiFi_HT20_MCS8.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS8_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_MCS9_WAVEFORM_NAME"),_T("WiFi_HT20_MCS9.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS9_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS0_WAVEFORM_NAME"),_T("WiFi_HT40_MCS0.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS0_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS10_WAVEFORM_NAME"),_T("WiFi_HT40_MCS10.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS10_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS11_WAVEFORM_NAME"),_T("WiFi_HT40_MCS11.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS11_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS12_WAVEFORM_NAME"),_T("WiFi_HT40_MCS12.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS12_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS13_WAVEFORM_NAME"),_T("WiFi_HT40_MCS13.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS13_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS14_WAVEFORM_NAME"),_T("WiFi_HT40_MCS14.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS14_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS15_WAVEFORM_NAME"),_T("WiFi_HT40_MCS15.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS15_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS16_WAVEFORM_NAME"),_T("WiFi_HT40_MCS16.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS16_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS17_WAVEFORM_NAME"),_T("WiFi_HT40_MCS17.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS17_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS18_WAVEFORM_NAME"),_T("WiFi_HT40_MCS18.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS18_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS19_WAVEFORM_NAME"),_T("WiFi_HT40_MCS19.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS19_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS1_WAVEFORM_NAME"),_T("WiFi_HT40_MCS1.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS1_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS20_WAVEFORM_NAME"),_T("WiFi_HT40_MCS20.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS20_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS21_WAVEFORM_NAME"),_T("WiFi_HT40_MCS21.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS21_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS22_WAVEFORM_NAME"),_T("WiFi_HT40_MCS22.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS22_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS23_WAVEFORM_NAME"),_T("WiFi_HT40_MCS23.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS23_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS24_WAVEFORM_NAME"),_T("WiFi_HT40_MCS24.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS24_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS25_WAVEFORM_NAME"),_T("WiFi_HT40_MCS25.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS25_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS26_WAVEFORM_NAME"),_T("WiFi_HT40_MCS26.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS26_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS27_WAVEFORM_NAME"),_T("WiFi_HT40_MCS27.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS27_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS28_WAVEFORM_NAME"),_T("WiFi_HT40_MCS28.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS28_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS29_WAVEFORM_NAME"),_T("WiFi_HT40_MCS29.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS29_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS2_WAVEFORM_NAME"),_T("WiFi_HT40_MCS2.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS2_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS30_WAVEFORM_NAME"),_T("WiFi_HT40_MCS30.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS30_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS31_WAVEFORM_NAME"),_T("WiFi_HT40_MCS31.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS31_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS3_WAVEFORM_NAME"),_T("WiFi_HT40_MCS3.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS3_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS4_WAVEFORM_NAME"),_T("WiFi_HT40_MCS4.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS4_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS5_WAVEFORM_NAME"),_T("WiFi_HT40_MCS5.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS5_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS6_WAVEFORM_NAME"),_T("WiFi_HT40_MCS6.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS6_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS7_WAVEFORM_NAME"),_T("WiFi_HT40_MCS7.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS7_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS8_WAVEFORM_NAME"),_T("WiFi_HT40_MCS8.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS8_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_MCS9_WAVEFORM_NAME"),_T("WiFi_HT40_MCS9.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS9_WAVEFORM_NAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_WAVEFORM_DESTINATION_MAC"),_T("010000C0FFEE"),g_tsGlobalSetting.PER_WAVEFORM_DESTINATION_MAC,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_WAVEFORM_PATH"),_T("../mod"),g_tsGlobalSetting.PER_WAVEFORM_PATH,BUFFER_SIZE,szFilenameGlobalSetting);


	// new add string type parameters , IQLite_Core_3.0.1
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS10_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS10.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS10_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS11_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS11.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS11_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS12_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS12.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS12_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS13_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS13.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS13_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS14_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS14.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS14_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS15_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS15.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS15_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS16_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS16.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS16_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS17_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS17.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS17_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS18_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS18.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS18_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS19_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS19.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS19_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS20_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS20.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS20_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS21_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS21.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS21_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS22_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS22.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS22_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS23_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS23.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS23_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS24_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS24.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS24_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS25_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS25.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS25_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS26_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS26.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS26_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS27_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS27.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS27_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS28_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS28.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS28_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS29_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS29.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS29_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS30_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS30.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS30_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS31_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS31.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS31_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS8_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS8.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS8_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_L_GI_MCS9_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS9.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS9_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS10_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS10.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS10_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS11_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS11.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS11_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS12_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS12.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS12_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS13_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS13.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS13_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS14_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS14.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS14_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS15_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS15.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS15_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS16_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS16.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS16_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS17_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS17.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS17_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS18_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS18.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS18_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS19_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS19.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS19_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS20_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS20.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS20_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS21_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS21.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS21_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS22_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS22.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS22_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS23_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS23.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS23_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS24_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS24.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS24_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS25_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS25.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS25_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS26_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS26.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS26_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS27_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS27.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS27_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS28_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS28.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS28_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS29_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS29.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS29_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS30_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS30.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS30_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS31_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS31.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS31_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS8_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS8.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS8_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_GREENFIELD_S_GI_MCS9_REFERENCE_FILE_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS9.ref"),g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS9_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS24_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MCS24.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS24_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS25_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MCS25.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS25_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS26_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MCS26.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS26_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS27_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MCS27.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS27_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS28_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MCS28.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS28_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS29_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MCS29.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS29_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS30_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MCS30.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS30_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_MCS31_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MCS31.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS31_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS10_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS10.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS10_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS11_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS11.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS11_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS12_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS12.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS12_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS13_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS13.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS13_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS14_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS14.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS14_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS15_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS15.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS15_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS16_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS16.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS16_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS17_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS17.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS17_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS18_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS18.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS18_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS19_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS19.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS19_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS20_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS20.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS20_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS21_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS21.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS21_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS22_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS22.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS22_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS23_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS23.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS23_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS24_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS24.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS24_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS25_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS25.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS25_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS26_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS26.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS26_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS27_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS27.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS27_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS28_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS28.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS28_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS29_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS29.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS29_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS30_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS30.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS30_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS31_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS31.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS31_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS8_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS8.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS8_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT20_MIXED_S_GI_MCS9_REFERENCE_FILE_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS9.ref"),g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS9_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS10_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS10.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS10_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS11_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS11.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS11_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS12_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS12.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS12_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS13_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS13.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS13_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS14_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS14.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS14_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS15_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS15.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS15_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS16_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS16.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS16_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS17_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS17.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS17_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS18_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS18.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS18_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS19_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS19.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS19_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS20_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS20.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS20_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS21_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS21.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS21_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS22_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS22.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS22_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS23_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS23.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS23_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS24_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS24.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS24_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS25_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS25.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS25_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS26_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS26.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS26_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS27_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS27.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS27_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS28_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS28.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS28_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS29_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS29.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS29_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS30_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS30.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS30_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS31_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS31.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS31_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS8_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS8.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS8_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_L_GI_MCS9_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS9.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS9_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS10_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS10.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS10_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS11_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS11.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS11_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS12_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS12.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS12_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS13_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS13.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS13_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS14_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS14.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS14_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS15_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS15.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS15_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS16_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS16.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS16_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS17_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS17.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS17_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS18_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS18.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS18_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS19_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS19.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS19_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS20_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS20.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS20_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS21_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS21.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS21_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS22_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS22.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS22_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS23_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS23.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS23_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS24_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS24.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS24_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS25_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS25.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS25_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS26_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS26.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS26_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS27_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS27.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS27_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS28_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS28.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS28_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS29_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS29.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS29_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS30_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS30.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS30_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS31_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS31.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS31_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS8_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS8.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS8_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_GREENFIELD_S_GI_MCS9_REFERENCE_FILE_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS9.ref"),g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS9_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS24_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MCS24.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS24_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS25_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MCS25.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS25_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS26_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MCS26.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS26_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS27_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MCS27.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS27_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS28_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MCS28.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS28_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS29_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MCS29.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS29_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS30_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MCS30.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS30_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_MCS31_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MCS31.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS31_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS10_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS10.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS10_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS11_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS11.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS11_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS12_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS12.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS12_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS13_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS13.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS13_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS14_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS14.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS14_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS15_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS15.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS15_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS16_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS16.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS16_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS17_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS17.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS17_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS18_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS18.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS18_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS19_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS19.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS19_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS20_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS20.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS20_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS21_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS21.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS21_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS22_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS22.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS22_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS23_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS23.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS23_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS24_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS24.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS24_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS25_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS25.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS25_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS26_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS26.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS26_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS27_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS27.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS27_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS28_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS28.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS28_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS29_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS29.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS29_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS30_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS30.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS30_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS31_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS31.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS31_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS8_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS8.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS8_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("EVM_11N_HT40_MIXED_S_GI_MCS9_REFERENCE_FILE_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS9.ref"),g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS9_REFERENCE_FILE_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS0.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS10.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS11.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS12.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS13.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS14.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS15.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS16.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS17.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS18.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS19.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS1.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS20.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS21.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS22.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS23.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS24.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS25.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS26.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS27.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS28.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS29.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS2.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS30.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS31.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS3.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS4.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS5.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS6.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS7.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS8.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_L_GI_MCS9.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS0.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS10.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS11.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS12.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS13.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS14.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS15.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS16.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS17.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS18.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS19.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS1.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS20.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS21.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS22.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS23.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS24.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS25.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS26.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS27.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS28.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS29.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS2.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS30.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS31.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS3.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS4.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS5.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS6.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS7.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS8.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME"), _T("WiFi_HT20_GREENFIELD_S_GI_MCS9.mod"),g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS0_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS0.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS0_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS10_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS10.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS10_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS11_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS11.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS11_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS12_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS12.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS12_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS13_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS13.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS13_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS14_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS14.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS14_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS15_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS15.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS15_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS16_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS16.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS16_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS17_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS17.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS17_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS18_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS18.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS18_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS19_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS19.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS19_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS1_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS1.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS1_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS20_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS20.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS20_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS21_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS21.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS21_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS22_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS22.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS22_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS23_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS23.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS23_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS24_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS24.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS24_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS25_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS25.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS25_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS26_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS26.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS26_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS27_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS27.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS27_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS28_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS28.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS28_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS29_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS29.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS29_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS2_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS2.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS2_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS30_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS30.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS30_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS31_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS31.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS31_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS3_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS3.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS3_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS4_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS4.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS4_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS5_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS5.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS5_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS6_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS6.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS6_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS7_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS7.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS7_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS8_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS8.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS8_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT20_MIXED_S_GI_MCS9_WAVEFORM_NAME"), _T("WiFi_HT20_MIXED_S_GI_MCS9.mod"),g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS9_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS0.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS10.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS11.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS12.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS13.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS14.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS15.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS16.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS17.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS18.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS19.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS1.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS20.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS21.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS22.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS23.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS24.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS25.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS26.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS27.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS28.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS29.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS2.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS30.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS31.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS3.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS4.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS5.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS6.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS7.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS8.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_L_GI_MCS9.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS0.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS10.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS11.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS12.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS13.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS14.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS15.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS16.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS17.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS18.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS19.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS1.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS20.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS21.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS22.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS23.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS24.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS25.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS26.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS27.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS28.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS29.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS2I.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS30.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS31.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS3.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS4.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS5.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS6.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS7.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS8.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME"), _T("WiFi_HT40_GREENFIELD_S_GI_MCS9.mod"),g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS0_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS0.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS0_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS10_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS10.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS10_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS11_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS11.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS11_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS12_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS12.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS12_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS13_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS13.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS13_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS14_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS14.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS14_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS15_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS15.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS15_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS16_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS16.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS16_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS17_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS17.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS17_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS18_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS18.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS18_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS19_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS19.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS19_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS1_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS1.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS1_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS20_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS20.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS20_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS21_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS21.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS21_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS22_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS22.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS22_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS23_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS23.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS23_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS24_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS24.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS24_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS25_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS25.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS25_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS26_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS26.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS26_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS27_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS27.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS27_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS28_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS28.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS28_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS29_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS29.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS29_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS2_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS2.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS2_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS30_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS30.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS30_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS31_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS31.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS31_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS3_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS3.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS3_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS4_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS4.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS4_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS5_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS5.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS5_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS6_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS6.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS6_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS7_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS7.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS7_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS8_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS8.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS8_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11N_HT40_MIXED_S_GI_MCS9_WAVEFORM_NAME"), _T("WiFi_HT40_MIXED_S_GI_MCS9.mod"),g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS9_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11P_HALF-12_WAVEFORM_NAME"), _T("WiFi_HALF-12.mod"),g_tsGlobalSetting.PER_11P_HALF12_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11P_HALF-18_WAVEFORM_NAME"), _T("WiFi_HALF-18.mod"),g_tsGlobalSetting.PER_11P_HALF18_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11P_HALF-24_WAVEFORM_NAME"), _T("WiFi_HALF-24.mod"),g_tsGlobalSetting.PER_11P_HALF24_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11P_HALF-27_WAVEFORM_NAME"), _T("WiFi_HALF-27.mod"),g_tsGlobalSetting.PER_11P_HALF27_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11P_HALF-3_WAVEFORM_NAME"), _T("WiFi_HALF-3.mod"),g_tsGlobalSetting.PER_11P_HALF3_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11P_HALF-4_5_WAVEFORM_NAME"), _T("WiFi_HALF-4_5.mod"),g_tsGlobalSetting.PER_11P_HALF4_5_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11P_HALF-6_WAVEFORM_NAME"), _T("WiFi_HALF-6.mod"),g_tsGlobalSetting.PER_11P_HALF6_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11P_HALF-9_WAVEFORM_NAME"), _T("WiFi_HALF-9.mod"),g_tsGlobalSetting.PER_11P_HALF9_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11P_QUAR-12_WAVEFORM_NAME"), _T("WiFi_QUAR-12.mod"),g_tsGlobalSetting.PER_11P_QUAR12_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11P_QUAR-13_5_WAVEFORM_NAME"), _T("WiFi_QUAR-13_5.mod"),g_tsGlobalSetting.PER_11P_QUAR13_5_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11P_QUAR-1_5_WAVEFORM_NAME"), _T("WiFi_QUAR-1_5.mod"),g_tsGlobalSetting.PER_11P_QUAR1_5_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11P_QUAR-2_25_WAVEFORM_NAME"), _T("WiFi_QUAR-2_25.mod"),g_tsGlobalSetting.PER_11P_QUAR2_25_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11P_QUAR-3_WAVEFORM_NAME"), _T("WiFi_QUAR-3.mod"),g_tsGlobalSetting.PER_11P_QUAR3_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11P_QUAR-4_5_WAVEFORM_NAME"), _T("WiFi_QUAR-4_5.mod"),g_tsGlobalSetting.PER_11P_QUAR4_5_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11P_QUAR-6_WAVEFORM_NAME"), _T("WiFi_QUAR-6.mod"),g_tsGlobalSetting.PER_11P_QUAR6_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("GLOBAL_SETTINGS"),_T("PER_11P_QUAR-9_WAVEFORM_NAME"), _T("WiFi_QUAR-9.mod"),g_tsGlobalSetting.PER_11P_QUAR9_WAVEFORM_NAME ,BUFFER_SIZE,szFilenameGlobalSetting);
	// new add end

	// test control settings
	g_tsGlobalSetting.TestCtrl.RunMode=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("RunMode"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.RepeatTimes=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("RepeatTimes"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.ExitWhenDone=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("ExitWhenDone"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.RetryMode=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("RetryMode"),1,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.ShowFailInfo=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("ShowFailInfo"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.nRetryTimes=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("RetryTimes"),3,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.ReloadDutDLL=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("ReloadDutDLL"),0,szFilenameGlobalSetting);
	g_tsGlobalSetting.TestCtrl.IQ_TESTER_RECONNECT=GetPrivateProfileInt(_T("TEST_CONTROL"),_T("IQ_RECONNECT"),0,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("IQ_TESTER_IP1"),_T("192.168.100.254"),g_tsGlobalSetting.TestCtrl.IQ_TESTER_IP1,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("IQ_TESTER_IP2"),_T(""),g_tsGlobalSetting.TestCtrl.IQ_TESTER_IP2,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("IQ_TESTER_IP3"),_T(""),g_tsGlobalSetting.TestCtrl.IQ_TESTER_IP3,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("IQ_TESTER_IP4"),_T(""),g_tsGlobalSetting.TestCtrl.IQ_TESTER_IP4,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("DUT_DLL_FILENAME"),_T("LP_DUT.DLL"),g_tsGlobalSetting.TestCtrl.szDUT_DLL_FILENAME,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("ConnectionString"),_T(""),g_tsGlobalSetting.TestCtrl.szConnectionString,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("TX_PATHLOSS_FILE"),_T("path_loss.csv"),g_tsGlobalSetting.TestCtrl.szTxPathLossFile,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("RX_PATHLOSS_FILE"),_T("path_loss.csv"),g_tsGlobalSetting.TestCtrl.szRxPathLossFile,BUFFER_SIZE,szFilenameGlobalSetting);
	GetPrivateProfileString(_T("TEST_CONTROL"),_T("PACKET_FORMAT"),_T("MIXED"),g_tsGlobalSetting.TestCtrl.PACKET_FORMAT,BUFFER_SIZE,szFilenameGlobalSetting);


	return 0;
}

int run_Global_Settings_AddParam(TM_ID TM_Test)
{
	// Read parameters
	read_Global_Settings("global_settings.ini");

	TM_AddIntegerParameter(TM_Test, "ANALYSIS_11AG_AMPL_TRACK",g_tsGlobalSetting.ANALYSIS_11AG_AMPL_TRACK  );
	TM_AddIntegerParameter(TM_Test, "ANALYSIS_11AG_CH_ESTIMATE",g_tsGlobalSetting.ANALYSIS_11AG_CH_ESTIMATE  );
	TM_AddIntegerParameter(TM_Test, "ANALYSIS_11AG_FREQ_SYNC",g_tsGlobalSetting.ANALYSIS_11AG_FREQ_SYNC  );
	TM_AddIntegerParameter(TM_Test, "ANALYSIS_11AG_PH_CORR_MODE",g_tsGlobalSetting.ANALYSIS_11AG_PH_CORR_MODE  );
	TM_AddIntegerParameter(TM_Test, "ANALYSIS_11AG_SYM_TIM_CORR",g_tsGlobalSetting.ANALYSIS_11AG_SYM_TIM_CORR  );
	TM_AddIntegerParameter(TM_Test, "ANALYSIS_11B_DC_REMOVE_FLAG",g_tsGlobalSetting.ANALYSIS_11B_DC_REMOVE_FLAG  );
	TM_AddIntegerParameter(TM_Test, "ANALYSIS_11B_EQ_TAPS",g_tsGlobalSetting.ANALYSIS_11B_EQ_TAPS  );
	TM_AddIntegerParameter(TM_Test, "ANALYSIS_11B_FIXED_01_DATA_SEQUENCE",g_tsGlobalSetting.ANALYSIS_11B_FIXED_01_DATA_SEQUENCE  );
	TM_AddIntegerParameter(TM_Test, "ANALYSIS_11B_METHOD_11B",g_tsGlobalSetting.ANALYSIS_11B_METHOD_11B  );
	TM_AddIntegerParameter(TM_Test, "ANALYSIS_11N_AMPLITUDE_TRACKING",g_tsGlobalSetting.ANALYSIS_11N_AMPLITUDE_TRACKING  );
	TM_AddIntegerParameter(TM_Test, "ANALYSIS_11N_DECODE_PSDU",g_tsGlobalSetting.ANALYSIS_11N_DECODE_PSDU  );
	TM_AddIntegerParameter(TM_Test, "ANALYSIS_11N_FREQUENCY_CORRELATION",g_tsGlobalSetting.ANALYSIS_11N_FREQUENCY_CORRELATION  );
	TM_AddIntegerParameter(TM_Test, "ANALYSIS_11N_FULL_PACKET_CHANNEL_EST",g_tsGlobalSetting.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST  );
	TM_AddIntegerParameter(TM_Test, "ANALYSIS_11N_PHASE_CORR",g_tsGlobalSetting.ANALYSIS_11N_PHASE_CORR  );
	TM_AddIntegerParameter(TM_Test, "ANALYSIS_11N_SYM_TIMING_CORR",g_tsGlobalSetting.ANALYSIS_11N_SYM_TIMING_CORR  );
	TM_AddIntegerParameter(TM_Test, "DUT_KEEP_TRANSMIT",g_tsGlobalSetting.DUT_KEEP_TRANSMIT  );
	TM_AddIntegerParameter(TM_Test, "DUT_RX_SETTLE_TIME_MS",g_tsGlobalSetting.DUT_RX_SETTLE_TIME_MS  );
	TM_AddIntegerParameter(TM_Test, "DUT_TX_SETTLE_TIME_MS",g_tsGlobalSetting.DUT_TX_SETTLE_TIME_MS  );
//	TM_AddIntegerParameter(TM_Test, "EVM_11AG_SAMPLE_INTERVAL_US",g_tsGlobalSetting.EVM_11AG_SAMPLE_INTERVAL_US  );
//	TM_AddIntegerParameter(TM_Test, "EVM_11B_L_SAMPLE_INTERVAL_US",g_tsGlobalSetting.EVM_11B_L_SAMPLE_INTERVAL_US  );
//	TM_AddIntegerParameter(TM_Test, "EVM_11B_S_SAMPLE_INTERVAL_US",g_tsGlobalSetting.EVM_11B_S_SAMPLE_INTERVAL_US  );
//	TM_AddIntegerParameter(TM_Test, "EVM_11N_GREENFIELD_SAMPLE_INTERVAL_US",g_tsGlobalSetting.EVM_11N_GREENFIELD_SAMPLE_INTERVAL_US  );
//	TM_AddIntegerParameter(TM_Test, "EVM_11N_MIXED_SAMPLE_INTERVAL_US",g_tsGlobalSetting.EVM_11N_MIXED_SAMPLE_INTERVAL_US  );
	TM_AddIntegerParameter(TM_Test, "EVM_AVERAGE",g_tsGlobalSetting.EVM_AVERAGE  );
	TM_AddIntegerParameter(TM_Test, "EVM_SYMBOL_NUM",g_tsGlobalSetting.EVM_SYMBOL_NUM  );
	TM_AddIntegerParameter(TM_Test, "IQ2010_EXT_ENABLE",g_tsGlobalSetting.IQ2010_EXT_ENABLE  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11AG_12M",g_tsGlobalSetting.IQ_P_TO_A_11AG_12M  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11AG_18M",g_tsGlobalSetting.IQ_P_TO_A_11AG_18M  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11AG_24M",g_tsGlobalSetting.IQ_P_TO_A_11AG_24M  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11AG_36M",g_tsGlobalSetting.IQ_P_TO_A_11AG_36M  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11AG_48M",g_tsGlobalSetting.IQ_P_TO_A_11AG_48M  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11AG_54M",g_tsGlobalSetting.IQ_P_TO_A_11AG_54M  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11AG_6M",g_tsGlobalSetting.IQ_P_TO_A_11AG_6M  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11AG_9M",g_tsGlobalSetting.IQ_P_TO_A_11AG_9M  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11B_11M",g_tsGlobalSetting.IQ_P_TO_A_11B_11M  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11B_1M",g_tsGlobalSetting.IQ_P_TO_A_11B_1M  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11B_2M",g_tsGlobalSetting.IQ_P_TO_A_11B_2M  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11B_5_5M",g_tsGlobalSetting.IQ_P_TO_A_11B_5_5M  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11B_CH14",g_tsGlobalSetting.IQ_P_TO_A_11B_CH14  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11N_MCS0",g_tsGlobalSetting.IQ_P_TO_A_11N_MCS0  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11N_MCS1",g_tsGlobalSetting.IQ_P_TO_A_11N_MCS1  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11N_MCS2",g_tsGlobalSetting.IQ_P_TO_A_11N_MCS2  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11N_MCS3",g_tsGlobalSetting.IQ_P_TO_A_11N_MCS3  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11N_MCS4",g_tsGlobalSetting.IQ_P_TO_A_11N_MCS4  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11N_MCS5",g_tsGlobalSetting.IQ_P_TO_A_11N_MCS5  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11N_MCS6",g_tsGlobalSetting.IQ_P_TO_A_11N_MCS6  );
	TM_AddIntegerParameter(TM_Test, "IQ_P_TO_A_11N_MCS7",g_tsGlobalSetting.IQ_P_TO_A_11N_MCS7  );
	TM_AddIntegerParameter(TM_Test, "MASK_DSSS_SAMPLE_INTERVAL_US",g_tsGlobalSetting.MASK_DSSS_SAMPLE_INTERVAL_US  );
	TM_AddIntegerParameter(TM_Test, "MASK_OFDM_SAMPLE_INTERVAL_US",g_tsGlobalSetting.MASK_OFDM_SAMPLE_INTERVAL_US  );
	TM_AddIntegerParameter(TM_Test, "PER_11AG_12_PACKETS_NUM",g_tsGlobalSetting.PER_11AG_12_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11AG_18_PACKETS_NUM",g_tsGlobalSetting.PER_11AG_18_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11AG_24_PACKETS_NUM",g_tsGlobalSetting.PER_11AG_24_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11AG_36_PACKETS_NUM",g_tsGlobalSetting.PER_11AG_36_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11AG_48_PACKETS_NUM",g_tsGlobalSetting.PER_11AG_48_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11AG_54_PACKETS_NUM",g_tsGlobalSetting.PER_11AG_54_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11AG_6_PACKETS_NUM",g_tsGlobalSetting.PER_11AG_6_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11AG_9_PACKETS_NUM",g_tsGlobalSetting.PER_11AG_9_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11B_11_PACKETS_NUM",g_tsGlobalSetting.PER_11B_11_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11B_1_PACKETS_NUM",g_tsGlobalSetting.PER_11B_1_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11B_2_PACKETS_NUM",g_tsGlobalSetting.PER_11B_2_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11B_5_5_PACKETS_NUM",g_tsGlobalSetting.PER_11B_5_5_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS0_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS0_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS10_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS10_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS11_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS11_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS12_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS12_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS13_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS13_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS14_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS14_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS15_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS15_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS16_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS16_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS17_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS17_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS18_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS18_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS19_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS19_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS1_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS1_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS20_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS20_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS21_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS21_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS22_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS22_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS23_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS23_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS24_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS24_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS25_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS25_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS26_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS26_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS27_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS27_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS28_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS28_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS29_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS29_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS2_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS2_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS30_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS30_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS31_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS31_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS3_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS3_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS4_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS4_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS5_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS5_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS6_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS6_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS7_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS7_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS8_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS8_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_MCS9_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS9_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS0_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS0_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS10_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS10_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS11_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS11_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS12_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS12_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS13_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS13_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS14_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS14_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS15_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS15_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS16_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS16_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS17_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS17_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS18_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS18_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS19_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS19_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS1_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS1_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS20_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS20_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS21_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS21_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS22_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS22_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS23_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS23_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS24_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS24_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS25_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS25_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS26_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS26_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS27_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS27_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS28_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS28_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS29_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS29_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS2_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS2_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS30_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS30_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS31_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS31_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS3_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS3_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS4_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS4_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS5_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS5_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS6_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS6_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS7_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS7_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS8_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS8_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_MCS9_PACKETS_NUM",g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS9_PACKETS_NUM  );
	TM_AddIntegerParameter(TM_Test, "PER_VSG_TIMEOUT_SEC",g_tsGlobalSetting.PER_VSG_TIMEOUT_SEC  );
	TM_AddIntegerParameter(TM_Test, "PM_AVERAGE",g_tsGlobalSetting.PM_AVERAGE  );
	TM_AddIntegerParameter(TM_Test, "PM_DSSS_SAMPLE_INTERVAL_US",g_tsGlobalSetting.PM_DSSS_SAMPLE_INTERVAL_US  );
	TM_AddIntegerParameter(TM_Test, "PM_IF_FREQ_SHIFT_MHZ",g_tsGlobalSetting.PM_IF_FREQ_SHIFT_MHZ  );
	TM_AddIntegerParameter(TM_Test, "PM_OFDM_SAMPLE_INTERVAL_US",g_tsGlobalSetting.PM_OFDM_SAMPLE_INTERVAL_US  );
	TM_AddIntegerParameter(TM_Test, "SPECTRUM_DSSS_SAMPLE_INTERVAL_US",g_tsGlobalSetting.SPECTRUM_DSSS_SAMPLE_INTERVAL_US  );
	TM_AddIntegerParameter(TM_Test, "SPECTRUM_OFDM_SAMPLE_INTERVAL_US",g_tsGlobalSetting.SPECTRUM_OFDM_SAMPLE_INTERVAL_US  );
	TM_AddIntegerParameter(TM_Test, "VSA_PORT",g_tsGlobalSetting.VSA_PORT  );
	TM_AddIntegerParameter(TM_Test, "VSA_SAVE_CAPTURE_ALWAYS",g_tsGlobalSetting.VSA_SAVE_CAPTURE_ALWAYS  );
	TM_AddIntegerParameter(TM_Test, "VSA_SAVE_CAPTURE_ON_FAILED",g_tsGlobalSetting.VSA_SAVE_CAPTURE_ON_FAILED  );
	TM_AddIntegerParameter(TM_Test, "VSA_SKIP_PACKET_COUNT",g_tsGlobalSetting.VSA_SKIP_PACKET_COUNT  );
	TM_AddIntegerParameter(TM_Test, "VSA_TRIGGER_TIMEOUT_SEC",g_tsGlobalSetting.VSA_TRIGGER_TIMEOUT_SEC  );
	TM_AddIntegerParameter(TM_Test, "VSA_TRIGGER_TYPE",g_tsGlobalSetting.VSA_TRIGGER_TYPE  );
	TM_AddIntegerParameter(TM_Test, "VSG_PORT",g_tsGlobalSetting.VSG_PORT  );

	// new add int parameters, IQLite_Core_3.0.1
	TM_AddIntegerParameter(TM_Test, "AUTO_GAIN_CONTROL_ENABLE", g_tsGlobalSetting.AUTO_GAIN_CONTROL_ENABLE );
	TM_AddIntegerParameter(TM_Test, "AUTO_READING_LIMIT", g_tsGlobalSetting.AUTO_READING_LIMIT );
	TM_AddIntegerParameter(TM_Test, "EVM_CAPTURE_TIME_11AG_US", g_tsGlobalSetting.EVM_CAPTURE_TIME_11AG_US );
	TM_AddIntegerParameter(TM_Test, "EVM_CAPTURE_TIME_11B_L_US", g_tsGlobalSetting.EVM_CAPTURE_TIME_11B_L_US );
	TM_AddIntegerParameter(TM_Test, "EVM_CAPTURE_TIME_11B_S_US", g_tsGlobalSetting.EVM_CAPTURE_TIME_11B_S_US );
	TM_AddIntegerParameter(TM_Test, "EVM_CAPTURE_TIME_11N_GREENFIELD_US", g_tsGlobalSetting.EVM_CAPTURE_TIME_11N_GREENFIELD_US );
	TM_AddIntegerParameter(TM_Test, "EVM_CAPTURE_TIME_11N_MIXED_US", g_tsGlobalSetting.EVM_CAPTURE_TIME_11N_MIXED_US );
	TM_AddIntegerParameter(TM_Test, "EVM_CAPTURE_TIME_11P_HALF_US", g_tsGlobalSetting.EVM_CAPTURE_TIME_11P_HALF_US );
	TM_AddIntegerParameter(TM_Test, "EVM_CAPTURE_TIME_11P_QUAR_US", g_tsGlobalSetting.EVM_CAPTURE_TIME_11P_QUAR_US );
	TM_AddIntegerParameter(TM_Test, "MASK_FFT_AVERAGE", g_tsGlobalSetting.MASK_FFT_AVERAGE );
	TM_AddIntegerParameter(TM_Test, "MASK_HALF_SAMPLE_INTERVAL_US", g_tsGlobalSetting.MASK_HALF_SAMPLE_INTERVAL_US );
	TM_AddIntegerParameter(TM_Test, "MASK_QUAR_SAMPLE_INTERVAL_US", g_tsGlobalSetting.MASK_QUAR_SAMPLE_INTERVAL_US );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS0_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS0_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS10_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS10_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS11_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS11_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS12_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS12_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS13_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS13_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS14_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS14_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS15_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS15_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS16_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS16_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS17_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS17_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS18_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS18_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS19_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS19_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS1_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS1_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS20_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS20_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS21_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS21_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS22_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS22_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS23_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS23_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS24_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS24_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS25_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS25_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS26_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS26_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS27_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS27_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS28_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS28_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS29_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS29_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS2_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS2_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS30_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS30_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS31_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS31_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS3_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS3_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS4_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS4_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS5_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS5_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS6_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS6_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS7_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS7_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS8_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS8_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_L_GI_MCS9_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS9_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS0_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS0_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS10_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS10_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS11_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS11_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS12_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS12_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS13_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS13_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS14_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS14_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS15_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS15_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS16_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS16_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS17_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS17_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS18_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS18_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS19_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS19_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS1_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS1_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS20_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS20_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS21_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS21_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS22_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS22_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS23_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS23_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS24_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS24_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS25_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS25_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS26_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS26_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS27_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS27_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS28_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS28_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS29_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS29_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS2_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS2_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS30_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS30_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS31_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS31_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS3_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS3_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS4_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS4_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS5_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS5_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS6_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS6_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS7_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS7_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS8_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS8_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_GREENFIELD_S_GI_MCS9_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS9_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS0_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS0_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS10_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS10_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS11_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS11_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS12_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS12_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS13_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS13_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS14_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS14_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS15_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS15_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS16_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS16_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS17_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS17_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS18_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS18_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS19_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS19_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS1_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS1_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS20_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS20_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS21_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS21_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS22_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS22_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS23_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS23_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS24_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS24_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS25_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS25_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS26_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS26_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS27_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS27_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS28_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS28_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS29_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS29_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS2_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS2_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS30_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS30_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS31_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS31_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS3_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS3_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS4_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS4_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS5_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS5_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS6_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS6_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS7_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS7_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS8_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS8_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT20_MIXED_S_GI_MCS9_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS9_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS0_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS0_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS10_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS10_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS11_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS11_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS12_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS12_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS13_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS13_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS14_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS14_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS15_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS15_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS16_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS16_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS17_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS17_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS18_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS18_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS19_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS19_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS1_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS1_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS20_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS20_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS21_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS21_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS22_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS22_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS23_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS23_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS24_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS24_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS25_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS25_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS26_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS26_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS27_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS27_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS28_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS28_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS29_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS29_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS2_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS2_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS30_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS30_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS31_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS31_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS3_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS3_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS4_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS4_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS5_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS5_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS6_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS6_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS7_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS7_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS8_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS8_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_L_GI_MCS9_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS9_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS0_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS0_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS10_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS10_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS11_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS11_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS12_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS12_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS13_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS13_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS14_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS14_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS15_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS15_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS16_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS16_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS17_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS17_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS18_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS18_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS19_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS19_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS1_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS1_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS20_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS20_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS21_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS21_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS22_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS22_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS23_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS23_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS24_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS24_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS25_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS25_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS26_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS26_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS27_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS27_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS28_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS28_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS29_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS29_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS2_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS2_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS30_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS30_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS31_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS31_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS3_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS3_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS4_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS4_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS5_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS5_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS6_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS6_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS7_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS7_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS8_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS8_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_GREENFIELD_S_GI_MCS9_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS9_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS0_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS0_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS10_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS10_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS11_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS11_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS12_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS12_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS13_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS13_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS14_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS14_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS15_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS15_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS16_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS16_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS17_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS17_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS18_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS18_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS19_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS19_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS1_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS1_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS20_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS20_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS21_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS21_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS22_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS22_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS23_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS23_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS24_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS24_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS25_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS25_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS26_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS26_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS27_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS27_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS28_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS28_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS29_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS29_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS2_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS2_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS30_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS30_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS31_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS31_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS3_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS3_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS4_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS4_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS5_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS5_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS6_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS6_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS7_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS7_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS8_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS8_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11N_HT40_MIXED_S_GI_MCS9_PACKETS_NUM", g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS9_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11P_HALF12_PACKETS_NUM", g_tsGlobalSetting.PER_11P_HALF12_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11P_HALF18_PACKETS_NUM", g_tsGlobalSetting.PER_11P_HALF18_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11P_HALF24_PACKETS_NUM", g_tsGlobalSetting.PER_11P_HALF24_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11P_HALF27_PACKETS_NUM", g_tsGlobalSetting.PER_11P_HALF27_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11P_HALF3_PACKETS_NUM", g_tsGlobalSetting.PER_11P_HALF3_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11P_HALF4_5_PACKETS_NUM", g_tsGlobalSetting.PER_11P_HALF4_5_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11P_HALF6_PACKETS_NUM", g_tsGlobalSetting.PER_11P_HALF6_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11P_HALF9_PACKETS_NUM", g_tsGlobalSetting.PER_11P_HALF9_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11P_QUAR12_PACKETS_NUM", g_tsGlobalSetting.PER_11P_QUAR12_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11P_QUAR13_5_PACKETS_NUM", g_tsGlobalSetting.PER_11P_QUAR13_5_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11P_QUAR1_5_PACKETS_NUM", g_tsGlobalSetting.PER_11P_QUAR1_5_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11P_QUAR2_25_PACKETS_NUM", g_tsGlobalSetting.PER_11P_QUAR2_25_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11P_QUAR3_PACKETS_NUM", g_tsGlobalSetting.PER_11P_QUAR3_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11P_QUAR4_5_PACKETS_NUM", g_tsGlobalSetting.PER_11P_QUAR4_5_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11P_QUAR6_PACKETS_NUM", g_tsGlobalSetting.PER_11P_QUAR6_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PER_11P_QUAR9_PACKETS_NUM", g_tsGlobalSetting.PER_11P_QUAR9_PACKETS_NUM );
	TM_AddIntegerParameter(TM_Test, "PM_HALF_SAMPLE_INTERVAL_US", g_tsGlobalSetting.PM_HALF_SAMPLE_INTERVAL_US );
	TM_AddIntegerParameter(TM_Test, "PM_QUAR_SAMPLE_INTERVAL_US", g_tsGlobalSetting.PM_QUAR_SAMPLE_INTERVAL_US );
	TM_AddIntegerParameter(TM_Test, "RELATIVE_LIMIT", g_tsGlobalSetting.RELATIVE_LIMIT );
	TM_AddIntegerParameter(TM_Test, "RESET_TEST_ITEM_DURING_RETRY", g_tsGlobalSetting.RESET_TEST_ITEM_DURING_RETRY );
	TM_AddIntegerParameter(TM_Test, "RETRY_ERROR_ITEMS", g_tsGlobalSetting.RETRY_ERROR_ITEMS );
	TM_AddIntegerParameter(TM_Test, "RETRY_TEST_ITEM", g_tsGlobalSetting.RETRY_TEST_ITEM );
	TM_AddIntegerParameter(TM_Test, "SPECTRUM_HALF_SAMPLE_INTERVAL_US", g_tsGlobalSetting.SPECTRUM_HALF_SAMPLE_INTERVAL_US );
	TM_AddIntegerParameter(TM_Test, "SPECTRUM_QUAR_SAMPLE_INTERVAL_US", g_tsGlobalSetting.SPECTRUM_QUAR_SAMPLE_INTERVAL_US );

	// new add end

	// add double param
	TM_AddDoubleParameter(TM_Test, "VSA_ACK_POWER_RMS_DBM",g_tsGlobalSetting.VSA_ACK_POWER_RMS_DBM  );
	TM_AddDoubleParameter(TM_Test, "VSA_ACK_TRIGGER_LEVEL_DBM",g_tsGlobalSetting.VSA_ACK_TRIGGER_LEVEL_DBM  );
	TM_AddDoubleParameter(TM_Test, "VSA_AMPLITUDE_TOLERANCE_DB",g_tsGlobalSetting.VSA_AMPLITUDE_TOLERANCE_DB  );
	TM_AddDoubleParameter(TM_Test, "VSA_PRE_TRIGGER_TIME_US",g_tsGlobalSetting.VSA_PRE_TRIGGER_TIME_US  );
	TM_AddDoubleParameter(TM_Test, "VSA_TRIGGER_LEVEL_DB",g_tsGlobalSetting.VSA_TRIGGER_LEVEL_DB  );
	TM_AddDoubleParameter(TM_Test, "VSG_MAX_POWER_11B",g_tsGlobalSetting.VSG_MAX_POWER_11B  );
	TM_AddDoubleParameter(TM_Test, "VSG_MAX_POWER_11G",g_tsGlobalSetting.VSG_MAX_POWER_11G  );
	TM_AddDoubleParameter(TM_Test, "VSG_MAX_POWER_11N",g_tsGlobalSetting.VSG_MAX_POWER_11N  );

	// add string param
	char szAnsiStr[BUFFER_SIZE]="";

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS8_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_GREENFIELD_MCS8_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS9_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_GREENFIELD_MCS9_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS10_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_GREENFIELD_MCS10_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS11_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_GREENFIELD_MCS11_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS12_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_GREENFIELD_MCS12_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS13_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_GREENFIELD_MCS13_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS14_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_GREENFIELD_MCS14_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS15_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_GREENFIELD_MCS15_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS16_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_GREENFIELD_MCS16_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS17_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_GREENFIELD_MCS17_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS18_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_GREENFIELD_MCS18_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS19_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_GREENFIELD_MCS19_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS20_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_GREENFIELD_MCS20_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS21_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_GREENFIELD_MCS21_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS22_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_GREENFIELD_MCS22_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_MCS23_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_GREENFIELD_MCS23_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS8_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_GREENFIELD_MCS8_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS9_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_GREENFIELD_MCS9_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS10_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_GREENFIELD_MCS10_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS11_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_GREENFIELD_MCS11_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS12_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_GREENFIELD_MCS12_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS13_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_GREENFIELD_MCS13_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS14_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_GREENFIELD_MCS14_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS15_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_GREENFIELD_MCS15_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS16_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_GREENFIELD_MCS16_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS17_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_GREENFIELD_MCS17_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS18_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_GREENFIELD_MCS18_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS19_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_GREENFIELD_MCS19_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS20_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_GREENFIELD_MCS20_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS21_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_GREENFIELD_MCS21_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS22_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_GREENFIELD_MCS22_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_MCS23_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_GREENFIELD_MCS23_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS10_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_MIXED_MCS10_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS11_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_MIXED_MCS11_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS12_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_MIXED_MCS12_REFERENCE_FILE_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS13_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_MIXED_MCS13_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS14_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_MIXED_MCS14_REFERENCE_FILE_NAME",szAnsiStr  );
	
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS15_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_MIXED_MCS15_REFERENCE_FILE_NAME",szAnsiStr  );
	
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS16_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_MIXED_MCS16_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS17_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_MIXED_MCS17_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS18_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_MIXED_MCS18_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS19_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_MIXED_MCS19_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS20_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_MIXED_MCS20_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS21_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_MIXED_MCS21_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS22_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_MIXED_MCS22_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS23_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_MIXED_MCS23_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS8_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_MIXED_MCS8_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS9_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT20_MIXED_MCS9_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS10_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_MIXED_MCS10_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS11_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_MIXED_MCS11_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS12_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_MIXED_MCS12_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS13_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_MIXED_MCS13_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS14_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_MIXED_MCS14_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS15_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_MIXED_MCS15_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS16_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_MIXED_MCS16_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS17_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_MIXED_MCS17_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS18_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_MIXED_MCS18_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS19_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_MIXED_MCS19_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS20_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_MIXED_MCS20_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS21_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_MIXED_MCS21_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS22_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_MIXED_MCS22_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS23_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_MIXED_MCS23_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS8_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_MIXED_MCS8_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS9_REFERENCE_FILE_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "EVM_11N_HT40_MIXED_MCS9_REFERENCE_FILE_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11AG_12_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11AG_12_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11AG_18_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11AG_18_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11AG_24_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11AG_24_WAVEFORM_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.PER_11AG_36_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11AG_36_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11AG_48_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11AG_48_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11AG_54_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11AG_54_WAVEFORM_NAME",szAnsiStr);

	WChar2Char(g_tsGlobalSetting.PER_11AG_6_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11AG_6_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11AG_9_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11AG_9_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11B_11L_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11B_11L_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11B_11S_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11B_11S_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11B_1_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11B_1_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11B_2L_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11B_2L_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11B_2S_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11B_2S_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11B_5_5L_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11B_5_5L_WAVEFORM_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.PER_11B_5_5S_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11B_5_5S_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS0_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS0_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS10_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS10_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS11_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS11_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS12_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS12_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS13_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS13_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS14_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS14_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS15_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS15_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS16_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS16_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS17_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS17_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS18_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS18_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS19_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS19_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS1_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS1_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS20_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS20_WAVEFORM_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS21_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS21_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS22_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS22_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS23_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS23_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS24_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS24_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS25_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS25_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS26_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS26_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS27_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS27_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS28_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS28_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS29_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS29_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS2_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS2_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS30_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS30_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS31_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS31_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS3_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS3_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS4_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS4_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS5_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS5_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS6_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS6_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS7_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS7_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS8_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS8_WAVEFORM_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_MCS9_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT20_MIXED_MCS9_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS0_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS0_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS10_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS10_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS11_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS11_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS12_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS12_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS13_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS13_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS14_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS14_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS15_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS15_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS16_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS16_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS17_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS17_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS18_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS18_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS19_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS19_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS1_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS1_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS20_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS20_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS21_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS21_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS22_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS22_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS23_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS23_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS24_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS24_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS25_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS25_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS26_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS26_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS27_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS27_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS28_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS28_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS29_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS29_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS2_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS2_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS30_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS30_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS31_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS31_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS3_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS3_WAVEFORM_NAME",szAnsiStr );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS4_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS4_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS5_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS5_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS6_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS6_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS7_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS7_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS8_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS8_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_MCS9_WAVEFORM_NAME,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_11N_HT40_MIXED_MCS9_WAVEFORM_NAME",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_WAVEFORM_DESTINATION_MAC,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_WAVEFORM_DESTINATION_MAC",szAnsiStr  );

	WChar2Char(g_tsGlobalSetting.PER_WAVEFORM_PATH,BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test, "PER_WAVEFORM_PATH",szAnsiStr  );

	// new add string parameters, IQLite_Core_3.01
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS10_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS10_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS11_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS11_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS12_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS12_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS13_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS13_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS14_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS14_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS15_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS15_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS16_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS16_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS17_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS17_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS18_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS18_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS19_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS19_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS20_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS20_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS21_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS21_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS22_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS22_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS23_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS23_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS24_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS24_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS25_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS25_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS26_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS26_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS27_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS27_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS28_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS28_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS29_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS29_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS30_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS30_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS31_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS31_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS8_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS8_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_L_GI_MCS9_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_L_GI_MCS9_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS10_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS10_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS11_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS11_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS12_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS12_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS13_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS13_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS14_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS14_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS15_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS15_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS16_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS16_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS17_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS17_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS18_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS18_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS19_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS19_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS20_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS20_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS21_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS21_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS22_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS22_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS23_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS23_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS24_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS24_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS25_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS25_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS26_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS26_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS27_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS27_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS28_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS28_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS29_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS29_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS30_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS30_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS31_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS31_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS8_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS8_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_GREENFIELD_S_GI_MCS9_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_GREENFIELD_S_GI_MCS9_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS24_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_MCS24_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS25_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_MCS25_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS26_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_MCS26_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS27_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_MCS27_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS28_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_MCS28_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS29_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_MCS29_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS30_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_MCS30_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_MCS31_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_MCS31_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS10_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS10_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS11_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS11_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS12_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS12_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS13_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS13_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS14_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS14_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS15_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS15_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS16_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS16_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS17_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS17_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS18_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS18_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS19_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS19_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS20_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS20_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS21_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS21_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS22_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS22_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS23_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS23_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS24_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS24_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS25_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS25_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS26_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS26_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS27_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS27_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS28_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS28_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS29_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS29_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS30_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS30_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS31_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS31_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS8_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS8_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT20_MIXED_S_GI_MCS9_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT20_MIXED_S_GI_MCS9_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS10_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS10_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS11_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS11_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS12_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS12_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS13_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS13_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS14_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS14_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS15_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS15_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS16_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS16_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS17_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS17_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS18_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS18_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS19_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS19_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS20_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS20_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS21_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS21_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS22_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS22_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS23_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS23_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS24_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS24_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS25_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS25_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS26_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS26_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS27_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS27_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS28_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS28_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS29_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS29_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS30_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS30_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS31_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS31_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS8_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS8_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_L_GI_MCS9_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_L_GI_MCS9_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS10_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS10_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS11_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS11_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS12_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS12_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS13_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS13_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS14_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS14_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS15_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS15_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS16_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS16_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS17_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS17_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS18_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS18_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS19_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS19_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS20_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS20_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS21_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS21_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS22_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS22_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS23_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS23_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS24_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS24_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS25_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS25_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS26_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS26_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS27_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS27_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS28_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS28_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS29_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS29_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS30_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS30_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS31_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS31_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS8_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS8_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_GREENFIELD_S_GI_MCS9_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_GREENFIELD_S_GI_MCS9_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS24_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_MCS24_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS25_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_MCS25_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS26_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_MCS26_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS27_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_MCS27_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS28_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_MCS28_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS29_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_MCS29_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS30_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_MCS30_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_MCS31_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_MCS31_REFERENCE_FILE_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS10_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS10_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS11_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS11_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS12_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS12_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS13_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS13_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS14_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS14_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS15_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS15_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS16_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS16_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS17_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS17_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS18_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS18_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS19_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS19_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS20_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS20_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS21_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS21_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS22_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS22_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS23_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS23_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS24_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS24_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS25_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS25_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS26_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS26_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS27_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS27_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS28_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS28_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS29_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS29_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS30_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS30_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS31_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS31_REFERENCE_FILE_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS8_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS8_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.EVM_11N_HT40_MIXED_S_GI_MCS9_REFERENCE_FILE_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"EVM_11N_HT40_MIXED_S_GI_MCS9_REFERENCE_FILE_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS0_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS0_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS10_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS10_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS11_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS11_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS12_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS12_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS13_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS13_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS14_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS14_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS15_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS15_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS16_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS16_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS17_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS17_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS18_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS18_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS19_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS19_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS1_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS1_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS20_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS20_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS21_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS21_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS22_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS22_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS23_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS23_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS24_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS24_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS25_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS25_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS26_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS26_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS27_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS27_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS28_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS28_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS29_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS29_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS2_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS2_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS30_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS30_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS31_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS31_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS3_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS3_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS4_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS4_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS5_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS5_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS6_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS6_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS7_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS7_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS8_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS8_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT20_MIXED_S_GI_MCS9_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT20_MIXED_S_GI_MCS9_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS0_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS0_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS10_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS10_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS11_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS11_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS12_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS12_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS13_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS13_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS14_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS14_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS15_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS15_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS16_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS16_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS17_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS17_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS18_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS18_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS19_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS19_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS1_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS1_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS20_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS20_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS21_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS21_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS22_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS22_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS23_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS23_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS24_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS24_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS25_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS25_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS26_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS26_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS27_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS27_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS28_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS28_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS29_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS29_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS2_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS2_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS30_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS30_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS31_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS31_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS3_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS3_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS4_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS4_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS5_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS5_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS6_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS6_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS7_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS7_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS8_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS8_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11N_HT40_MIXED_S_GI_MCS9_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11N_HT40_MIXED_S_GI_MCS9_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11P_HALF12_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11P_HALF12_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11P_HALF18_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11P_HALF18_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11P_HALF24_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11P_HALF24_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11P_HALF27_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11P_HALF27_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11P_HALF3_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11P_HALF3_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11P_HALF4_5_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11P_HALF4_5_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11P_HALF6_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11P_HALF6_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11P_HALF9_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11P_HALF9_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11P_QUAR12_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);  
	TM_AddStringParameter(TM_Test,"PER_11P_QUAR12_WAVEFORM_NAME", szAnsiStr );  
	WChar2Char(g_tsGlobalSetting.PER_11P_QUAR13_5_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11P_QUAR13_5_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11P_QUAR1_5_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11P_QUAR1_5_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11P_QUAR2_25_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11P_QUAR2_25_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11P_QUAR3_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11P_QUAR3_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11P_QUAR4_5_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE); 
	TM_AddStringParameter(TM_Test,"PER_11P_QUAR4_5_WAVEFORM_NAME", szAnsiStr ); 
	WChar2Char(g_tsGlobalSetting.PER_11P_QUAR6_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11P_QUAR6_WAVEFORM_NAME", szAnsiStr );
	WChar2Char(g_tsGlobalSetting.PER_11P_QUAR9_WAVEFORM_NAME , BUFFER_SIZE,szAnsiStr,BUFFER_SIZE);
	TM_AddStringParameter(TM_Test,"PER_11P_QUAR9_WAVEFORM_NAME", szAnsiStr );
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
				&txParam.nFreq,
				txParam.szDataRate,
				txParam.szBandWidth,
				szTemp1,
				szTemp2,
				szTemp3
				);
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
				&txParam.nFreq,
				txParam.szDataRate,
				txParam.szBandWidth,
				szTemp1,
				szTemp3
				);
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
				&txParam.nFreq,
				txParam.szDataRate,
				txParam.szBandWidth,
				szTemp1,
				szTemp3
				);
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
				&rxParam.nFreq,
				rxParam.szDataRate,
				rxParam.szBandWidth,
				szTemp1,
				szTemp2,
				szTemp3
				);
			strncpy_s(szTemp1,sizeof(szTemp1)-1,szTemp1+1,strlen(szTemp1+1));
			strncpy_s(szTemp2,sizeof(szTemp2)-1,szTemp2+1,strlen(szTemp2+1));
			strncpy_s(szTemp3,sizeof(szTemp3)-1,szTemp3+1,strlen(szTemp3+1));

			rxParam.dRxPowerLevel=atof(szTemp1);
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
		if(txParam->nFreq <= 2484) // 2.4g test item
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
		if(txParam->nFreq <= 2484) // 2.4g test item
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
		if(txParam->nFreq <= 2484) // 2.4g test item
		{
			// EVM average
				
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
	if(txParam->nFreq >=2484)
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

	if(strstr(rxParam->szDataRate,"OFDM") || strstr(rxParam->szDataRate,"MCS"))
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