#include "stdafx.h"
#include "TestManager.h"
#include "BT_Test.h"
#include "BT_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include "math.h"
using namespace std;

// These global variables are declared in BT_Test_Internal.cpp
extern TM_ID                 g_BT_Test_ID;
extern vDUT_ID               g_BT_Dut;

// This global variable is declared in BT_Global_Setting.cpp
extern BT_GLOBAL_SETTING g_BTGlobalSettingParam;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, BT_SETTING_STRUCT> l_txVerifyBDRParamMap;

// Return Value Container
map<string, BT_SETTING_STRUCT> l_txVerifyBDRReturnMap;

struct tagParam
{
	// Mandatory Parameters
	int       ANALYZE_POWER_ONLY;
	int       TRANSMIT_0XF0_SEQUENCE_FOR_DELTA_F1_AVG;
	int       FREQ_MHZ;								/*! The center frequency (MHz). */
	char      PACKET_TYPE[MAX_BUFFER_SIZE];			/*! The pack type to verify BDR. */
	int		  PACKET_LENGTH;						/*! The number of packet to verify BDR. */
	int       TX_POWER_LEVEL;						/*! The output power to verify BDR. */
	double    EXPECTED_TX_POWER_DBM;                /*! The expected TX power dBm at TX_POWER_LEVEL. */
	double    CABLE_LOSS_DB;						/*! The path loss of test system. */
	double    SAMPLING_TIME_US;						/*! The sampling time to verify BDR. */

} l_txVerifyBDRParam;

struct tagReturn
{
	// <Perform LP_AnalyzePower>
	double   POWER_AVERAGE_DBM;
	double   POWER_PEAK_DBM;
	double   TARGET_POWER_DBM;
	// <Perform LP_AnalyzeBluetooth>
	double   DATA_RATE_DETECT;
	double   BANDWIDTH_20DB;
	double   DELTA_F1_AVERAGE;
	double   DELTA_F2_MAX;
	double   DELTA_F2_AVERAGE;
	double   DELTA_F2_MAX_ACCESS;
	double   DELTA_F2_AV_ACCESS;
	double   DELTA_F2_F1_AV_RATIO;
	double   FREQ_DEVIATION;
	double   FREQ_DEVIATION_PK_TO_PK;
	double   FREQ_EST;
	double   FREQ_EST_HEADER;
	double   FREQ_DRIFT;
	double   MAX_FREQ_DRIFT_RATE;
	int	     PAYLOAD_ERRORS;
	double	 MAX_POWER_ACP_DBM[BT_ACP_SECTION];

	double   CABLE_LOSS_DB;						/*! The path loss of test system. */
	char     ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txVerifyBDRReturn;
#pragma endregion

#ifndef WIN32
int initTXVerifyBDRContainers = InitializeTXVerifyBDRContainers();
#endif

int ClearTxVerifyBDRReturn(void)
{
	l_txVerifyBDRParamMap.clear();
	l_txVerifyBDRReturnMap.clear();
	return 0;
}

//! BT TX Verify BDR
/*!
 * Input Parameters
 *
 *  - Mandatory
 *      -# FREQ_MHZ (double): The center frequency (MHz)
 *      -# TX_POWER (double): The power (dBm) DUT is going to transmit at the antenna port
 *
 * Return Values
 *      -# A string for error message
 *
 * \return 0 No error occurred
 * \return -1 Error(s) occurred.  Please see the returned error message for details
 */


BT_TEST_API int BT_TX_Verify_BDR(void)
{
	int    err = ERR_OK;

	bool   analysisOK = false, captureOK  = false, vDutActived = false;
	int    dummyValue   = 0;
	int	   packetLength = 0;
	int    avgIteration = 0;
	double dummyMax     = 0;
	double dummyMin     = 0;
	double samplingTimeUs = 0, cableLossDb = 0;
	char   vDutErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	/*---------------------------------------*
	 * Clear Return Parameters and Container *
	 *---------------------------------------*/
	ClearReturnParameters(l_txVerifyBDRReturnMap);

	/*------------------------*
	 * Respond to QUERY_INPUT *
	 *------------------------*/
	err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_txVerifyBDRParamMap);
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
		RespondToQueryReturn(l_txVerifyBDRReturnMap);
		return err;
	}
	else
	{
		// do nothing
	}

	/*----------------------------------------------------------------------------------------
	 * Declare vectors for storing test results: vector< double >
	 *-----------------------------------------------------------------------------------------*/
	vector< double >		powerAvEachBurst	(g_BTGlobalSettingParam.TX_BDR_AVERAGE);
	vector< double >		powerPkEachBurst	(g_BTGlobalSettingParam.TX_BDR_AVERAGE);
	vector< double >		bandwidth20dB	    (g_BTGlobalSettingParam.TX_BDR_AVERAGE);
	vector< double >		deltaF1Average	    (g_BTGlobalSettingParam.TX_BDR_AVERAGE);
	vector< double >		freqDeviation	    (g_BTGlobalSettingParam.TX_BDR_AVERAGE);
	vector< double >		freqDeviationPktoPk (g_BTGlobalSettingParam.TX_BDR_AVERAGE);
	vector< double >		freqEst				(g_BTGlobalSettingParam.TX_BDR_AVERAGE);
	vector< double >		freqDrift			(g_BTGlobalSettingParam.TX_BDR_AVERAGE);
	vector< double >		freqEstHeader		(g_BTGlobalSettingParam.TX_BDR_AVERAGE);
	vector< double >		maxFreqDriftRate	(g_BTGlobalSettingParam.TX_BDR_AVERAGE);
	vector< double >		deltaF2Max			(g_BTGlobalSettingParam.TX_BDR_AVERAGE);
	vector< double >		deltaF2Average		(g_BTGlobalSettingParam.TX_BDR_AVERAGE);
	vector< double >		deltaF2MaxAccess	(g_BTGlobalSettingParam.TX_BDR_AVERAGE);
	vector< double >		deltaF2AvAccess		(g_BTGlobalSettingParam.TX_BDR_AVERAGE);
	vector< vector<double> >    maxPowerAcpDbm	(BT_ACP_SECTION, vector<double>(g_BTGlobalSettingParam.TX_BDR_AVERAGE));

	l_txVerifyBDRReturn.PAYLOAD_ERRORS = 0;

	try
	{
		/*-----------------------------------------------------------*
		 * Both g_BT_Test_ID and g_BT_Dut need to be valid (>=0) *
		 *-----------------------------------------------------------*/
		TM_ClearReturns(g_BT_Test_ID);
		if( g_BT_Test_ID<0 || g_BT_Dut<0 )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] BT_Test_ID or BT_Dut not valid. BT_Test_ID = %d and BT_Dut = %d.\n", g_BT_Test_ID, g_BT_Dut);
			throw logMessage;
		}
		else
		{
			// do nothing
		}

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_txVerifyBDRParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Input parameters are not completed.\n");
			throw logMessage;
		}

		// Check path loss (by ant and freq)
		if ( 0==l_txVerifyBDRParam.CABLE_LOSS_DB )
		{
			err = TM_GetPathLossAtFrequency(g_BT_Test_ID, l_txVerifyBDRParam.FREQ_MHZ, &l_txVerifyBDRParam.CABLE_LOSS_DB, 0, TX_TABLE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Failed to get CABLE_LOSS_DB from path loss table.\n", err);
				throw logMessage;
			}
		}
		else
		{
			// do nothing
		}

		/*----------------------------*
		 * Disable VSG output signal  *
		 *----------------------------*/
		// make sure no signal is generated by the VSG
		err = ::LP_EnableVsgRF(0);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
		}

