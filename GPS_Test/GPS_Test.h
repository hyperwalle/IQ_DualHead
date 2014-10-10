/*! \mainpage GPS Test Functions
 *
 * \section intro_sec Introduction
 *
 * This project, GPS_Test, provides all test functions in compliance with 802.11 a/b/g/n standards, by
 * using LitePoint wireless testers, including IQview/IQflex, IQmax, and IQnxn.
 *
 * GPS_Test functions are managed by Test Manager (TM).  All functions are not meant to be called 
 * directly by any users.  Instead, all functions will be executed by calling TM_Run() with a corresponding
 * keyword.  TM_Run() is declared in TestManager as described below:
 *
 * \code
 * TM_API TM_RETURN TM_Run(TM_ID technologyID, const TM_STR functionKeyword);
 * \endcode 
 *
 * In order to use all GPS_Test functions, users have to call one function at the very beginning:
 *
 * \code
 * TM_ID GPS_Test;
 * GPS_Test = GPS_Test_Register();
 * \endcode
 *
 * \section revision_sec Revision History
 *
 * \subsection revision_0_0_1 0.0.1
 * Date: Jen 15, 2011
 * - First draft
 */

/*! \file GPS_Test.h
 * \brief GPS Test Functions 
 */ 


#ifndef _GPS_TEST_H_
#define _GPS_TEST_H_

#ifdef GPS_TEST_EXPORTS

#define GPS_TEST_API extern "C" __declspec(dllexport)
#else
    #ifdef __cplusplus
        #define GPS_TEST_API extern "C" __declspec(dllimport) 
    #else
        #define GPS_TEST_API extern
    #endif
#endif

#include "vDUT.h"
#include "IQlite_logger.h"
#include "TestManager.h"
#include "GPS_Test_Internal.h"

//GPS_TEST_API GPS_GLOBAL_SETTING	g_GPSGlobalSettingParam;
//GPS_TEST_API TM_ID				g_GPS_Test_ID;
//GPS_TEST_API vDUT_ID				g_GPS_Dut;

//! Register GPS Test functions with TestManager
/*!
 *
 * \return An ID (>=0) that represents the technology
 * \remark This is the only function that needs to be called at the top application level.
 *         This function installs a callback function for each one of its supported GPS Test Keywords
 *         (defined in TestManager).
 */
GPS_TEST_API int GPS_technologyRegister(void);

GPS_TEST_API int GPS_Global_Setting(void);
GPS_TEST_API int GPS_Connect_IQTester(void);
GPS_TEST_API int GPS_Disconnect_IQTester(void);

GPS_TEST_API int GPS_Insert_Dut(void);
GPS_TEST_API int GPS_Initialize_Dut(void);
GPS_TEST_API int GPS_Remove_Dut(void);

GPS_TEST_API int GPS_LoadPathLossTable(void);
GPS_TEST_API int GPS_Transmit_Scenario(void);
GPS_TEST_API int GPS_ModulatedMode(void);
GPS_TEST_API int GPS_ContinueWave(void);

GPS_TEST_API int GetGPSTestID(TM_ID* GPSTestID);
GPS_TEST_API int GetGPSDutID(vDUT_ID* GPSDutID);
GPS_TEST_API int GetGPSGlobalSetting(GPS_GLOBAL_SETTING** GPSGlobleSetting);
//GPS_TEST_API int SetTesterNumber(int testerNumber);
//GPS_TEST_API int GetTesterNumber(int *testerNumber);
//GPS_TEST_API int GetTesterType(int* testerType);
//GPS_TEST_API int SetTesterType(int testerType);
//GPS_TEST_API int SetTesterReconnect(int flag);
//GPS_TEST_API int GetTesterReconnect(int *flag);

GPS_TEST_API int  ValidateIPAddress(char* ipAddress);
GPS_TEST_API int  AverageTestResult(double *resultArray, int averageTimes, int logType, double &averageResult, double &maxResult, double &minResult);
GPS_TEST_API int  RespondToQueryInput( std::map<std::string, GPS_SETTING_STRUCT>& inputMap);
GPS_TEST_API int  RespondToQueryReturn( std::map<std::string, GPS_SETTING_STRUCT>& returnMap);
GPS_TEST_API int  ReturnTestResults( std::map<std::string, GPS_SETTING_STRUCT>& returnMap);
GPS_TEST_API int  ClearReturnParameters( std::map<std::string, GPS_SETTING_STRUCT>& returnMap);
GPS_TEST_API int  ReturnErrorMessage(char *errParameter, const char *format, ...);
GPS_TEST_API int  LogReturnMessage(char *errMsg, int sizeOfBuf, LOGGER_LEVEL level, const char *format, ...);
GPS_TEST_API int  GetInputParameters( std::map<std::string, GPS_SETTING_STRUCT>& inputMap);
GPS_TEST_API int  GetPacketLength(char* perfix, char* postfix, char* packetType, int *packetLength);
GPS_TEST_API double CheckSamplingTime(int GPSMode, char *preamble11B, char *dataRate, char *packetFormat11N);
GPS_TEST_API void ParseSplitString(const std::string &splitteValue, const std::string spliteString, std::map<std::string, std::string> &spliteMap);

#endif // end of #ifndef _GPS_TEST_H_

