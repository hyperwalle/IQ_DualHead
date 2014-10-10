#ifndef _WIMAX_TEST_H_
#define _WIMAX_TEST_H_


#ifdef WIMAX_TEST_EXPORTS

#define WIMAX_TEST_API extern "C" __declspec(dllexport)
#else
    #ifdef __cplusplus
        #define WIMAX_TEST_API extern "C" __declspec(dllimport) 
    #else
        #define WIMAX_TEST_API extern
    #endif
#endif

#include "vDUT.h"
#include "IQlite_logger.h"
#include "TestManager.h"
#include "WIMAX_Test_Internal.h"


//! Register WIMAX Test functions with TestManager
/*!
 *
 * \return An ID (>=0) that represents the technology
 * \remark This is the only function that needs to be called at the top application level.
 *         This function installs a callback function for each one of its supported WIMAX Test Keywords
 *         (defined in TestManager).
 */
WIMAX_TEST_API int WIMAX_technologyRegister(void);

WIMAX_TEST_API int WIMAX_Global_Setting(void);
WIMAX_TEST_API int WIMAX_Connect_IQTester(void);
WIMAX_TEST_API int WIMAX_Disconnect_IQTester(void);

WIMAX_TEST_API int WIMAX_Insert_Dut(void);
WIMAX_TEST_API int WIMAX_Initialize_Dut(void);
WIMAX_TEST_API int WIMAX_Remove_Dut(void);

WIMAX_TEST_API int WIMAX_Write_Eeprom(void);
WIMAX_TEST_API int WIMAX_Read_Eeprom(void);
WIMAX_TEST_API int WIMAX_Write_Mac_Address(void);
WIMAX_TEST_API int WIMAX_Read_Mac_Address(void);

WIMAX_TEST_API int WIMAX_LoadPathLossTable(void);

WIMAX_TEST_API int WIMAX_TXRX_Calibration(void);
WIMAX_TEST_API int WIMAX_Reset_Dut(void);
WIMAX_TEST_API int WIMAX_TX_Verify_Evm(void);
WIMAX_TEST_API int WIMAX_TX_Verify_Mask(void);
WIMAX_TEST_API int WIMAX_TX_Verify_Power_Step(void);
WIMAX_TEST_API int WIMAX_RX_Verify_Per(void);
WIMAX_TEST_API int WIMAX_TX_RX_Verification(void);


WIMAX_TEST_API int   GetWIMAXTestID(TM_ID* WIMAXTestID);
WIMAX_TEST_API int   GetWIMAXDutID(vDUT_ID* WIMAXDutID);
WIMAX_TEST_API int   GetWIMAXGlobalSetting(WIMAX_GLOBAL_SETTING** WIMAXGlobleSetting);
WIMAX_TEST_API int   SetTesterNumber(int testerNumber);
WIMAX_TEST_API int   GetTesterNumber(int *testerNumber);
WIMAX_TEST_API int   GetTesterType(int* testerType);
WIMAX_TEST_API int   SetTesterType(int testerType);
WIMAX_TEST_API int   SetTesterReconnect(int flag);
WIMAX_TEST_API int   GetTesterReconnect(int *flag);
WIMAX_TEST_API int   SetDutConfigChanged(bool changed);
WIMAX_TEST_API int   GetDutConfigChanged(bool *changed);
WIMAX_TEST_API int   GetDutTxActived(bool* vDutTxActived);
WIMAX_TEST_API int   SetDutTxActived(bool vDutTxActived);
WIMAX_TEST_API int   GetVsgContinuousTxMode(bool* vsgContinuousTxMode);
WIMAX_TEST_API int   SetVsgContinuousTxMode(bool* vsgContinuousTxMode);
WIMAX_TEST_API int   GetDefaultFilePath(char* defaultFilePath, int sizeOfBuf);
WIMAX_TEST_API int   SetDefaultFilePath(char* defaultFilePath);

WIMAX_TEST_API int  AnalyzeCWFrequency(double centerFreqMHz, double vsaAmpPowerDbm, double samplingTimeUs, int avgTimes, double *cwFreqResultHz, char *errorMsg);
WIMAX_TEST_API int  QuickCaptureRetry(double centerFreqMHz, double samplingTimeUS, int triggerType, double *PowerDbm, char *errorMsg);


WIMAX_TEST_API int  RespondToQueryInput( std::map<std::string, WIMAX_SETTING_STRUCT>& inputMap);
WIMAX_TEST_API int  RespondToQueryReturn( std::map<std::string, WIMAX_SETTING_STRUCT>& returnMap);
WIMAX_TEST_API int  ReturnTestResults( std::map<std::string, WIMAX_SETTING_STRUCT>& returnMap);
WIMAX_TEST_API int  ClearReturnParameters( std::map<std::string, WIMAX_SETTING_STRUCT>& returnMap);
WIMAX_TEST_API int  ReturnErrorMessage(char *errParameter, const char *format, ...);
WIMAX_TEST_API int  LogReturnMessage(char *errMsg, int sizeOfBuf, LOGGER_LEVEL level, const char *format, ...);
WIMAX_TEST_API int  GetInputParameters( std::map<std::string, WIMAX_SETTING_STRUCT>& inputMap);
WIMAX_TEST_API int  CheckDutTransmitStatus(void);

WIMAX_TEST_API int  WIMAXTestMode(char* strDataRate, char* strBandwidth, int* WIMAXMode, int* WIMAXStreamNum);
WIMAX_TEST_API int  GetWaveformFileName(char* perfix, char* postfix, int WIMAXMode, char* bandwidth, char* datarate, char* preamble, char* packetFormat11N, char* waveformFileName, int bufferSize);
WIMAX_TEST_API int  GetPacketNumber(int WIMAXMode, char* bandwidth, char* datarate, char* packetFormat11N, int *packetNumber);
WIMAX_TEST_API int  WIMAXSaveSigFile(char* fileName);
WIMAX_TEST_API int  CheckPathLossTable(int testID, int freqMHz, int ant01, int ant02, int ant03, int ant04, double *cableLoss, double *cableLossReturn, double *cableLossDb);
WIMAX_TEST_API int  CheckPathLossTableExt(int testID, int freqMHz, int ant01, int ant02, int ant03, int ant04, double *cableLoss, double *cableLossReturn, double *cableLossDb, int indicatorTxRx);
WIMAX_TEST_API int  AverageTestResult(double *resultArray, int averageTimes, int logType, double &averageResult, double &maxResult, double &minResult);
WIMAX_TEST_API double CalcCableLossDb(int ant1, int ant2, int ant3, int ant4, double cableLoss1, double cableLoss2, double cableLoss3, double cableLoss4);
WIMAX_TEST_API int CheckAntennaOrderByStream(int streamOrder, int ant1, int ant2, int ant3, int ant4, int *antOrder);

WIMAX_TEST_API int  ValidateIPAddress(char* ipAddress);
WIMAX_TEST_API void ParseSplitString(const std::string &splitteValue, const std::string spliteString, std::map<std::string, std::string> &spliteMap);

/* Return fail percentage as a output result for power mask verification of specified RBW */ /* #LPTW# cfy,-2010/06/07- */
WIMAX_TEST_API double VerifyPowerMask (double *freqBuf, double *maskBuf, int maskBufCnt, double cableLoss, int analysingRBWlevel);
                                    
WIMAX_TEST_API int  UpdateMaskLimitsByFile( char* maskLimitsName );
WIMAX_TEST_API void InitializeInternalParameters(void);


#endif // end of #ifndef _WIMAX_TEST_H_