#pragma region Configure DUT to transmit
		/*-------------------------------------------*
		 * Configure DUT to transmit - PRBS9 Pattern *
		 *-------------------------------------------*/
		// Set DUT RF frequency, Tx power, data rate
		// And clear vDUT parameters at beginning.
		vDUT_ClearParameters(g_BT_Dut);

		vDUT_AddIntegerParameter(g_BT_Dut, "FREQ_MHZ",			l_txVerifyBDRParam.FREQ_MHZ);
		// Description				Enum in the IQFact
		// 0x00 8-bit Pattern		ZERO =0: all zero
		// 0xFF 8-bit Pattern      ONE1 =1: all ones
		// 0xAA 8-bit Pattern      HEX_A=2: 1010
		// 0xF0 8-bit Pattern		HEX_F0=3: 11110000
		// PRBS9 Pattern           RANDOM=4;  PRBS=0
		vDUT_AddIntegerParameter(g_BT_Dut, "MODULATION_TYPE",   BT_PATTERN_PRBS);	//{0x04, "PRBS9 Pattern"}
		// {0x00, "ACL  EDR"},
		// {0x01, "ACL  Basic"},
		// {0x02, "eSCO EDR"},
		// {0x03, "eSCO Basic"},
		// {0x04, "SCO  Basic"}
		vDUT_AddIntegerParameter(g_BT_Dut, "LOGIC_CHANNEL",     1);	//{0x01, "ACL Basic"}
		vDUT_AddStringParameter (g_BT_Dut, "PACKET_TYPE",	    l_txVerifyBDRParam.PACKET_TYPE);

		// Check packet length
		if (0==l_txVerifyBDRParam.PACKET_LENGTH)
		{
			GetPacketLength("BER", "PACKETS_LENGTH", l_txVerifyBDRParam.PACKET_TYPE, &packetLength);
		}
		else
		{
			packetLength = l_txVerifyBDRParam.PACKET_LENGTH;
		}
		vDUT_AddIntegerParameter(g_BT_Dut, "PACKET_LENGTH",     packetLength);
		vDUT_AddIntegerParameter(g_BT_Dut, "TX_POWER_LEVEL",	l_txVerifyBDRParam.TX_POWER_LEVEL);
		vDUT_AddDoubleParameter(g_BT_Dut, "EXPECTED_TX_POWER_DBM", l_txVerifyBDRParam.EXPECTED_TX_POWER_DBM);

		err = vDUT_Run(g_BT_Dut, "QUERY_POWER_DBM");
		if ( ERR_OK!=err )
		{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.
			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
			{
				err = -1;	// set err to -1 indicates "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else	// Return generic error message to the upper layer.
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(QUERY_POWER_DBM) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			// do nothing
		}

		err = vDUT_GetDoubleReturn(g_BT_Dut, "POWER_DBM", &l_txVerifyBDRReturn.TARGET_POWER_DBM);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_GetDoubleReturn(POWER_DBM) return error.\n");
			throw logMessage;
		}

		err = vDUT_Run(g_BT_Dut, "TX_START");
		if ( ERR_OK!=err )
		{
			vDutActived = false;
			// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer
			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
			{
				err = -1;	// set err to -1 indicates "Error"
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else	// Return generic error message to the upper layer
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(TX_START) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			vDutActived = true;
		}

		// Delay for DUT settle
		if (0!=g_BTGlobalSettingParam.DUT_TX_SETTLE_TIME_MS)
		{
			Sleep(g_BTGlobalSettingParam.DUT_TX_SETTLE_TIME_MS);
		}
		else
		{
			// do nothing
		}
#pragma endregion

#pragma region Setup LP Tester and Capture
		/*--------------------*
		 * Setup IQtester VSA *
		 *--------------------*/
		double peakToAvgRatio = g_BTGlobalSettingParam.IQ_P_TO_A_BDR;
		cableLossDb = l_txVerifyBDRParam.CABLE_LOSS_DB;

		err = LP_SetVsaBluetooth(  l_txVerifyBDRParam.FREQ_MHZ*1e6,
				l_txVerifyBDRReturn.TARGET_POWER_DBM-cableLossDb+peakToAvgRatio,
				g_BTGlobalSettingParam.VSA_PORT,
				g_BTGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
				g_BTGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000
				);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to setup VSA.\n");
			throw logMessage;
		}

		// Check capture time
		if (0==l_txVerifyBDRParam.SAMPLING_TIME_US)
		{
			samplingTimeUs = PacketTypeToSamplingTimeUs(l_txVerifyBDRParam.PACKET_TYPE);
		}
		else	// SAMPLING_TIME_US != 0
		{
			samplingTimeUs = l_txVerifyBDRParam.SAMPLING_TIME_US;
		}

		if ( 1==l_txVerifyBDRParam.ANALYZE_POWER_ONLY )
		{
			/*--------------------------------*
			 * Start "while" loop for average *
			 *--------------------------------*/
			avgIteration = 0;
			while ( avgIteration<g_BTGlobalSettingParam.TX_BDR_AVERAGE )
			{
				analysisOK = false;
				captureOK  = false;

				/*----------------------------*
				 * Perform normal VSA capture *
				 *----------------------------*/
				err = LP_VsaDataCapture( samplingTimeUs/1000000, g_BTGlobalSettingParam.VSA_TRIGGER_TYPE );
				if( ERR_OK!=err )	// capture is failed
				{
					double rxAmpl;
					LP_Agc(&rxAmpl, TRUE);	// do auto range on all testers
					err = LP_VsaDataCapture( samplingTimeUs/1000000, g_BTGlobalSettingParam.VSA_TRIGGER_TYPE );
					if( ERR_OK!=err )	// capture is failed
					{
						// Fail Capture
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to capture signal at %d MHz.\n", l_txVerifyBDRParam.FREQ_MHZ);
						throw logMessage;
					}
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] LP_VsaDataCapture() at %d MHz return OK.\n", l_txVerifyBDRParam.FREQ_MHZ);
				}

				/*--------------*
				 *  Capture OK  *
				 *--------------*/
				captureOK = true;
				if (1==g_BTGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
				{
					// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_BDR_PWR_SaveAlways", l_txVerifyBDRParam.FREQ_MHZ, l_txVerifyBDRParam.PACKET_TYPE);
					BTSaveSigFile(sigFileNameBuffer);
				}
				else
				{
					// do nothing
				}

				/*--------------------------*
				 *  Perform power analysis  *
				 *--------------------------*/
				double dummy_T_INTERVAL      = 3.2;
				double dummy_MAX_POW_DIFF_DB = 15.0;
				err = LP_AnalyzePower( dummy_T_INTERVAL/1000000, dummy_MAX_POW_DIFF_DB );
				if (ERR_OK!=err)
				{	// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_POWER_AnalysisFailed", l_txVerifyBDRParam.FREQ_MHZ, l_txVerifyBDRParam.PACKET_TYPE);
					BTSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] LP_AnalyzePower() return error.\n");
					throw logMessage;
				}
				else
				{
					// do nothing
				}

#pragma region Retrieve analysis Results
				/*-----------------------------*
				 *  Retrieve analysis Results  *
				 *-----------------------------*/
				analysisOK = true;

				// powerAvEachBurst
				powerAvEachBurst[avgIteration] = LP_GetScalarMeasurement("P_av_no_gap_all_dBm",0);
				if ( -99.00 >= powerAvEachBurst[avgIteration] )
				{
					analysisOK = false;
					l_txVerifyBDRReturn.POWER_AVERAGE_DBM = NA_NUMBER;
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_POWER_AnalysisFailed", l_txVerifyBDRParam.FREQ_MHZ, l_txVerifyBDRParam.PACKET_TYPE);
					BTSaveSigFile(sigFileNameBuffer);
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(P_av_no_gap_all_dBm) return error.\n");
					throw logMessage;
				}
				else
				{
					powerAvEachBurst[avgIteration] = powerAvEachBurst[avgIteration] + cableLossDb;
				}

				// powerPkEachBurst
				powerPkEachBurst[avgIteration] = LP_GetScalarMeasurement("P_pk_each_burst_dBm",0);
				if ( -99.00 >= powerPkEachBurst[avgIteration] )
				{
					analysisOK = false;
					l_txVerifyBDRReturn.POWER_PEAK_DBM = NA_NUMBER;
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_POWER_AnalysisFailed", l_txVerifyBDRParam.FREQ_MHZ, l_txVerifyBDRParam.PACKET_TYPE);
					BTSaveSigFile(sigFileNameBuffer);
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(P_pk_each_burst_dBm) return error.\n");
					throw logMessage;
				}
				else
				{
					powerPkEachBurst[avgIteration] = powerPkEachBurst[avgIteration] + cableLossDb;
				}

				avgIteration++;
#pragma endregion
			}	// End - avgIteration

