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
map<string, BT_SETTING_STRUCT> l_txVerifyEDRParamMap;

// Return Value Container
map<string, BT_SETTING_STRUCT> l_txVerifyEDRReturnMap;

struct tagParam
{
    // Mandatory Parameters
	int       FREQ_MHZ;								/*! The center frequency (MHz). */
	char      PACKET_TYPE[MAX_BUFFER_SIZE];			/*! The packet type to verify EDR. */
	int		  PACKET_LENGTH;						/*! The number of packets to verify EDR. */
	int       TX_POWER_LEVEL;						/*! The output power to verify EDR. */
    double    EXPECTED_TX_POWER_DBM;                /*! The expected TX power dBm at TX_POWER_LEVEL. */
	double    CABLE_LOSS_DB;						/*! The path loss of test system. */
	double    SAMPLING_TIME_US;						/*! The sampling time to verify EDR. */ 

} l_txVerifyEDRParam;

struct tagReturn
{   
	double   POWER_AVERAGE_DBM;
	double   POWER_PEAK_DBM;
	double   TARGET_POWER_DBM;
	// <Perform LP_AnalyzeBluetooth>
	double   DATA_RATE_DETECT;
	double   FREQ_DEVIATION;
	double   FREQ_DEVIATION_PK_TO_PK;
	double   FREQ_EST;
	double   FREQ_EST_HEADER;
	double   EDR_EVM_AV;
	double   EDR_EVM_PK;
	double   EDR_POW_DIFF_DB;
	//double   EDR_FREQ_EXTREME_EDR_ONLY;
	double   EDR_PROB_EVM_99_PASS;
	//double   EDR_EVM_VS_TIME;
	double   EDR_OMEGA_I;
	double	 EDR_EXTREME_OMEGA_0;
	double   EDR_EXTREME_OMEGA_I0;
	int	     PAYLOAD_ERRORS;
	double	 MAX_POWER_EDR_DBM[BT_ACP_SECTION];

