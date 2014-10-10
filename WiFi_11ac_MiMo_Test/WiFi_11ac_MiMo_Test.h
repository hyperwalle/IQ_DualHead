/*! \mainpage WiFi Test Functions
 *
 * \section intro_sec Introduction
 *
 * This project, WiFi_Test, provides all test functions in compliance with 802.11 a/b/g/n standards, by
 * using LitePoint wireless testers, including IQview/IQflex, IQmax, and IQnxn.
 *
 * WiFi_Test functions are managed by Test Manager (TM).  All functions are not meant to be called 
 * directly by any users.  Instead, all functions will be executed by calling TM_Run() with a corresponding
 * keyword.  TM_Run() is declared in TestManager as below:
 *
 * \code
 * TM_API TM_RETURN TM_Run(TM_ID technologyID, const TM_STR functionKeyword);
 * \endcode 
 *
 * In order to use all WiFi_Test functions, users have to call one function at the very beginning:
 *
 * \code
 * TM_ID WiFi_Test;
 * WiFi_Test = WiFi_Test_Register();
 * \endcode
 *
 * \section revision_sec Revision History
 *
 * \subsection revision_0_0_2 0.0.2
 * Date: Mar 16, 2009
 * - Release with introduction of Flow Composer
 *
 * \subsection revision_0_0_1 0.0.1
 * Date: Feb 08, 2009
 * - First draft
 */

/*! \file WiFi_11ac_Test.h
 * \brief WiFi Test Functions 
 */ 


#ifndef _WIFI_11AC_MIMO_TEST_H_
#define _WIFI_11AC_MIMO_TEST_H_

#if !defined (__cplusplus)
    #define WIFI_11AC_MIMO_TEST_API extern
#elif !defined (WIN32)
    #define WIFI_11AC_MIMO_TEST_API extern "C"
#elif defined(WIFI_11AC_MIMO_TEST_EXPORTS)
    #define WIFI_11AC_MIMO_TEST_API extern "C" __declspec(dllexport)
#else
    #define WIFI_11AC_MIMO_TEST_API extern "C" __declspec(dllimport) 
#endif


#include "IQlite_logger.h"
#include "TestManager.h"
#include "WiFi_11ac_MiMo_Test_Internal.h"
#include "vDUT.h" // vDUT_ID

#include "IQmeasure.h"


//! Register WiFi Test functions with TestManager
/*!
 *
 * \return An ID (>=0) that represents the technology
 * \remark This is the only function that needs to be called at the top application level.
 *         This function installs a callback function for each one of its supported WiFi Test Keywords
 *         (defined in TestManager).
 */
WIFI_11AC_MIMO_TEST_API int WIFI_11AC_MIMO_technologyRegister(void);