#pragma region Averaging and Saving Test Result
			/*----------------------------------*
			 * Averaging and Saving Test Result *
			 *----------------------------------*/
			if ( (ERR_OK==err) && captureOK && analysisOK )
			{
				// Average Power test result
				err = ::AverageTestResult(&powerAvEachBurst[0], avgIteration, LOG_10, l_txVerifyBDRReturn.POWER_AVERAGE_DBM, dummyMax, dummyMin);
				// Peak Power test result
				err = ::AverageTestResult(&powerPkEachBurst[0], avgIteration, LOG_10, dummyMax, l_txVerifyBDRReturn.POWER_PEAK_DBM, dummyMin);
			}
			else
			{
				// do nothing
			}
#pragma endregion

			/*-----------*
			 *  Tx Stop  *
			 *-----------*/
			err = vDUT_Run(g_BT_Dut, "TX_STOP");
			if ( ERR_OK!=err )
			{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer
				err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
				{
					err = -1;	// indicates that there is an error
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
					throw logMessage;
				}
				else	// Return generic error message to the upper layer
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(TX_STOP) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				// do nothing
			}

		}
		else	// otherwise, perform Bluetooth analysis
		{
			/*--------------------------------*
			 * Start "while" loop for average *
			 *--------------------------------*/
			avgIteration = 0;
			while ( avgIteration<g_BTGlobalSettingParam.TX_BDR_AVERAGE )
			{
				analysisOK = false;
				captureOK  = false;

				// Perform normal VSA capture
				err = LP_VsaDataCapture( samplingTimeUs/1000000, g_BTGlobalSettingParam.VSA_TRIGGER_TYPE );
				if( ERR_OK!=err )	// capture is failed
				{
					double rxAmpl;
					LP_Agc(&rxAmpl, TRUE);	// do auto range on all testers
					err = LP_VsaDataCapture( samplingTimeUs/1000000, g_BTGlobalSettingParam.VSA_TRIGGER_TYPE );
					if( ERR_OK!=err )	// capture is failed
					{
						// Fail Capture
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to capture signal at %d MHz.\n", l_txVerifyBDRParam.FREQ_MHZ);
						throw logMessage;
					}
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] LP_VsaDataCapture() at %d MHz return OK.\n", l_txVerifyBDRParam.FREQ_MHZ);
				}

				/*--------------*
				 *  Capture OK  *
				 *--------------*/
				captureOK = true;
				if (1==g_BTGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
				{
					// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_BDR_PRBS9_SaveAlways", l_txVerifyBDRParam.FREQ_MHZ, l_txVerifyBDRParam.PACKET_TYPE);
					BTSaveSigFile(sigFileNameBuffer);
				}
				else
				{
					// do nothing
				}

				/*------------------------------*
				 *  Perform Bluetooth analysis  *
				 *------------------------------*/
				err = LP_AnalyzeBluetooth( 1, "AllPlus" );		// 0 (auto), or 1, 2, 3;
				if (ERR_OK!=err)
				{	// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_BDR_AnalysisFailed", l_txVerifyBDRParam.FREQ_MHZ, l_txVerifyBDRParam.PACKET_TYPE);
					BTSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] LP_AnalyzeBluetooth() return error.\n");
					throw logMessage;
				}
				else
				{
					// do nothing
				}

#pragma region Retrieve analysis Results
				/*-----------------------------*
				 *  Retrieve analysis results  *
				 *-----------------------------*/
				analysisOK = true;

				if ( 1==LP_GetScalarMeasurement("valid",0) )
				{
					// powerAvEachBurst
					powerAvEachBurst[avgIteration] = LP_GetScalarMeasurement("P_av_each_burst", 0);
					powerAvEachBurst[avgIteration] = 10 * log10(powerAvEachBurst[avgIteration]);
					if ( -99.00 >= powerAvEachBurst[avgIteration] )
					{
						analysisOK = false;
						l_txVerifyBDRReturn.POWER_AVERAGE_DBM = NA_NUMBER;
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_BDR_AnalysisFailed", l_txVerifyBDRParam.FREQ_MHZ, l_txVerifyBDRParam.PACKET_TYPE);
						BTSaveSigFile(sigFileNameBuffer);
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(P_av_each_burst) return error.\n");
						throw logMessage;
					}
					else
					{
						powerAvEachBurst[avgIteration] = powerAvEachBurst[avgIteration] + cableLossDb;
					}
					// powerPkEachBurst
					powerPkEachBurst[avgIteration] = LP_GetScalarMeasurement("P_pk_each_burst", 0);
					powerPkEachBurst[avgIteration] = 10 * log10(powerPkEachBurst[avgIteration]);
					if ( -99.00 >= powerPkEachBurst[avgIteration] )
					{
						analysisOK = false;
						l_txVerifyBDRReturn.POWER_PEAK_DBM = NA_NUMBER;
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_BDR_AnalysisFailed", l_txVerifyBDRParam.FREQ_MHZ, l_txVerifyBDRParam.PACKET_TYPE);
						BTSaveSigFile(sigFileNameBuffer);
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(P_pk_each_burst) return error.\n");
						throw logMessage;
					}
					else
					{
						powerPkEachBurst[avgIteration] = powerPkEachBurst[avgIteration] + cableLossDb;
					}
					l_txVerifyBDRReturn.DATA_RATE_DETECT	 = LP_GetScalarMeasurement("dataRateDetect",0);
					bandwidth20dB[avgIteration]				 = LP_GetScalarMeasurement("bandwidth20dB", 0) / 1000;		// MHz
					l_txVerifyBDRReturn.PAYLOAD_ERRORS += (int)LP_GetScalarMeasurement("payloadErrors", 0);
					freqEst[avgIteration]					 = LP_GetScalarMeasurement("freq_est",  0) / 1000;

					if ( 1==LP_GetScalarMeasurement("acpErrValid",0) )
					{
						for (int i=0;i<=10;i++)
						{
							// maxPowerAcpDbm
							maxPowerAcpDbm[i][avgIteration]			 = LP_GetScalarMeasurement("maxPowerAcpDbm", i) + cableLossDb;
						}
					}
					else
					{
						analysisOK = false;
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_BDR_ResultsFailed", l_txVerifyBDRParam.FREQ_MHZ, l_txVerifyBDRParam.PACKET_TYPE);
						BTSaveSigFile(sigFileNameBuffer);
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "BT TX BDR (ACP) retrieve analysis results not valid.\n");
						throw logMessage;
					}
				}
				else
				{
					analysisOK = false;
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_BDR_ResultsFailed", l_txVerifyBDRParam.FREQ_MHZ, l_txVerifyBDRParam.PACKET_TYPE);
					BTSaveSigFile(sigFileNameBuffer);
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "BT TX BDR retrieve analysis results not valid.\n");
					throw logMessage;
				}

				avgIteration++;