	double   CABLE_LOSS_DB;						/*! The path loss of the test system. */
    char     ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txVerifyEDRReturn;
#pragma endregion

#ifndef WIN32
int initTXVerifyEDRContainers = InitializeTXVerifyEDRContainers();
#endif

int ClearTxVerifyEDRReturn(void)
{
	l_txVerifyEDRParamMap.clear();
	l_txVerifyEDRReturnMap.clear();
	return 0;
}

//! BT TX Verify EDR
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


BT_TEST_API int BT_TX_Verify_EDR(void)
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
     * Clear Return Parameters and Containers *
     *---------------------------------------*/
	ClearReturnParameters(l_txVerifyEDRReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_txVerifyEDRParamMap);
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
        RespondToQueryReturn(l_txVerifyEDRReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	/*----------------------------------------------------------------------------------------
	 * Declare vectors for storing test results: vector< double >
	 *-----------------------------------------------------------------------------------------*/
	vector< double >		powerAvEachBurst	 (g_BTGlobalSettingParam.TX_EDR_AVERAGE);
	vector< double >		powerPkEachBurst	 (g_BTGlobalSettingParam.TX_EDR_AVERAGE);
	vector< double >		freqDeviation		 (g_BTGlobalSettingParam.TX_EDR_AVERAGE);
	vector< double >		freqDeviationPktoPk	 (g_BTGlobalSettingParam.TX_EDR_AVERAGE);
	vector< double >		freqEst				 (g_BTGlobalSettingParam.TX_EDR_AVERAGE);
	vector< double >		freqEstHeader		 (g_BTGlobalSettingParam.TX_EDR_AVERAGE);
	vector< double >		edrEVMAv			 (g_BTGlobalSettingParam.TX_EDR_AVERAGE);
	vector< double >		edrEVMpk			 (g_BTGlobalSettingParam.TX_EDR_AVERAGE);
	vector< double >		edrPowDiffdB		 (g_BTGlobalSettingParam.TX_EDR_AVERAGE);
	vector< double >		edrFreqExtremeEdronly(g_BTGlobalSettingParam.TX_EDR_AVERAGE);
	vector< double >		edrOmegaI			 (g_BTGlobalSettingParam.TX_EDR_AVERAGE);
	vector< double >		edrExtremeOmega0	 (g_BTGlobalSettingParam.TX_EDR_AVERAGE);
	vector< double >		edrExtremeOmegaI0	 (g_BTGlobalSettingParam.TX_EDR_AVERAGE);
	vector< vector<double> >    maxPowerEdrDbm	 (BT_ACP_SECTION, vector<double>(g_BTGlobalSettingParam.TX_EDR_AVERAGE));	

	l_txVerifyEDRReturn.PAYLOAD_ERRORS = 0;


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
		err = GetInputParameters(l_txVerifyEDRParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Input parameters are not completed.\n");
			throw logMessage;
		}		

		// Check path loss (by ant and freq)
		if ( 0==l_txVerifyEDRParam.CABLE_LOSS_DB )
		{
			err = TM_GetPathLossAtFrequency(g_BT_Test_ID, l_txVerifyEDRParam.FREQ_MHZ, &l_txVerifyEDRParam.CABLE_LOSS_DB, 0, TX_TABLE);
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
		// Set DUT RF frequency, tx power, data rate
		// And clear vDUT parameters at the beginning.
		vDUT_ClearParameters(g_BT_Dut);

		vDUT_AddIntegerParameter(g_BT_Dut, "FREQ_MHZ",			l_txVerifyEDRParam.FREQ_MHZ);	
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
		vDUT_AddIntegerParameter(g_BT_Dut, "LOGIC_CHANNEL",     0);	//{0x00, "ACL  EDR"}
		vDUT_AddStringParameter (g_BT_Dut, "PACKET_TYPE",	    l_txVerifyEDRParam.PACKET_TYPE);
		// Check packet length 		
		if (0==l_txVerifyEDRParam.PACKET_LENGTH)
		{
			GetPacketLength("BER", "PACKETS_LENGTH", l_txVerifyEDRParam.PACKET_TYPE, &packetLength);
		}
		else	
		{
			packetLength = l_txVerifyEDRParam.PACKET_LENGTH;
		}
		vDUT_AddIntegerParameter(g_BT_Dut, "PACKET_LENGTH",     packetLength);
		vDUT_AddIntegerParameter(g_BT_Dut, "TX_POWER_LEVEL",	l_txVerifyEDRParam.TX_POWER_LEVEL);
        vDUT_AddDoubleParameter(g_BT_Dut, "EXPECTED_TX_POWER_DBM", l_txVerifyEDRParam.EXPECTED_TX_POWER_DBM);

		err = vDUT_Run(g_BT_Dut, "QUERY_POWER_DBM");
		if ( ERR_OK!=err )
		{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer			
			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
			{
				err = -1;	// indicates an error has occurred
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else	// Return error message
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(QUERY_POWER_DBM) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			// do nothing
		}

		err = vDUT_GetDoubleReturn(g_BT_Dut, "POWER_DBM", &l_txVerifyEDRReturn.TARGET_POWER_DBM);
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
				err = -1;	// set err to -1, means "Error"
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

#pragma region Setup LP Tester and Capture
		/*--------------------*
		 * Setup IQtester VSA *
		 *--------------------*/
		double peakToAvgRatio = g_BTGlobalSettingParam.IQ_P_TO_A_EDR;
		cableLossDb = l_txVerifyEDRParam.CABLE_LOSS_DB;

		err = LP_SetVsaBluetooth(  l_txVerifyEDRParam.FREQ_MHZ*1e6,
								   l_txVerifyEDRReturn.TARGET_POWER_DBM-cableLossDb+peakToAvgRatio,
								   g_BTGlobalSettingParam.VSA_PORT,
								   g_BTGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
								   g_BTGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000
								 );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to setup VSA.\n");
			throw logMessage;
		}		

#pragma endregion

		// Check Capture Time 
		if (0==l_txVerifyEDRParam.SAMPLING_TIME_US)
		{
			samplingTimeUs = PacketTypeToSamplingTimeUs(l_txVerifyEDRParam.PACKET_TYPE);
		}
		else	// SAMPLING_TIME_US != 0
		{
			samplingTimeUs = l_txVerifyEDRParam.SAMPLING_TIME_US;
		}

		/*--------------------------------*
		 * Start "while" loop for average *
		 *--------------------------------*/
		avgIteration = 0;
		while ( avgIteration<g_BTGlobalSettingParam.TX_EDR_AVERAGE )
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to capture signal at %d MHz.\n", l_txVerifyEDRParam.FREQ_MHZ);
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] LP_VsaDataCapture() at %d MHz return OK.\n", l_txVerifyEDRParam.FREQ_MHZ);
			}	

