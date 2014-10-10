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

/*! \file WiFi_MiMo_Test.h
 * \brief WiFi Test Functions 
 */ 


#ifndef _WIFI_MIMO_TEST_H_
#define _WIFI_MIMO_TEST_H_

#if !defined (__cplusplus)
    #define WIFI_MIMO_TEST_API extern
#elif !defined (WIN32)
    #define WIFI_MIMO_TEST_API extern "C"
#elif defined(WIFI_MIMO_TEST_EXPORTS)
    #define WIFI_MIMO_TEST_API extern "C" __declspec(dllexport)
#else
    #define WIFI_MIMO_TEST_API extern "C" __declspec(dllimport) 
#endif


#include "IQlite_logger.h"
#include "TestManager.h"
#include "WiFi_MiMo_Test_Internal.h"
#include "vDUT.h" // vDUT_ID


//! Register WiFi Test functions with TestManager
/*!
 *
 * \return An ID (>=0) that represents the technology
 * \remark This is the only function that needs to be called at the top application level.
 *         This function installs a callback function for each one of its supported WiFi Test Keywords
 *         (defined in TestManager).
 */
WIFI_MIMO_TEST_API int WIFI_MIMO_technologyRegister(void);

WIFI_MIMO_TEST_API int WiFi_Global_Setting(void);
WIFI_MIMO_TEST_API int WiFi_Insert_Dut(void);
WIFI_MIMO_TEST_API int WiFi_Initialize_Dut(void);
WIFI_MIMO_TEST_API int WiFi_Remove_Dut(void);
WIFI_MIMO_TEST_API int WiFi_Connect_IQTester(void);
WIFI_MIMO_TEST_API int WiFi_Disconnect_IQTester(void);
WIFI_MIMO_TEST_API int WiFi_TX_Verify_Evm(void);
WIFI_MIMO_TEST_API int WiFi_RX_Verify_Per(void);
WIFI_MIMO_TEST_API int WiFi_TX_Verify_Mask(void);
WIFI_MIMO_TEST_API int WiFi_TX_Verify_Power(void);
WIFI_MIMO_TEST_API int WiFi_TX_Verify_Spectrum(void);
WIFI_MIMO_TEST_API int WiFi_TX_Calibration(void);
WIFI_MIMO_TEST_API int WiFi_Write_Eeprom(void);
WIFI_MIMO_TEST_API int WiFi_Read_Eeprom(void);
WIFI_MIMO_TEST_API int WiFi_Write_BB_Register(void);
WIFI_MIMO_TEST_API int WiFi_Read_BB_Register(void);
WIFI_MIMO_TEST_API int WiFi_Write_RF_Register(void);
WIFI_MIMO_TEST_API int WiFi_Read_RF_Register(void);
WIFI_MIMO_TEST_API int WiFi_Write_Mac_Register(void);
WIFI_MIMO_TEST_API int WiFi_Read_Mac_Register(void);
WIFI_MIMO_TEST_API int WiFi_Write_Soc_Register(void);
WIFI_MIMO_TEST_API int WiFi_Read_Soc_Register(void);
WIFI_MIMO_TEST_API int WiFi_Get_Serial_Number(void);
WIFI_MIMO_TEST_API int WiFi_Write_Mac_Address(void);
WIFI_MIMO_TEST_API int WiFi_Read_Mac_Address(void);
WIFI_MIMO_TEST_API int WiFi_LoadPathLossTable(void);

WIFI_MIMO_TEST_API int WiFi_Run_External_Program(void);
WIFI_MIMO_TEST_API int WiFi_Power_Mode_Dut(void);
WIFI_MIMO_TEST_API int WiFi_Run_DUT_Command(void);

WIFI_MIMO_TEST_API WIFI_GLOBAL_SETTING	g_globalSettingParam;
WIFI_MIMO_TEST_API TM_ID					g_WiFi_Test_ID;
WIFI_MIMO_TEST_API vDUT_ID				g_WiFi_Dut;