#pragma endregion
			}	// End - avgIteration

#pragma region Averaging and Saving Test Result
			/*----------------------------------*
			 * Averaging and Saving Test Result *
			 *----------------------------------*/
			if ( (ERR_OK==err) && captureOK && analysisOK )
			{
				// Average Power test result
				err = ::AverageTestResult(&powerAvEachBurst[0], avgIteration, LOG_10, l_txVerifyBDRReturn.POWER_AVERAGE_DBM, dummyMax, dummyMin);
				// Peak Power test result
				err = ::AverageTestResult(&powerPkEachBurst[0], avgIteration, LOG_10, dummyMax, l_txVerifyBDRReturn.POWER_PEAK_DBM, dummyMin);
				// Bandwidth_20dB test result
				err = ::AverageTestResult(&bandwidth20dB[0], avgIteration, Linear, l_txVerifyBDRReturn.BANDWIDTH_20DB, dummyMax, dummyMin);
				//Initial Carrier Frequency Tolerance
				err = ::AverageTestResult(&freqEst[0], avgIteration, Linear, l_txVerifyBDRReturn.FREQ_EST, dummyMax, dummyMin);

				// ACP
				for (int i=0;i<=10;i++)
				{
					err = ::AverageTestResult(&maxPowerAcpDbm[i][0], avgIteration, LOG_10, l_txVerifyBDRReturn.MAX_POWER_ACP_DBM[i], dummyMax, dummyMin);
				}
			}
			else
			{
				// do nothing
			}