WIFI_11AC_MIMO_TEST_API int WiFi_Global_Setting(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Insert_Dut(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Initialize_Dut(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Remove_Dut(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Connect_IQTester(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Disconnect_IQTester(void);
WIFI_11AC_MIMO_TEST_API int WiFi_11ac_MiMo_TX_Verify_Evm(void);
WIFI_11AC_MIMO_TEST_API int WiFi_RX_Verify_Per(void);
WIFI_11AC_MIMO_TEST_API int WiFi_TX_Verify_Mask(void);
WIFI_11AC_MIMO_TEST_API int WiFi_TX_Verify_Power(void);
WIFI_11AC_MIMO_TEST_API int WiFi_TX_Verify_Spectrum(void);
WIFI_11AC_MIMO_TEST_API int WiFi_TX_Calibration(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Write_Eeprom(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Read_Eeprom(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Write_BB_Register(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Read_BB_Register(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Write_RF_Register(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Read_RF_Register(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Write_Mac_Register(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Read_Mac_Register(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Write_Soc_Register(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Read_Soc_Register(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Get_Serial_Number(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Write_Mac_Address(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Read_Mac_Address(void);
WIFI_11AC_MIMO_TEST_API int WiFi_LoadPathLossTable(void);

WIFI_11AC_MIMO_TEST_API int WiFi_Run_External_Program(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Power_Mode_Dut(void);
WIFI_11AC_MIMO_TEST_API int WiFi_Run_DUT_Command(void);

WIFI_11AC_MIMO_TEST_API int   GetWiFiTestID(TM_ID* wifiTestID);
WIFI_11AC_MIMO_TEST_API int   SetWiFiTestID(TM_ID wifiTestID);
WIFI_11AC_MIMO_TEST_API int   GetWiFiDutID(vDUT_ID* wifiDutID);
WIFI_11AC_MIMO_TEST_API int   GetWiFiGlobalSetting(WIFI_GLOBAL_SETTING** wifiGlobleSetting);
WIFI_11AC_MIMO_TEST_API int   GetWiFiRecordParam(WIFI_RECORD_PARAM** wifiRecordParam);
WIFI_11AC_MIMO_TEST_API int   SetTesterNumber(int testerNumber);
WIFI_11AC_MIMO_TEST_API int   GetTesterNumber(int *testerNumber);
WIFI_11AC_MIMO_TEST_API int   GetTesterType(int* testerType);
WIFI_11AC_MIMO_TEST_API int   SetTesterType(int testerType);
WIFI_11AC_MIMO_TEST_API int   SetTesterName(int flag);
WIFI_11AC_MIMO_TEST_API int   GetTesterName(int *flag);
WIFI_11AC_MIMO_TEST_API int   SetTesterReconnect(int flag);
WIFI_11AC_MIMO_TEST_API int   GetTesterReconnect(int *flag);
WIFI_11AC_MIMO_TEST_API int   SetDutConfigChanged(bool changed);
WIFI_11AC_MIMO_TEST_API int   GetDutConfigChanged(bool *changed);
WIFI_11AC_MIMO_TEST_API int   GetDutTxActived(bool* vDutTxActived);
WIFI_11AC_MIMO_TEST_API int   SetDutTxActived(bool vDutTxActived);


WIFI_11AC_MIMO_TEST_API int  RespondToQueryInput( std::map<std::string, WIFI_SETTING_STRUCT>& inputMap);
WIFI_11AC_MIMO_TEST_API int  RespondToQueryReturn( std::map<std::string, WIFI_SETTING_STRUCT>& returnMap);
WIFI_11AC_MIMO_TEST_API int  ReturnTestResults( std::map<std::string, WIFI_SETTING_STRUCT>& returnMap);
WIFI_11AC_MIMO_TEST_API int  ClearReturnParameters( std::map<std::string, WIFI_SETTING_STRUCT>& returnMap);
WIFI_11AC_MIMO_TEST_API int  ReturnErrorMessage(char *errParameter, const char *format, ...);
WIFI_11AC_MIMO_TEST_API int  LogReturnMessage(char *errMsg, int sizeOfBuf, LOGGER_LEVEL level, const char *format, ...);
WIFI_11AC_MIMO_TEST_API int  GetInputParameters( std::map<std::string, WIFI_SETTING_STRUCT>& inputMap);
WIFI_11AC_MIMO_TEST_API int  CheckDutStatus(void);
WIFI_11AC_MIMO_TEST_API int  CheckTesterConsistentStatus( std::map<std::string, std::string> &versionMap);

WIFI_11AC_MIMO_TEST_API int       WiFi_11ac_TestMode(char* strDataRate, int* cbw, int* wifiMode, int* wifiStreamNum, char* pktFormat);
WIFI_11AC_MIMO_TEST_API int       WiFiTestMode(char* strDataRate, char* chBandwidth, int* wifiMode, int* wifiStreamNum);
WIFI_11AC_MIMO_TEST_API int      GetDefaultWaveformFileName(char* filePath,char* fileType,int wifiMode,  int streamNum_11AC,
															int chBW, char* datarate, char* preamble, char* packetFormat, char* guardInterval, 
															char* waveformFileName, int bufferSize);

WIFI_11AC_MIMO_TEST_API int       GetWaveformFileName(char* perfix, char* postfix, int wifiMode,  char* bandwidth,
													  char* datarate, char* preamble, char* packetFormat, 
													  char* waveformFileName, int bufferSize);
WIFI_11AC_MIMO_TEST_API int       GetPacketNumber(int wifiMode,  char* packetFormat, int *packetNumber);
WIFI_11AC_MIMO_TEST_API double    CalcCableLossDb(int ant1, int ant2, int ant3, int ant4, double cableLoss1, double cableLoss2, double cableLoss3, double cableLoss4);
WIFI_11AC_MIMO_TEST_API int       WiFiSaveSigFile(char* fileName);
WIFI_11AC_MIMO_TEST_API int       CheckPathLossTable(int testID, int freqMHz, int ant01, int ant02, int ant03, int ant04, double *cableLoss, double *cableLossReturn, double *cableLossDb);
WIFI_11AC_MIMO_TEST_API int       CheckPathLossTableExt(int testID, int freqMHz, int ant01, int ant02, int ant03, int ant04, double *cableLoss, double *cableLossReturn, double *cableLossDb, int indicatorTxRx);
WIFI_11AC_MIMO_TEST_API int       AverageTestResult(double *resultArray, int averageTimes, int logType, double &averageResult, double &maxResult, double &minResult);
WIFI_11AC_MIMO_TEST_API double    CheckSamplingTime(int wifiMode, char *preamble11B, char *dataRate, char *packetFormat);
WIFI_11AC_MIMO_TEST_API int       CheckConnectionVsgAndAntennaPort( std::map<std::string, WIFI_SETTING_STRUCT>& inputMap, int *vsgMappingRx, int *portStatus);
WIFI_11AC_MIMO_TEST_API int       PassTestResultToDutInputContainer(std::map<std::string, WIFI_SETTING_STRUCT>& returnMap);
WIFI_11AC_MIMO_TEST_API void	  ParseSplitString(const std::string &splitteValue, const std::string spliteString, std::map<std::string, std::string> &spliteMap);

WIFI_11AC_MIMO_TEST_API int  ValidateIPAddress(char* ipAddress);
WIFI_11AC_MIMO_TEST_API int  VerifyPowerMask (int txIndex, double *freqBuf, double *maskBuf, int maskBufCnt, int wifiMode, int bssBW, int cbw);
WIFI_11AC_MIMO_TEST_API int  VerifyOBW(double *maskBuf, int maskBufCnt, double obwPercentage, int &startIndex, int &stopIndex);
WIFI_11AC_MIMO_TEST_API int  LoResultSpectralFlatness11AG(int* carrierNoUpA,
								  int* carrierNoUpB,
								  int* carrierNoLoA,
								  int* carrierNoLoB,
								  double* margin, 
								  int* leastMarginCarrier,
								  double* loLeakage, 
								  double* valueDbUpSectionA,
								  double* valueDbUpSectionB,
								  double* valueDbLoSectionA,
								  double* valueDbLoSectionB);

WIFI_11AC_MIMO_TEST_API int  LoResultSpectralFlatness11N(int basedOnVsa,
								 int wifiMode,
								 int* carrierNoUpA,
								 int* carrierNoUpB,
								 int* carrierNoLoA,
								 int* carrierNoLoB,
								 double* failMargin,
								 int* leastMarginCarrier,
								 double* loLeakage,
								 double* valueDbUpSectionA,
								 double* valueDbUpSectionB,
								 double* valueDbLoSectionA,
								 double* valueDbLoSectionB,
								 int streamIndex);

WIFI_11AC_MIMO_TEST_API int  LoResultSpectralFlatness11AC(int basedOnVsa,
								  int cbw,
								  int* carrierNoUpA,
								  int* carrierNoUpB,
								  int* carrierNoLoA,
								  int* carrierNoLoB,
								  double* failMargin,
								  int* leastMarginCarrier,
								  double* loLeakage,
								  double* valueDbUpSectionA,
								  double* valueDbUpSectionB,
								  double* valueDbLoSectionA,
								  double* valueDbLoSectionB,
								  int streamIndex);


WIFI_11AC_MIMO_TEST_API void ParseVersionString(const std::string &versionString, std::map<std::string, std::string> &versionMap);
WIFI_11AC_MIMO_TEST_API int CheckConnectionVsaMasterAndAntennaPort( std::map<std::string, WIFI_SETTING_STRUCT>& inputMap);
WIFI_11AC_MIMO_TEST_API int CheckConnectionVsaAndAntennaPort( std::map<std::string, WIFI_SETTING_STRUCT>& inputMap, int *vsaMappingTx, int *portStatus);
WIFI_11AC_MIMO_TEST_API double CalculateIQtoP(double data_i, double data_q);
WIFI_11AC_MIMO_TEST_API int GetChannelList(int bssBW,int bssPrimary, int chPrimary20,int *chPrimary40, int *chPrimary80);
WIFI_11AC_MIMO_TEST_API int CheckChannelFreq(int bssBW, int cbw, int bssPrimary, int chFreq);
WIFI_11AC_MIMO_TEST_API int CheckChPrimary20(int bssBW, int cbw, int bssPrimary, int chFreq, int chPrimary20);
WIFI_11AC_MIMO_TEST_API int	CheckCommonParameters_WiFi_11ac_MiMo(std::map<std::string, WIFI_SETTING_STRUCT>& inputMap, 
														std::map<std::string, WIFI_SETTING_STRUCT>& returnMap, 
														TM_ID WiFi_11ac_Test_ID,
														int *VsaOrVsgEnabled,
														int *bssBW, 
														int *cbw,
														int *bssPchannel,
														int *bssSchannel,
														int *wifiMode, 
														int *wifiStreamNum, 
														double *samplingTimeUs 
														//double *cableLossDb
														);			// Check common input parameters of all WiFi_11ac core functions 
#endif // end of #ifndef _WIFI_11AC_MIMO_TEST_H_