			/*--------------*
			 *  Capture OK  *
			 *--------------*/
			captureOK = true;
			if (1==g_BTGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
			{
				// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON			
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_EDR_PRBS9_SaveAlways", l_txVerifyEDRParam.FREQ_MHZ, l_txVerifyEDRParam.PACKET_TYPE);
				BTSaveSigFile(sigFileNameBuffer);
			}
			else
			{
				// do nothing
			}

			/*------------------------------*
			 *  Perform Bluetooth analysis  *
			 *------------------------------*/
			err = LP_AnalyzeBluetooth( 0, "AllPlus" );		// 0 (auto), or 1, 2, 3
			if (ERR_OK!=err)
			{	// Fail Analysis, thus save capture (Signal File) for debug
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_EDR_AnalysisFailed", l_txVerifyEDRParam.FREQ_MHZ, l_txVerifyEDRParam.PACKET_TYPE);
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

			if ( (1==LP_GetScalarMeasurement("EdrEVMvalid",0))&&(1==LP_GetScalarMeasurement("valid",0)) )
			{
				// powerAvEachBurst  	
				powerAvEachBurst[avgIteration] = LP_GetScalarMeasurement("P_av_each_burst", 0);
				powerAvEachBurst[avgIteration] = 10 * log10(powerAvEachBurst[avgIteration]);
				if ( -99.00 >= powerAvEachBurst[avgIteration] )
				{
					analysisOK = false;
					l_txVerifyEDRReturn.POWER_AVERAGE_DBM = NA_NUMBER;
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_EDR_AnalysisFailed", l_txVerifyEDRParam.FREQ_MHZ, l_txVerifyEDRParam.PACKET_TYPE);
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
					l_txVerifyEDRReturn.POWER_PEAK_DBM = NA_NUMBER;
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_EDR_AnalysisFailed", l_txVerifyEDRParam.FREQ_MHZ, l_txVerifyEDRParam.PACKET_TYPE);
					BTSaveSigFile(sigFileNameBuffer);
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(P_pk_each_burst) return error.\n");
					throw logMessage;
				}
				else
				{
					powerPkEachBurst[avgIteration] = powerPkEachBurst[avgIteration] + cableLossDb;
				}

				l_txVerifyEDRReturn.DATA_RATE_DETECT	= LP_GetScalarMeasurement("dataRateDetect",0);
				freqDeviation[avgIteration]				= LP_GetScalarMeasurement("freq_deviation",0) / 1000;
				freqDeviationPktoPk[avgIteration]		= LP_GetScalarMeasurement("freq_deviationpktopk",0) / 1000;
				freqEst[avgIteration]					= LP_GetScalarMeasurement("freq_est",0) / 1000;
				freqEstHeader[avgIteration]				= LP_GetScalarMeasurement("freq_estHeader",0) / 1000;
				l_txVerifyEDRReturn.PAYLOAD_ERRORS	   += (int)LP_GetScalarMeasurement("payloadErrors",0);

				edrEVMAv[avgIteration]					 = LP_GetScalarMeasurement("EdrEVMAv",0);
				edrEVMpk[avgIteration]					 = LP_GetScalarMeasurement("EdrEVMpk",0);		
				edrPowDiffdB[avgIteration]				 = LP_GetScalarMeasurement("EdrPowDiffdB",0);
				edrFreqExtremeEdronly[avgIteration]		 = LP_GetScalarMeasurement("EdrFreqExtremeEdronly",0);
				l_txVerifyEDRReturn.EDR_PROB_EVM_99_PASS = LP_GetScalarMeasurement("EdrprobEVM99pass",0)*100;

				edrOmegaI[avgIteration]			= LP_GetScalarMeasurement("EdrOmegaI",0) / 1000;
				edrExtremeOmega0[avgIteration]	= LP_GetScalarMeasurement("EdrExtremeOmega0",0) / 1000;
				edrExtremeOmegaI0[avgIteration] = LP_GetScalarMeasurement("EdrExtremeOmegaI0",0) / 1000;

				if ( 1==LP_GetScalarMeasurement("acpErrValid",0) )
				{
					for (int i=0;i<=10;i++)
					{
						// maxPowerEdrDbm  	
						maxPowerEdrDbm[i][avgIteration]			 = LP_GetScalarMeasurement("maxPowerEdrDbm", i) + cableLossDb;	
					}
				}
				else
				{
					analysisOK = false;
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_EDR_ResultsFailed", l_txVerifyEDRParam.FREQ_MHZ, l_txVerifyEDRParam.PACKET_TYPE);
					BTSaveSigFile(sigFileNameBuffer);
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "BT TX EDR (ACP) retrieve analysis results not valid.\n");
					throw logMessage;
				}

			}
			else
			{
				analysisOK = false;
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_EDR_ResultsFailed", l_txVerifyEDRParam.FREQ_MHZ, l_txVerifyEDRParam.PACKET_TYPE);
				BTSaveSigFile(sigFileNameBuffer);
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "BT TX EDR retrieve analysis results not valid.\n");
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
			err = ::AverageTestResult(&powerAvEachBurst[0], avgIteration, LOG_10, l_txVerifyEDRReturn.POWER_AVERAGE_DBM, dummyMax, dummyMin);
			// Peak Power test result
			err = ::AverageTestResult(&powerPkEachBurst[0], avgIteration, LOG_10, dummyMax, l_txVerifyEDRReturn.POWER_PEAK_DBM, dummyMin);
			err = ::AverageTestResult(&freqDeviation[0], avgIteration, Linear, l_txVerifyEDRReturn.FREQ_DEVIATION, dummyMax, dummyMin);
			err = ::AverageTestResult(&freqDeviationPktoPk[0], avgIteration, Linear, l_txVerifyEDRReturn.FREQ_DEVIATION_PK_TO_PK, dummyMax, dummyMin);
			err = ::AverageTestResult(&freqEst[0], avgIteration, Linear, l_txVerifyEDRReturn.FREQ_EST, dummyMax, dummyMin);
			err = ::AverageTestResult(&freqEstHeader[0], avgIteration, Linear, l_txVerifyEDRReturn.FREQ_EST_HEADER, dummyMax, dummyMin);
			err = ::AverageTestResult(&edrEVMAv[0], avgIteration, LOG_20, l_txVerifyEDRReturn.EDR_EVM_AV, dummyMax, dummyMin);
			err = ::AverageTestResult(&edrEVMpk[0], avgIteration, LOG_20, dummyMax, l_txVerifyEDRReturn.EDR_EVM_PK, dummyMin);
			err = ::AverageTestResult(&edrPowDiffdB[0], avgIteration, LOG_10, l_txVerifyEDRReturn.EDR_POW_DIFF_DB, dummyMax, dummyMin);
			err = ::AverageTestResult(&edrOmegaI[0], avgIteration, Linear, l_txVerifyEDRReturn.EDR_OMEGA_I, dummyMax, dummyMin);
			err = ::AverageTestResult(&edrExtremeOmega0[0], avgIteration, Linear, l_txVerifyEDRReturn.EDR_EXTREME_OMEGA_0, dummyMax, dummyMin);
			err = ::AverageTestResult(&edrExtremeOmegaI0[0], avgIteration, Linear, l_txVerifyEDRReturn.EDR_EXTREME_OMEGA_I0, dummyMax, dummyMin);

			// ACP
			for (int i=0;i<=10;i++)
			{
				err = ::AverageTestResult(&maxPowerEdrDbm[i][0], avgIteration, LOG_10, l_txVerifyEDRReturn.MAX_POWER_EDR_DBM[i], dummyMax, dummyMin);
			}

			// By TRM/CA/13/C, the OFFSET_1(index_6) and OFFSET_-1(index_4) must report relative power in dB
			l_txVerifyEDRReturn.MAX_POWER_EDR_DBM[4] = l_txVerifyEDRReturn.MAX_POWER_EDR_DBM[4] - l_txVerifyEDRReturn.MAX_POWER_EDR_DBM[5];
			l_txVerifyEDRReturn.MAX_POWER_EDR_DBM[6] = l_txVerifyEDRReturn.MAX_POWER_EDR_DBM[6] - l_txVerifyEDRReturn.MAX_POWER_EDR_DBM[5];
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
				err = -1;	// indicates an error has occurred
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else	// returns normal error message
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(TX_STOP) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			vDutActived = false;
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err && captureOK && analysisOK )
		{
			// Return Path Loss (dB)
			l_txVerifyEDRReturn.CABLE_LOSS_DB = l_txVerifyEDRParam.CABLE_LOSS_DB;

			sprintf_s(l_txVerifyEDRReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txVerifyEDRReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txVerifyEDRReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_txVerifyEDRReturn.ERROR_MESSAGE, "[BT] Unknown Error!\n");
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
    freqDeviation.clear();
	freqDeviationPktoPk.clear();
	freqEst.clear();
	freqEstHeader.clear();
	edrEVMAv.clear();
    edrEVMpk.clear();
	edrPowDiffdB.clear();
	edrFreqExtremeEdronly.clear();
	edrOmegaI.clear();
	edrExtremeOmega0.clear();
	edrExtremeOmegaI0.clear();
	maxPowerEdrDbm.clear();


    return err;
}

int InitializeTXVerifyEDRContainers(void)
{
    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_txVerifyEDRParamMap.clear();

    BT_SETTING_STRUCT setting;

    l_txVerifyEDRParam.FREQ_MHZ = 2402;
    setting.type = BT_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyEDRParam.FREQ_MHZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEDRParam.FREQ_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Channel center frequency in MHz";
        l_txVerifyEDRParamMap.insert( pair<string,BT_SETTING_STRUCT>("FREQ_MHZ", setting) );
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

    strcpy_s(l_txVerifyEDRParam.PACKET_TYPE, MAX_BUFFER_SIZE, "2DH1");
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEDRParam.PACKET_TYPE))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyEDRParam.PACKET_TYPE;
        setting.unit        = "";
        setting.helpText    = "Sets the packet type, the type can be 2DH1, 2DH3, 2DH5, 3DH1, 3DH3, 3DH5. default is 2DH1";
        l_txVerifyEDRParamMap.insert( pair<string,BT_SETTING_STRUCT>("PACKET_TYPE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyEDRParam.PACKET_LENGTH = 0;
    setting.type = BT_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyEDRParam.PACKET_LENGTH))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEDRParam.PACKET_LENGTH;
        setting.unit        = "";
        setting.helpText    = "The number of packet to verify EDR.";
        l_txVerifyEDRParamMap.insert( pair<string,BT_SETTING_STRUCT>("PACKET_LENGTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyEDRParam.TX_POWER_LEVEL = 0;
    setting.type = BT_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyEDRParam.TX_POWER_LEVEL))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRParam.TX_POWER_LEVEL;
        setting.unit        = "Level";
        setting.helpText    = "Expected power level at DUT antenna port. Level can be 0, 1, 2, 3, 4, 5, 6 and 7 (MaxPower => MinPower, if supported.)";
        l_txVerifyEDRParamMap.insert( pair<string,BT_SETTING_STRUCT>("TX_POWER_LEVEL", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEDRParam.EXPECTED_TX_POWER_DBM = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEDRParam.EXPECTED_TX_POWER_DBM))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRParam.EXPECTED_TX_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Expected TX power dBm at TX_POWER_LEVEL. Default is NA_NUMBER.  If set to non NA_NUMBER, the value will override default power dBm setting.";
        l_txVerifyEDRParamMap.insert( pair<string,BT_SETTING_STRUCT>("EXPECTED_TX_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyEDRParam.CABLE_LOSS_DB = 0.0;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEDRParam.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEDRParam.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_txVerifyEDRParamMap.insert( pair<string,BT_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_txVerifyEDRParam.SAMPLING_TIME_US = 0;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEDRParam.SAMPLING_TIME_US))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRParam.SAMPLING_TIME_US;
        setting.unit        = "us";
        setting.helpText    = "Capture time in micro-seconds";
        l_txVerifyEDRParamMap.insert( pair<string,BT_SETTING_STRUCT>("SAMPLING_TIME_US", setting) );
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
    l_txVerifyEDRReturnMap.clear();

    l_txVerifyEDRReturn.POWER_AVERAGE_DBM = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEDRReturn.POWER_AVERAGE_DBM))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRReturn.POWER_AVERAGE_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Average power in dBm.";
        l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("POWER_AVERAGE_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEDRReturn.POWER_PEAK_DBM = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEDRReturn.POWER_PEAK_DBM))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRReturn.POWER_PEAK_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Peak power in dBm.";
        l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("POWER_PEAK_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEDRReturn.TARGET_POWER_DBM = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEDRReturn.TARGET_POWER_DBM))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRReturn.TARGET_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Expected target power dBm at DUT antenna port.";
        l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("TARGET_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	// <Perform LP_AnalyzeBluetooth>
    l_txVerifyEDRReturn.DATA_RATE_DETECT = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEDRReturn.DATA_RATE_DETECT))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRReturn.DATA_RATE_DETECT;
        setting.unit        = "Mbps";
        setting.helpText    = "Bluetooth datarate, can be 1, 2 or 3 Mbps.";
        l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("DATA_RATE_DETECT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyEDRReturn.EDR_EVM_AV = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEDRReturn.EDR_EVM_AV))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRReturn.EDR_EVM_AV;
        setting.unit        = "";
        setting.helpText    = "RMS Differential EVM value (EDR only).";
        l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("EDR_EVM_AV", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyEDRReturn.EDR_EVM_PK = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEDRReturn.EDR_EVM_PK))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRReturn.EDR_EVM_PK;
        setting.unit        = "";
        setting.helpText    = "Peak Differential EVM value (EDR only).";
        l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("EDR_EVM_PK", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyEDRReturn.EDR_POW_DIFF_DB = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEDRReturn.EDR_POW_DIFF_DB))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRReturn.EDR_POW_DIFF_DB;
        setting.unit        = "dB";
        setting.helpText    = "Relative power of EDR section to FM section of packet, in dB.";
        l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("EDR_POW_DIFF_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyEDRReturn.EDR_PROB_EVM_99_PASS = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEDRReturn.EDR_PROB_EVM_99_PASS))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRReturn.EDR_PROB_EVM_99_PASS;
        setting.unit        = "%";
        setting.helpText    = "The percentage of symbols with EVM below the threshold. Threshold for 2 Mbps is 0.3 for 3 Mbps is 0.2.";
        l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("EDR_PROB_EVM_99_PASS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyEDRReturn.FREQ_DEVIATION = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEDRReturn.FREQ_DEVIATION))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRReturn.FREQ_DEVIATION;
        setting.unit        = "kHz";
        setting.helpText    = "Similar to the measurement result for deltaF1Avg as specified in BLUETOOTH TEST SPECIFICATION Ver. 1.2/2.0/2.0 + EDR [vol 2] version 2.0.E.2. Result measured from Header data. Result in kHz.";
        l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("FREQ_DEVIATION", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyEDRReturn.FREQ_DEVIATION_PK_TO_PK = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEDRReturn.FREQ_DEVIATION_PK_TO_PK))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRReturn.FREQ_DEVIATION_PK_TO_PK;
        setting.unit        = "kHz";
        setting.helpText    = "Peak to Peak Frequency Deviation, in kHz during header";
        l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("FREQ_DEVIATION_PK_TO_PK", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyEDRReturn.FREQ_EST = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEDRReturn.FREQ_EST))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRReturn.FREQ_EST;
        setting.unit        = "kHz";
        setting.helpText    = "Initial freq offset of each burst detected, in kHz.";
        l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("FREQ_EST", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyEDRReturn.FREQ_EST_HEADER = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEDRReturn.FREQ_EST_HEADER))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRReturn.FREQ_EST_HEADER;
        setting.unit        = "kHz";
        setting.helpText    = "Estimates the Frequency Offset during the Header in kHz.";
        l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("FREQ_EST_HEADER", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyEDRReturn.EDR_OMEGA_I = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEDRReturn.EDR_OMEGA_I))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRReturn.EDR_OMEGA_I;
        setting.unit        = "kHz";
        setting.helpText    = "Estimates the Omega_i during the Header in kHz.";
        l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("EDR_OMEGA_I", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyEDRReturn.EDR_EXTREME_OMEGA_0 = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEDRReturn.EDR_EXTREME_OMEGA_0))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRReturn.EDR_EXTREME_OMEGA_0;
        setting.unit        = "kHz";
        setting.helpText    = "Extreme value of Omega_0, same as EdrFreqExtremeEdronly.";
        l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("EDR_EXTREME_OMEGA_0", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyEDRReturn.EDR_EXTREME_OMEGA_I0 = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEDRReturn.EDR_EXTREME_OMEGA_I0))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRReturn.EDR_EXTREME_OMEGA_I0;
        setting.unit        = "kHz";
        setting.helpText    = "Extreme value of (Omega_0 + Omega_i).";
        l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("EDR_EXTREME_OMEGA_I0", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyEDRReturn.PAYLOAD_ERRORS = (int)NA_NUMBER;
    setting.type = BT_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyEDRReturn.PAYLOAD_ERRORS))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEDRReturn.PAYLOAD_ERRORS;
        setting.unit        = "error";
        setting.helpText    = "Reports the number of data errors in Payload. Not counting CRC. If -1, this value has not been calculated. If larger negative number, it reports the length of the payload data vector. This happens when the length of the payload vector is shorter than the length indicated in the payload header.";
        l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("PAYLOAD_ERRORS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    for (int i=0;i<BT_ACP_SECTION;i++)
    {
		if ( (i==4)||(i==6) )	// By TRM/CA/13/C, the OFFSET_1(index_6) and OFFSET_-1(index_4) must report relative power in dB 
		{
			l_txVerifyEDRReturn.MAX_POWER_EDR_DBM[i] = NA_NUMBER;
			setting.type = BT_SETTING_TYPE_DOUBLE;
			if (sizeof(double)==sizeof(l_txVerifyEDRReturn.MAX_POWER_EDR_DBM[i]))    // Type_Checking
			{
				setting.value       = (void*)&l_txVerifyEDRReturn.MAX_POWER_EDR_DBM[i];
				char tempStr[MAX_BUFFER_SIZE];
				sprintf_s(tempStr, "ACP_MAX_POWER_DB_OFFSET_%d", i-5);
				setting.unit        = "dB";
				setting.helpText    = "Reports max power in 1 MHz bands at specific offsets from center frequency. The power at 0 MHz offset and +/-1 MHz offset is calculated differently from above. maxPowerEDRdBm follows 5.1.15 TRM/CA/13/C";
				l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>(tempStr, setting) );
			}
			else    
			{
				printf("Parameter Type Error!\n");
				exit(1);
			}
		}
		else
		{
			l_txVerifyEDRReturn.MAX_POWER_EDR_DBM[i] = NA_NUMBER;
			setting.type = BT_SETTING_TYPE_DOUBLE;
			if (sizeof(double)==sizeof(l_txVerifyEDRReturn.MAX_POWER_EDR_DBM[i]))    // Type_Checking
			{
				setting.value       = (void*)&l_txVerifyEDRReturn.MAX_POWER_EDR_DBM[i];
				char tempStr[MAX_BUFFER_SIZE];
				sprintf_s(tempStr, "ACP_MAX_POWER_DBM_OFFSET_%d", i-5);
				setting.unit        = "dBm";
				setting.helpText    = "Reports max power in 1 MHz bands at specific offsets from center frequency. The power at 0 MHz offset and +/-1 MHz offset is calculated differently from above. maxPowerEDRdBm follows 5.1.15 TRM/CA/13/C";
				l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>(tempStr, setting) );
			}
			else    
			{
				printf("Parameter Type Error!\n");
				exit(1);
			}
		}
	}

	l_txVerifyEDRReturn.CABLE_LOSS_DB = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEDRReturn.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEDRReturn.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
    
	// Error Message Return String
    l_txVerifyEDRReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEDRReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyEDRReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
       l_txVerifyEDRReturnMap.insert( pair<string,BT_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}