#pragma endregion

			/*-----------*
			 *  Tx Stop  *
			 *-----------*/
			err = vDUT_Run(g_BT_Dut, "TX_STOP");
			if ( ERR_OK!=err )
			{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer
				err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
				{
					err = -1;	// indicates that there is an "Error"
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
					throw logMessage;
				}
				else	// Return error message
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(TX_STOP) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				// do nothing
			}

			if ( 1==l_txVerifyBDRParam.TRANSMIT_0XF0_SEQUENCE_FOR_DELTA_F1_AVG )
			{
#pragma region Configure DUT to transmit
				/*---------------------------------------------------*
				 * Configure DUT to transmit - 11110000 data pattern *
				 *---------------------------------------------------*/
				// Set DUT RF frequency, tx power, data rate
				// And clear vDUT parameters at beginning.
				vDUT_ClearParameters(g_BT_Dut);

				vDUT_AddIntegerParameter(g_BT_Dut, "FREQ_MHZ",			l_txVerifyBDRParam.FREQ_MHZ);
				vDUT_AddIntegerParameter(g_BT_Dut, "MODULATION_TYPE",   BT_PATTERN_F0);	// HEX_F0=3: 11110000
				vDUT_AddIntegerParameter(g_BT_Dut, "LOGIC_CHANNEL",     1);	//{0x01, "ACL Basic"}
				vDUT_AddStringParameter (g_BT_Dut, "PACKET_TYPE",	    l_txVerifyBDRParam.PACKET_TYPE);
				// Check packet length
				if (0==l_txVerifyBDRParam.PACKET_LENGTH)
				{
					GetPacketLength("BER", "PACKETS_LENGTH", l_txVerifyBDRParam.PACKET_TYPE, &packetLength);
				}
				else
				{
					packetLength = l_txVerifyBDRParam.PACKET_LENGTH;
				}
				vDUT_AddIntegerParameter(g_BT_Dut, "PACKET_LENGTH",     packetLength);
				vDUT_AddIntegerParameter(g_BT_Dut, "TX_POWER_LEVEL",	l_txVerifyBDRParam.TX_POWER_LEVEL);
				vDUT_AddDoubleParameter(g_BT_Dut, "EXPECTED_TX_POWER_DBM", l_txVerifyBDRParam.EXPECTED_TX_POWER_DBM);

				err = vDUT_Run(g_BT_Dut, "TX_START");
				if ( ERR_OK!=err )
				{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.
					err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
					{
						err = -1;	// Indicates that there is an "Error"
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
						throw logMessage;
					}
					else	// Returns error message
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(TX_START) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					// do nothing
				}

				// Delay for DUT settle
				if (0!=g_BTGlobalSettingParam.DUT_TX_SETTLE_TIME_MS)
				{
					Sleep(g_BTGlobalSettingParam.DUT_TX_SETTLE_TIME_MS);
				}
				else
				{
					// do nothing
				}
#pragma endregion

				/*--------------------------------*
				 * Start "while" loop for average *
				 *--------------------------------*/
				avgIteration = 0;
				while ( avgIteration<g_BTGlobalSettingParam.TX_BDR_AVERAGE )
				{
					analysisOK = false;
					captureOK  = false;

					/*----------------------------*
					 * Perform VSA capture *
					 *----------------------------*/
					err = LP_VsaDataCapture( samplingTimeUs/1000000, g_BTGlobalSettingParam.VSA_TRIGGER_TYPE );
					if( ERR_OK!=err )	// capture is failed
					{
						// Fail Capture
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to capture signal at %d MHz.\n", l_txVerifyBDRParam.FREQ_MHZ);
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] LP_VsaDataCapture() at %d MHz return OK.\n", l_txVerifyBDRParam.FREQ_MHZ);
					}

					/*--------------*
					 *  Capture OK  *
					 *--------------*/
					captureOK = true;
					if (1==g_BTGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
					{
						// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_BDR_0xF0_SaveAlways", l_txVerifyBDRParam.FREQ_MHZ, l_txVerifyBDRParam.PACKET_TYPE);
						BTSaveSigFile(sigFileNameBuffer);
					}
					else
					{
						// do nothing
					}

					/*------------------------------*
					 *  Perform Bluetooth analysis  *
					 *------------------------------*/
					err = LP_AnalyzeBluetooth( 1 );		// 0 (auto), or 1, 2, 3
					if (ERR_OK!=err)
					{	// Fail Analysis, thus save capture (Signal File) for debug
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_BDR_AnalysisFailed", l_txVerifyBDRParam.FREQ_MHZ, l_txVerifyBDRParam.PACKET_TYPE);
						BTSaveSigFile(sigFileNameBuffer);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] LP_AnalyzeBluetooth() return error.\n");
						throw logMessage;
					}
					else
					{
						// do nothing
					}


#pragma region Retrieve analysis Results
					/*-----------------------------*
					 *  Retrieve analysis results  *
					 *-----------------------------*/
					analysisOK = true;

					if ( 1==LP_GetScalarMeasurement("valid",0) )
					{
						deltaF1Average[avgIteration]		= LP_GetScalarMeasurement("deltaF1Average", 0) / 1000;	// Requires 00001111 data pattern
						freqDeviation[avgIteration]			= LP_GetScalarMeasurement("freq_deviation", 0) / 1000;
						freqDeviationPktoPk[avgIteration]	= LP_GetScalarMeasurement("freq_deviationpktopk", 0) / 1000;
					}
					else
					{
						analysisOK = false;
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_BDR_ResultsFailed", l_txVerifyBDRParam.FREQ_MHZ, l_txVerifyBDRParam.PACKET_TYPE);
						BTSaveSigFile(sigFileNameBuffer);
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "BT TX BDR retrieve analysis results not valid.\n");
						throw logMessage;
					}

					avgIteration++;
#pragma endregion
				}	// End - avgIteration

#pragma region Averaging and Saving Test Result
				/*----------------------------------*
				 * Averaging and Saving Test Result *
				 *----------------------------------*/
				if ( (ERR_OK==err) && captureOK && analysisOK )
				{
					err = ::AverageTestResult(&deltaF1Average[0], avgIteration, Linear, l_txVerifyBDRReturn.DELTA_F1_AVERAGE, dummyMax, dummyMin);
					err = ::AverageTestResult(&freqDeviation[0], avgIteration, Linear, l_txVerifyBDRReturn.FREQ_DEVIATION, dummyMax, dummyMin);
					err = ::AverageTestResult(&freqDeviationPktoPk[0], avgIteration, Linear, l_txVerifyBDRReturn.FREQ_DEVIATION_PK_TO_PK, dummyMax, dummyMin);
				}
				else
				{
					// do nothing
				}
#pragma endregion

				/*-----------*
				 *  Tx Stop  *
				 *-----------*/
				err = vDUT_Run(g_BT_Dut, "TX_STOP");
				if ( ERR_OK!=err )
				{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.
					err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// indicates that there is an "Error"
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
						throw logMessage;
					}
					else	// Return error message
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(TX_STOP) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					// do nothing
				}
			}
			else
			{
				// do nothing
			}

#pragma region Configure DUT to transmit
			/*------------------------------------------------------*
			 * Configure DUT to transmit - alternating data pattern *
			 *------------------------------------------------------*/
			// Set DUT RF frequency, tx power, data rate
			// And clear vDUT parameters at the beginning.
			vDUT_ClearParameters(g_BT_Dut);

			vDUT_AddIntegerParameter(g_BT_Dut, "FREQ_MHZ",			l_txVerifyBDRParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_BT_Dut, "MODULATION_TYPE",   BT_PATTERN_AA);	// HEX_A=2: 1010
			vDUT_AddIntegerParameter(g_BT_Dut, "LOGIC_CHANNEL",     1);	//{0x01, "ACL Basic"}
			vDUT_AddStringParameter (g_BT_Dut, "PACKET_TYPE",	    l_txVerifyBDRParam.PACKET_TYPE);
			// Check packet length
			if (0==l_txVerifyBDRParam.PACKET_LENGTH)
			{
				GetPacketLength("BER", "PACKETS_LENGTH", l_txVerifyBDRParam.PACKET_TYPE, &packetLength);
			}
			else
			{
				packetLength = l_txVerifyBDRParam.PACKET_LENGTH;
			}
			vDUT_AddIntegerParameter(g_BT_Dut, "PACKET_LENGTH",     packetLength);
			vDUT_AddIntegerParameter(g_BT_Dut, "TX_POWER_LEVEL",	l_txVerifyBDRParam.TX_POWER_LEVEL);
			vDUT_AddDoubleParameter(g_BT_Dut, "EXPECTED_TX_POWER_DBM", l_txVerifyBDRParam.EXPECTED_TX_POWER_DBM);

			err = vDUT_Run(g_BT_Dut, "TX_START");
			if ( ERR_OK!=err )
			{
				vDutActived = false;
				// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.
				err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
				{
					err = -1;	//indicates that there is an "Error"
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
					throw logMessage;
				}
				else	// Return error message
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(TX_START) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				vDutActived = true;
			}

			// Delay for DUT settle
			if (0!=g_BTGlobalSettingParam.DUT_TX_SETTLE_TIME_MS)
			{
				Sleep(g_BTGlobalSettingParam.DUT_TX_SETTLE_TIME_MS);
			}
			else
			{
				// do nothing
			}
