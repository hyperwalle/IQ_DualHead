/*! \mainpage FM Test Functions
 *
 * \section intro_sec Introduction
 *
 * This project, FM_Test, provides all test functions for testing FM Tx and RX features, by
 * using LitePoint wireless testers.
 *
 * FM_Test functions are managed by Test Manager (TM).  All functions are not meant to be called 
 * directly by any users.  Instead, all functions will be executed by calling TM_Run() with a corresponding
 * keyword.  TM_Run() is declared in TestManager as described below:
 *
 * \code
 * TM_API TM_RETURN TM_Run(TM_ID technologyID, const TM_STR functionKeyword);
 * \endcode 
 *
 * In order to use all FM_Test functions, users have to call one function at the very beginning:
 *
 * \code
 * TM_ID FM_Test;
 * FM_Test = FM_Test_Register();
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

/*! \file FM_Test.h
 * \brief FM Test Functions 
 */ 

#ifndef _FM_TEST_H_
#define _FM_TEST_H_

#ifdef FM_TEST_EXPORTS

#define FM_TEST_API extern "C" __declspec(dllexport)
#else
    #ifdef __cplusplus
        #define FM_TEST_API extern "C" __declspec(dllimport) 
    #else
        #define FM_TEST_API extern
    #endif
#endif



#include "vDUT.h"
#include "IQlite_logger.h"
#include "TestManager.h"
#include "FM_Test_Internal.h"

//! Register FM Test functions with TestManager
/*!
 *
 * \return An ID (>=0) that represents the technology
 * \remark This is the only function that needs to be called at the top application level.
 *         This function installs a callback function for each one of its supported FM Test Keywords
 *         (defined in TestManager).
 */
FM_TEST_API int FM_technologyRegister(void);

FM_TEST_API int FM_Global_Setting(void);
FM_TEST_API int FM_Insert_Dut(void);
FM_TEST_API int FM_Initialize_Dut(void);
FM_TEST_API int FM_Remove_Dut(void);
FM_TEST_API int FM_Connect_IQTester(void);
FM_TEST_API int FM_Disconnect_IQTester(void);
FM_TEST_API int FM_Run_External_Program(void);
FM_TEST_API int FM_LoadPathLossTable(void);

FM_TEST_API int FM_TX_Verify_RF_MEASUREMENTS(void);// Tx RF Measurements
FM_TEST_API int FM_TX_Verify_AUDIO(void); // includes crosstalk measurements

FM_TEST_API int FM_RX_Verify_AUDIO(void); // includes crosstalk and RSSI measurements
FM_TEST_API int FM_RX_Verify_RSSI(void); // RSSI Measurement 
FM_TEST_API int FM_RX_Verify_RDS(void); // RDS Measurement
FM_TEST_API int FM_RX_Verify_AM_SUPPRESSION(void);	// to be implemented

FM_TEST_API FM_GLOBAL_SETTING		g_FMGlobalSettingParam;
FM_TEST_API TM_ID					g_FM_Test_ID;
FM_TEST_API vDUT_ID					g_FM_Dut;

FM_TEST_API int   GetFMTestID(TM_ID* fmTestID);
FM_TEST_API int   GetFMDutID(vDUT_ID* fmDutID);
FM_TEST_API int   GetFMGlobalSetting(FM_GLOBAL_SETTING** fmGlobleSetting);
FM_TEST_API int SetTesterNumber(int testerNumber);
FM_TEST_API int GetTesterNumber(int *testerNumber);
FM_TEST_API int GetTesterType(int* testerType);
FM_TEST_API int SetTesterType(int testerType);
FM_TEST_API int SetTesterReconnect(int flag);
FM_TEST_API int GetTesterReconnect(int *flag);

FM_TEST_API int  AverageTestResult(double *resultArray, int averageTimes, int logType, double &averageResult, double &maxResult, double &minResult);
FM_TEST_API int  RespondToQueryInput( std::map<std::string, FM_SETTING_STRUCT>& inputMap);
FM_TEST_API int  RespondToQueryReturn( std::map<std::string, FM_SETTING_STRUCT>& returnMap);
FM_TEST_API int  ReturnTestResults( std::map<std::string, FM_SETTING_STRUCT>& returnMap);
FM_TEST_API int  ClearReturnParameters( std::map<std::string, FM_SETTING_STRUCT>& returnMap);
FM_TEST_API int  ReturnErrorMessage(char *errParameter, const char *format, ...);
FM_TEST_API int  LogReturnMessage(char *errMsg, int sizeOfBuf, LOGGER_LEVEL level, const char *format, ...);
FM_TEST_API int  GetInputParameters( std::map<std::string, FM_SETTING_STRUCT>& inputMap);
FM_TEST_API int  FMSaveSigFile(char* fileName);

#endif // end of #ifndef _FM_TEST_H_