WIFI_MIMO_TEST_API int   GetWiFiTestID(TM_ID* wifiTestID);
WIFI_MIMO_TEST_API int   SetWiFiTestID(TM_ID wifiTestID);
WIFI_MIMO_TEST_API int   GetWiFiDutID(vDUT_ID* wifiDutID);
WIFI_MIMO_TEST_API int   GetWiFiGlobalSetting(WIFI_GLOBAL_SETTING** wifiGlobleSetting);
WIFI_MIMO_TEST_API int   SetTesterNumber(int testerNumber);
WIFI_MIMO_TEST_API int   GetTesterNumber(int *testerNumber);
WIFI_MIMO_TEST_API int   GetTesterType(int* testerType);
WIFI_MIMO_TEST_API int   SetTesterType(int testerType);
WIFI_MIMO_TEST_API int   SetTesterReconnect(int flag);
WIFI_MIMO_TEST_API int   GetTesterReconnect(int *flag);
WIFI_MIMO_TEST_API int   SetDutConfigChanged(bool changed);
WIFI_MIMO_TEST_API int   GetDutConfigChanged(bool *changed);
WIFI_MIMO_TEST_API int   GetDutTxActived(bool* vDutTxActived);
WIFI_MIMO_TEST_API int   SetDutTxActived(bool vDutTxActived);
WIFI_MIMO_TEST_API int   GetTesterName(int *flag);
WIFI_MIMO_TEST_API int   SetTesterName(int flag);


WIFI_MIMO_TEST_API int  RespondToQueryInput( std::map<std::string, WIFI_SETTING_STRUCT>& inputMap);
WIFI_MIMO_TEST_API int  RespondToQueryReturn( std::map<std::string, WIFI_SETTING_STRUCT>& returnMap);
WIFI_MIMO_TEST_API int  ReturnTestResults( std::map<std::string, WIFI_SETTING_STRUCT>& returnMap);
WIFI_MIMO_TEST_API int  ClearReturnParameters( std::map<std::string, WIFI_SETTING_STRUCT>& returnMap);
WIFI_MIMO_TEST_API int  ReturnErrorMessage(char *errParameter, const char *format, ...);
WIFI_MIMO_TEST_API int  LogReturnMessage(char *errMsg, int sizeOfBuf, LOGGER_LEVEL level, const char *format, ...);
WIFI_MIMO_TEST_API int  GetInputParameters( std::map<std::string, WIFI_SETTING_STRUCT>& inputMap);
WIFI_MIMO_TEST_API int  CheckDutStatus(void);
WIFI_MIMO_TEST_API int  CheckTesterConsistentStatus( std::map<std::string, std::string> &versionMap);

WIFI_MIMO_TEST_API int       WiFiTestMode(char* strDataRate, char* strBandwidth, int* wifiMode, int* wifiStreamNum);
WIFI_MIMO_TEST_API int       GetWaveformFileName(char* perfix, char* postfix, int wifiMode, char* bandwidth, char* datarate, char* preamble, char* packetFormat11N, char* guardInterval11N, char* waveformFileName, int bufferSize);
WIFI_MIMO_TEST_API int       GetPacketNumber(int wifiMode, char* bandwidth, char* datarate, char* packetFormat11N, char* guardInterval11N, int *packetNumber);
WIFI_MIMO_TEST_API double    CalcCableLossDb(int ant1, int ant2, int ant3, int ant4, double cableLoss1, double cableLoss2, double cableLoss3, double cableLoss4);
WIFI_MIMO_TEST_API int       WiFiSaveSigFile(char* fileName);
WIFI_MIMO_TEST_API int       CheckPathLossTable(int testID, int freqMHz, int ant01, int ant02, int ant03, int ant04, double *cableLoss, double *cableLossReturn, double *cableLossDb);
WIFI_MIMO_TEST_API int       CheckPathLossTableExt(int testID, int freqMHz, int ant01, int ant02, int ant03, int ant04, double *cableLoss, double *cableLossReturn, double *cableLossDb, int indicatorTxRx);
WIFI_MIMO_TEST_API int       AverageTestResult(double *resultArray, int averageTimes, int logType, double &averageResult, double &maxResult, double &minResult);
WIFI_MIMO_TEST_API double    CheckSamplingTime(int wifiMode, char *preamble11B, char *dataRate, char *packetFormat11N);
WIFI_MIMO_TEST_API int       CheckConnectionVsgAndAntennaPort( std::map<std::string, WIFI_SETTING_STRUCT>& inputMap, int *vsgMappingRx, int *portStatus);
WIFI_MIMO_TEST_API int       PassTestResultToDutInputContainer(std::map<std::string, WIFI_SETTING_STRUCT>& returnMap);
WIFI_MIMO_TEST_API void		 ParseSplitString(const std::string &splitteValue, const std::string spliteString, std::map<std::string, std::string> &spliteMap);

#endif // end of #ifndef _WIFI_MIMO_TEST_H_