#pragma endregion

			/*--------------------------------*
			 * Start "while" loop for average *
			 *--------------------------------*/
			avgIteration = 0;
			while ( avgIteration<g_BTGlobalSettingParam.TX_BDR_AVERAGE )
			{
				analysisOK = false;
				captureOK  = false;

				/*----------------------------*
				 * Perform normal VSA capture *
				 *----------------------------*/
				err = LP_VsaDataCapture( samplingTimeUs/1000000, g_BTGlobalSettingParam.VSA_TRIGGER_TYPE );
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to capture signal at %d MHz.\n", l_txVerifyBDRParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] LP_VsaDataCapture() at %d MHz return OK.\n", l_txVerifyBDRParam.FREQ_MHZ);
				}

				/*--------------*
				 *  Capture OK  *
				 *--------------*/
				captureOK = true;
				if (1==g_BTGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
				{
					// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_BDR_0xAA_SaveAlways", l_txVerifyBDRParam.FREQ_MHZ, l_txVerifyBDRParam.PACKET_TYPE);
					BTSaveSigFile(sigFileNameBuffer);
				}
				else
				{
					// do nothing
				}

				/*------------------------------*
				 *  Perform Bluetooth analysis  *
				 *------------------------------*/
				err = LP_AnalyzeBluetooth( 1 );		// 0 (auto), or 1, 2, 3
				if (ERR_OK!=err)
				{	// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_BDR_AnalysisFailed", l_txVerifyBDRParam.FREQ_MHZ, l_txVerifyBDRParam.PACKET_TYPE);
					BTSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] LP_AnalyzeBluetooth() return error.\n");
					throw logMessage;
				}
				else
				{
					// do nothing
				}

#pragma region Retrieve analysis Results
				/*-----------------------------*
				 *  Retrieve analysis results  *
				 *-----------------------------*/
				analysisOK = true;

				if ( 1==LP_GetScalarMeasurement("valid",0) )
				{
					//Get result "freq_est" while payload is PRBS9
					//freqEst[avgIteration]			= LP_GetScalarMeasurement("freq_est",  0) / 1000;
					freqDrift[avgIteration]			= LP_GetScalarMeasurement("freq_drift",0) / 1000;
					freqEstHeader[avgIteration]		= LP_GetScalarMeasurement("freq_estHeader",  0) / 1000;
					maxFreqDriftRate[avgIteration]	= LP_GetScalarMeasurement("maxfreqDriftRate",0) / 1000;

					deltaF2Max[avgIteration]		= LP_GetScalarMeasurement("deltaF2Max",    0) / 1000;	// Requires alternating data pattern
					deltaF2Average[avgIteration]	= LP_GetScalarMeasurement("deltaF2Average",0) / 1000;	// Requires alternating data pattern
					deltaF2MaxAccess[avgIteration]	= LP_GetScalarMeasurement("deltaF2MaxAccess",0) / 1000;
					deltaF2AvAccess[avgIteration]	= LP_GetScalarMeasurement("deltaF2AvAccess" ,0) / 1000;
				}
				else
				{
					analysisOK = false;
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_BDR_ResultsFailed", l_txVerifyBDRParam.FREQ_MHZ, l_txVerifyBDRParam.PACKET_TYPE);
					BTSaveSigFile(sigFileNameBuffer);
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "BT TX BDR retrieve analysis results not valid.\n");
					throw logMessage;
				}
				avgIteration++;
#pragma endregion
			}	// End - avgIteration

#pragma region Averaging and Saving Test Result
			/*----------------------------------*
			 * Averaging and Saving Test Result *
			 *----------------------------------*/
			if ( (ERR_OK==err) && captureOK && analysisOK )
			{
				double dummyDouble = 0.0;
				//Get result "freq_est" while payload is PRBS9
				//err = ::AverageTestResult(&freqEst[0], avgIteration, Linear, l_txVerifyBDRReturn.FREQ_EST, dummyMax, dummyMin);
				err = ::AverageTestResult(&freqDrift[0], avgIteration, Linear, l_txVerifyBDRReturn.FREQ_DRIFT, dummyMax, dummyMin);
				err = ::AverageTestResult(&freqEstHeader[0], avgIteration, Linear, l_txVerifyBDRReturn.FREQ_EST_HEADER, dummyMax, dummyMin);
				err = ::AverageTestResult(&maxFreqDriftRate[0], avgIteration, Linear, l_txVerifyBDRReturn.MAX_FREQ_DRIFT_RATE, dummyMax, dummyMin);
				err = ::AverageTestResult(&deltaF2Max[0], avgIteration, Linear, dummyDouble, dummyMax, l_txVerifyBDRReturn.DELTA_F2_MAX);
				err = ::AverageTestResult(&deltaF2Average[0], avgIteration, Linear, l_txVerifyBDRReturn.DELTA_F2_AVERAGE, dummyMax, dummyMin);
				err = ::AverageTestResult(&deltaF2MaxAccess[0], avgIteration, Linear, dummyDouble, dummyMax, l_txVerifyBDRReturn.DELTA_F2_MAX_ACCESS);
				err = ::AverageTestResult(&deltaF2AvAccess[0], avgIteration, Linear, l_txVerifyBDRReturn.DELTA_F2_AV_ACCESS, dummyMax, dummyMin);
			}
			else
			{
				// do nothing
			}
