/*! \mainpage BT Test Functions
 *
 * \section intro_sec Introduction
 *
 * This project, BT_Test, provides all test functions in compliance with 802.11 a/b/g/n standards, by
 * using LitePoint wireless testers, including IQview/IQflex, IQmax, and IQnxn.
 *
 * BT_Test functions are managed by Test Manager (TM).  All functions are not meant to be called 
 * directly by any users.  Instead, all functions will be executed by calling TM_Run() with a corresponding
 * keyword.  TM_Run() is declared in TestManager as described below:
 *
 * \code
 * TM_API TM_RETURN TM_Run(TM_ID technologyID, const TM_STR functionKeyword);
 * \endcode 
 *
 * In order to use all BT_Test functions, users have to call one function at the very beginning:
 *
 * \code
 * TM_ID BT_Test;
 * BT_Test = BT_Test_Register();
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

/*! \file BT_Test.h
 * \brief BT Test Functions 
 */ 


#ifndef _BT_TEST_H_
#define _BT_TEST_H_

#if !defined (__cplusplus)
    #define BT_TEST_API extern
#elif !defined (WIN32)
    #define BT_TEST_API extern "C"
#elif defined(BT_TEST_EXPORTS)
    #define BT_TEST_API extern "C" __declspec(dllexport)
#else
    #define BT_TEST_API extern "C" __declspec(dllimport) 
#endif

#include "vDUT.h"
#include "IQlite_logger.h"
#include "TestManager.h"
#include "BT_Test_Internal.h"

//BT_TEST_API BT_GLOBAL_SETTING	g_BTGlobalSettingParam;
//BT_TEST_API TM_ID				g_BT_Test_ID;
//BT_TEST_API vDUT_ID				g_BT_Dut;

//! Register BT Test functions with TestManager
/*!
 *
 * \return An ID (>=0) that represents the technology
 * \remark This is the only function that needs to be called at the top application level.
 *         This function installs a callback function for each one of its supported BT Test Keywords
 *         (defined in TestManager).
 */
BT_TEST_API int BT_technologyRegister(void);

BT_TEST_API int BT_Global_Setting(void);
BT_TEST_API int BT_Insert_Dut(void);
BT_TEST_API int BT_Initialize_Dut(void);
// BTL LE Init
BT_TEST_API int BT_LE_Initialize(void);

BT_TEST_API int BT_Remove_Dut(void);
BT_TEST_API int BT_Connect_IQTester(void);
BT_TEST_API int BT_Disconnect_IQTester(void);
BT_TEST_API int BT_Reset_Dut(void);

BT_TEST_API int BT_TX_Verify_BDR(void);
BT_TEST_API int BT_TX_Verify_EDR(void);
BT_TEST_API int BT_TX_Verify_LE(void);

BT_TEST_API int BT_RX_Verify_BDR(void);
BT_TEST_API int BT_RX_Verify_EDR(void);
BT_TEST_API int BT_RX_Verify_LE(void);

BT_TEST_API int BT_TX_Power_Control(void);
BT_TEST_API int BT_TX_Calibration(void);

BT_TEST_API int BT_Write_BD_Address(void);
BT_TEST_API int BT_Read_BD_Address(void);
BT_TEST_API int BT_Write_DUT_Info(void);

BT_TEST_API int BT_Run_External_Program(void);
BT_TEST_API int BT_Power_Mode_Dut(void);

BT_TEST_API int BT_LoadPathLossTable(void);

BT_TEST_API int GetBTTestID(TM_ID* BTTestID);
BT_TEST_API int GetBTDutID(vDUT_ID* BTDutID);
BT_TEST_API int GetBTGlobalSetting(BT_GLOBAL_SETTING** BTGlobleSetting);
BT_TEST_API int SetTesterNumber(int testerNumber);
BT_TEST_API int GetTesterNumber(int *testerNumber);
BT_TEST_API int GetTesterType(int* testerType);
BT_TEST_API int SetTesterType(int testerType);
BT_TEST_API int SetTesterReconnect(int flag);
BT_TEST_API int GetTesterReconnect(int *flag);
BT_TEST_API int GetTesterName(int *flag);
BT_TEST_API int SetTesterName(int flag);

BT_TEST_API int  AnalyzeCWFrequency(double centerFreqMHz, double vsaAmpPowerDbm, double samplingTimeUs, int avgTimes, double *cwFreqResultHz, char *errorMsg);
BT_TEST_API int  ValidateIPAddress(char* ipAddress);
BT_TEST_API int  AverageTestResult(double *resultArray, int averageTimes, int logType, double &averageResult, double &maxResult, double &minResult);
BT_TEST_API int  RespondToQueryInput( std::map<std::string, BT_SETTING_STRUCT>& inputMap);
BT_TEST_API int  RespondToQueryReturn( std::map<std::string, BT_SETTING_STRUCT>& returnMap);
BT_TEST_API int  ReturnTestResults( std::map<std::string, BT_SETTING_STRUCT>& returnMap);
BT_TEST_API int  ClearReturnParameters( std::map<std::string, BT_SETTING_STRUCT>& returnMap);
BT_TEST_API int  ReturnErrorMessage(char *errParameter, const char *format, ...);
BT_TEST_API int  LogReturnMessage(char *errMsg, int sizeOfBuf, LOGGER_LEVEL level, const char *format, ...);
BT_TEST_API int  GetInputParameters( std::map<std::string, BT_SETTING_STRUCT>& inputMap);
BT_TEST_API int  BTSaveSigFile(char* fileName);
BT_TEST_API int  GetWaveformFileName(char* perfix, char* postfix, char* packetType, char* waveformFileName, int bufferSize);
BT_TEST_API int  GetPacketLength(char* perfix, char* postfix, char* packetType, int *packetLength);
BT_TEST_API double PacketTypeToSamplingTimeUs(char *packetType);
BT_TEST_API double CheckSamplingTime(int BTMode, char *preamble11B, char *dataRate, char *packetFormat11N);
BT_TEST_API void ParseSplitString(const std::string &splitteValue, const std::string spliteString, std::map<std::string, std::string> &spliteMap);
//added for LE 
BT_TEST_API int  AddWaveformPath( char* waveformName, char* waveformFileName, int bufferSize);

#endif // end of #ifndef _BT_TEST_H_