#pragma endregion

			/*-----------*
			 *  Tx Stop  *
			 *-----------*/
			err = vDUT_Run(g_BT_Dut, "TX_STOP");
			if ( ERR_OK!=err )
			{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.
				err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
				{
					err = -1;	// indicates that there is an "Error"
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
					throw logMessage;
				}
				else	// return error message
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(TX_STOP) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				vDutActived = false;
			}

		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err && captureOK && analysisOK )
		{
			// Return Path Loss (dB)
			l_txVerifyBDRReturn.CABLE_LOSS_DB = l_txVerifyBDRParam.CABLE_LOSS_DB;

			// DELTA_F2_AV / DELTA_F1_AV >= 0.8
			if (l_txVerifyBDRReturn.DELTA_F1_AVERAGE!=(NA_NUMBER/1000))
			{
				if (l_txVerifyBDRReturn.DELTA_F2_AVERAGE!=(NA_NUMBER/1000))
					l_txVerifyBDRReturn.DELTA_F2_F1_AV_RATIO = l_txVerifyBDRReturn.DELTA_F2_AVERAGE / l_txVerifyBDRReturn.DELTA_F1_AVERAGE;
				else if (l_txVerifyBDRReturn.DELTA_F2_AV_ACCESS!=(NA_NUMBER/1000))
					l_txVerifyBDRReturn.DELTA_F2_F1_AV_RATIO = l_txVerifyBDRReturn.DELTA_F2_AV_ACCESS / l_txVerifyBDRReturn.DELTA_F1_AVERAGE;
			}
			else
			{
				// do nothing
			}

			sprintf_s(l_txVerifyBDRReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txVerifyBDRReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_txVerifyBDRReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		ReturnErrorMessage(l_txVerifyBDRReturn.ERROR_MESSAGE, "[BT] Unknown Error!\n");
		err = -1;
	}

	// This is a special case and happens only when certain errors occur before the TX_STOP.
	// This is handled by error handling, but TX_STOP must be handled manually.

	if ( vDutActived )
	{
		vDUT_Run(g_BT_Dut, "TX_STOP");
	}
	else
	{
		// do nothing
	}

	// Free memory
	powerAvEachBurst.clear();
	powerPkEachBurst.clear();
	bandwidth20dB.clear();
	deltaF1Average.clear();
	freqDeviation.clear();
	freqDeviationPktoPk.clear();
	freqEst.clear();
	freqEstHeader.clear();
	freqDrift.clear();
	maxFreqDriftRate.clear();
	deltaF2Max.clear();
	deltaF2Average.clear();
	deltaF2MaxAccess.clear();
	deltaF2AvAccess.clear();
	maxPowerAcpDbm.clear();


	return err;
}

int InitializeTXVerifyBDRContainers(void)
{
	/*------------------*
	 * Input Parameters  *
	 *------------------*/
	l_txVerifyBDRParamMap.clear();

	BT_SETTING_STRUCT setting;

	l_txVerifyBDRParam.ANALYZE_POWER_ONLY = 0;
	setting.type = BT_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyBDRParam.ANALYZE_POWER_ONLY))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyBDRParam.ANALYZE_POWER_ONLY;
		setting.unit        = "";
		setting.helpText    = "The index to indicate ANALYZE_POWER_ONLY, default=0, 0: OFF, 1: ON ";
		l_txVerifyBDRParamMap.insert( pair<string,BT_SETTING_STRUCT>("ANALYZE_POWER_ONLY", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRParam.TRANSMIT_0XF0_SEQUENCE_FOR_DELTA_F1_AVG = 1;
	setting.type = BT_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyBDRParam.TRANSMIT_0XF0_SEQUENCE_FOR_DELTA_F1_AVG))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyBDRParam.TRANSMIT_0XF0_SEQUENCE_FOR_DELTA_F1_AVG;
		setting.unit        = "";
		setting.helpText    = "The index to transmit 11110000 sequence for delta_f1_avg, default=1, 0: OFF, 1: ON ";
		l_txVerifyBDRParamMap.insert( pair<string,BT_SETTING_STRUCT>("TRANSMIT_0XF0_SEQUENCE_FOR_DELTA_F1_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRParam.FREQ_MHZ = 2402;
	setting.type = BT_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyBDRParam.FREQ_MHZ))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyBDRParam.FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "Channel center frequency in MHz";
		l_txVerifyBDRParamMap.insert( pair<string,BT_SETTING_STRUCT>("FREQ_MHZ", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	//{0x00, "NULL"},
	//{0x01, "POLL"},
	//{0x02, "FHS"},
	//{0x03, "DM1"},
	//{0x04, "DH1 / 2DH1"},
	//{0x05, "HV1"},
	//{0x06, "HV2 / 2-EV3"},
	//{0x07, "HV3 / EV3 / 3-EV3"},
	//{0x08, "DV / 3DH1"},
	//{0x09, "AUX1 / PS"},
	//{0x0A, "DM3 / 2DH3"},
	//{0x0B, "DH3 / 3DH3"},
	//{0x0C, "EV4 / 2-EV5"},
	//{0x0D, "EV5 / 3-EV5"},
	//{0x0E, "DM5 / 2DH5"},
	//{0x0F, "DH5 / 3DH5"}

	strcpy_s(l_txVerifyBDRParam.PACKET_TYPE, MAX_BUFFER_SIZE, "1DH1");
	setting.type = BT_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyBDRParam.PACKET_TYPE))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyBDRParam.PACKET_TYPE;
		setting.unit        = "";
		setting.helpText    = "Sets the packet type, the type can be 1DH1, 1DH3, 1DH5. default is 1DH1";
		l_txVerifyBDRParamMap.insert( pair<string,BT_SETTING_STRUCT>("PACKET_TYPE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRParam.PACKET_LENGTH = 0;
	setting.type = BT_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyBDRParam.PACKET_LENGTH))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyBDRParam.PACKET_LENGTH;
		setting.unit        = "";
		setting.helpText    = "The number of packet to verify BDR.";
		l_txVerifyBDRParamMap.insert( pair<string,BT_SETTING_STRUCT>("PACKET_LENGTH", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRParam.TX_POWER_LEVEL = 0;
	setting.type = BT_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyBDRParam.TX_POWER_LEVEL))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRParam.TX_POWER_LEVEL;
		setting.unit        = "Level";
		setting.helpText    = "Expected power level at DUT antenna port. Level can be 0, 1, 2, 3, 4, 5, 6 and 7 (MaxPower => MinPower, if supported.)";
		l_txVerifyBDRParamMap.insert( pair<string,BT_SETTING_STRUCT>("TX_POWER_LEVEL", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRParam.EXPECTED_TX_POWER_DBM = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRParam.EXPECTED_TX_POWER_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRParam.EXPECTED_TX_POWER_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected TX power dBm at TX_POWER_LEVEL. Default is NA_NUMBER.  If set to non NA_NUMBER, the value will override default power dBm setting.";
		l_txVerifyBDRParamMap.insert( pair<string,BT_SETTING_STRUCT>("EXPECTED_TX_POWER_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRParam.CABLE_LOSS_DB = 0.0;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRParam.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyBDRParam.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester.";
		l_txVerifyBDRParamMap.insert( pair<string,BT_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRParam.SAMPLING_TIME_US = 0;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRParam.SAMPLING_TIME_US))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRParam.SAMPLING_TIME_US;
		setting.unit        = "us";
		setting.helpText    = "Capture time in micro-seconds";
		l_txVerifyBDRParamMap.insert( pair<string,BT_SETTING_STRUCT>("SAMPLING_TIME_US", setting) );
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
	l_txVerifyBDRReturnMap.clear();

	// <Perform LP_AnalyzePower>
	l_txVerifyBDRReturn.POWER_AVERAGE_DBM = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.POWER_AVERAGE_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.POWER_AVERAGE_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Average power in dBm.";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("POWER_AVERAGE_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRReturn.POWER_PEAK_DBM = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.POWER_PEAK_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.POWER_PEAK_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Peak power in dBm.";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("POWER_PEAK_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRReturn.TARGET_POWER_DBM = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.TARGET_POWER_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.TARGET_POWER_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected target power dBm at DUT antenna port.";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("TARGET_POWER_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// <Perform LP_AnalyzeBluetooth>
	l_txVerifyBDRReturn.DATA_RATE_DETECT = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.DATA_RATE_DETECT))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.DATA_RATE_DETECT;
		setting.unit        = "Mbps";
		setting.helpText    = "Bluetooth datarate, can be 1, 2 or 3 Mbps.";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("DATA_RATE_DETECT", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRReturn.BANDWIDTH_20DB = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.BANDWIDTH_20DB))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.BANDWIDTH_20DB;
		setting.unit        = "kHz";
		setting.helpText    = "20 dB bandwidth value kHz";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("BANDWIDTH_20DB", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRReturn.DELTA_F1_AVERAGE = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.DELTA_F1_AVERAGE))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.DELTA_F1_AVERAGE;
		setting.unit        = "kHz";
		setting.helpText    = "The measurement result for deltaF1Avg as specified in BLUETOOTH TEST SPECIFICATION Ver. 1.2/2.0/2.0 + EDR [vol 2] version 2.0.E.2. Requires 00001111 data pattern. Result in kHz.";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("DELTA_F1_AVERAGE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRReturn.DELTA_F2_MAX = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.DELTA_F2_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.DELTA_F2_MAX;
		setting.unit        = "kHz";
		setting.helpText    = "The measurement result for deltaF2Max as specified in BLUETOOTH TEST SPECIFICATION Ver. 1.2/2.0/2.0 + EDR [vol 2] version 2.0.E.2. Requires alternating data pattern. Result in kHz.";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("DELTA_F2_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRReturn.DELTA_F2_AVERAGE = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.DELTA_F2_AVERAGE))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.DELTA_F2_AVERAGE;
		setting.unit        = "kHz";
		setting.helpText    = "The measurement result for deltaF2Avg as specified in BLUETOOTH TEST SPECIFICATION Ver. 1.2/2.0/2.0 + EDR [vol 2] version 2.0.E.2. Requires alternating data pattern. Result in kHz.";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("DELTA_F2_AVERAGE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRReturn.DELTA_F2_MAX_ACCESS = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.DELTA_F2_MAX_ACCESS))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.DELTA_F2_MAX_ACCESS;
		setting.unit        = "kHz";
		setting.helpText    = "Similar to the measurement result for deltaF2Max as specified in BLUETOOTH TEST SPECIFICATION Ver. 1.2/2.0/2.0 + EDR [vol 2] version 2.0.E.2. Result measured from Access data. Result in kHz.";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("DELTA_F2_MAX_ACCESS", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRReturn.DELTA_F2_AV_ACCESS = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.DELTA_F2_AV_ACCESS))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.DELTA_F2_AV_ACCESS;
		setting.unit        = "kHz";
		setting.helpText    = "Similar to the measurement result for deltaF2Avg as specified in BLUETOOTH TEST SPECIFICATION Ver. 1.2/2.0/2.0 + EDR [vol 2] version 2.0.E.2. Result measured from Access data. Result in kHz.";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("DELTA_F2_AV_ACCESS", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRReturn.DELTA_F2_F1_AV_RATIO = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.DELTA_F2_F1_AV_RATIO))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.DELTA_F2_F1_AV_RATIO;
		setting.unit        = "";
		setting.helpText    = "The measurement result for deltaF2Avg/deltaF1Avg, typically the ratio should be >= 0.8";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("DELTA_F2_F1_AV_RATIO", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRReturn.FREQ_DEVIATION = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.FREQ_DEVIATION))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.FREQ_DEVIATION;
		setting.unit        = "kHz";
		setting.helpText    = "Similar to the measurement result for deltaF1Avg as specified in BLUETOOTH TEST SPECIFICATION Ver. 1.2/2.0/2.0 + EDR [vol 2] version 2.0.E.2. Result measured from Header data. Result in kHz.";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("FREQ_DEVIATION", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRReturn.FREQ_DEVIATION_PK_TO_PK = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.FREQ_DEVIATION_PK_TO_PK))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.FREQ_DEVIATION_PK_TO_PK;
		setting.unit        = "kHz";
		setting.helpText    = "Peak to Peak Frequency Deviation, in kHz during header";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("FREQ_DEVIATION_PK_TO_PK", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRReturn.FREQ_EST = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.FREQ_EST))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.FREQ_EST;
		setting.unit        = "kHz";
		setting.helpText    = "Initial freq offset of each burst detected, in kHz.";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("FREQ_EST", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRReturn.FREQ_EST_HEADER = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.FREQ_EST_HEADER))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.FREQ_EST_HEADER;
		setting.unit        = "kHz";
		setting.helpText    = "Estimates the Frequency Offset during the Header in kHz.";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("FREQ_EST_HEADER", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRReturn.FREQ_DRIFT = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.FREQ_DRIFT))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.FREQ_DRIFT;
		setting.unit        = "kHz";
		setting.helpText    = "Initial freq carrier drift of each burst detected, in kHz.";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("FREQ_DRIFT", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRReturn.MAX_FREQ_DRIFT_RATE = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.MAX_FREQ_DRIFT_RATE))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.MAX_FREQ_DRIFT_RATE;
		setting.unit        = "kHz";
		setting.helpText    = "1 Mbps only, takes the maximum drift over specified time interval";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("MAX_FREQ_DRIFT_RATE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyBDRReturn.PAYLOAD_ERRORS = (int)NA_NUMBER;
	setting.type = BT_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyBDRReturn.PAYLOAD_ERRORS))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyBDRReturn.PAYLOAD_ERRORS;
		setting.unit        = "error";
		setting.helpText    = "Reports the number of data errors in Payload. Not counting CRC. If -1, this value has not been calculated. If larger negative number, it reports the length of the payload data vector. This happens when the length of the payload vector is shorter than the length indicated in the payload header.";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("PAYLOAD_ERRORS", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<BT_ACP_SECTION;i++)
	{
		l_txVerifyBDRReturn.MAX_POWER_ACP_DBM[i] = NA_NUMBER;
		setting.type = BT_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyBDRReturn.MAX_POWER_ACP_DBM[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyBDRReturn.MAX_POWER_ACP_DBM[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "ACP_MAX_POWER_DBM_OFFSET_%d", i-5);
			setting.unit        = "dBm";
			setting.helpText    = "Reports max power in 1 MHz bands at specific offsets from center frequency. The offset in MHz is given in sequenceDefinition. Method according to 5.1.8 TRM/CA/06/C";
			l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_txVerifyBDRReturn.CABLE_LOSS_DB = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyBDRReturn.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyBDRReturn.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// Error Message Return String
	l_txVerifyBDRReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = BT_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyBDRReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyBDRReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_txVerifyBDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	return 0;
}

