#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_Test.h"
#include "WiFi_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include <math.h> // fmod on mac

using namespace std;


#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_txVerifyEvmParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_txVerifyEvmReturnMap;

struct tagParam
{
	// Mandatory Parameters
	int    FREQ_MHZ;                                /*! The center frequency (MHz). */
	char   BANDWIDTH[MAX_BUFFER_SIZE];              /*! The RF bandwidth to verify EVM. */
	char   DATA_RATE[MAX_BUFFER_SIZE];              /*! The data rate to verify EVM. */
	char   PREAMBLE[MAX_BUFFER_SIZE];               /*! The preamble type of 11B(only). */
	char   PACKET_FORMAT_11N[MAX_BUFFER_SIZE];      /*! The packet format of 11N(only). */
	char   GUARD_INTERVAL_11N[MAX_BUFFER_SIZE];     /*! The guard interval format of 11N(only). */
	double TX_POWER_DBM;                            /*! The output power to verify EVM. */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
	double SAMPLING_TIME_US;                        /*! The sampling time to verify EVM. */

	// DUT Parameters
	int    TX1;                                     /*!< DUT TX1 on/off. Default=1(on) */
	int    TX2;                                     /*!< DUT TX2 on/off. Default=0(off) */
	int    TX3;                                     /*!< DUT TX3 on/off. Default=0(off) */
	int    TX4;                                     /*!< DUT TX4 on/off. Default=0(off) */
} l_txVerifyEvmParam;

struct tagReturn
{
	double TX_POWER_DBM;							   /*!< TX Power dBm Setting */ // -cfy@sunnyvale, 2012/3/13-

	// EVM Test Result
	double EVM_AVG_DB;
	double EVM_AVG[MAX_DATA_STREAM];                   /*!< (Average) EVM test result on specific data stream. Format: EVM_AVG[SpecificStream] */
	double EVM_MAX[MAX_DATA_STREAM];                   /*!< (Maximum) EVM test result on specific data stream. Format: EVM_MAX[SpecificStream] */
	double EVM_MIN[MAX_DATA_STREAM];                   /*!< (Minimum) EVM test result on specific data stream. Format: EVM_MIN[SpecificStream] */

	//EVM Test Result in percentage;
	double EVM_AVG_PERCENT;
	double EVM_AVG_STREAM_PERCENT[MAX_DATA_STREAM];           /*!< (Average) EVM test result in percentage on specific data stream. Format: EVM_AVG_PERCENT[SpecificStream] */
	double EVM_MAX_STREAM_PERCENT[MAX_DATA_STREAM];           /*!< (Maximum) EVM test result in percentage on specific data stream. Format: EVM_MAX_PERCENT[SpecificStream] */
	double EVM_MIN_STREAM_PERCENT[MAX_DATA_STREAM];           /*!< (Minimum) EVM test result in percentage on specific data stream. Format: EVM_MIN_PERCENT[SpecificStream] */

	// EVM_PK Test Result
	double EVM_PK_DB;							   	   /*!< (11b only)EVM_PK over captured packets. */

	//EVM Test Result in percentage;
	double EVM_PK_PERCENT;							   /*!< (11b only)EVM_PK in percentage over captured packets. */

	// POWER Test Result
	double POWER_AVG_DBM;
	double POWER_AVG[MAX_DATA_STREAM];                 /*!< (Average) POWER test result on specific data stream. Format: POWER_AVG[SpecificStream] */
	double POWER_MAX[MAX_DATA_STREAM];                 /*!< (Maximum) POWER test result on specific data stream. Format: POWER_MAX[SpecificStream] */
	double POWER_MIN[MAX_DATA_STREAM];                 /*!< (Minimum) POWER test result on specific data stream. Format: POWER_MIN[SpecificStream] */

	double FREQ_ERROR_AVG;                             /*!< (Average) Frequency Error, unit is ppm */
	double FREQ_ERROR_MAX;                             /*!< (Maximum) Frequency Error, unit is ppm */
	double FREQ_ERROR_MIN;                             /*!< (Minimum) Frequency Error, unit is ppm */

	double AMP_ERR_DB;								   /*!< IQ Match Amplitude Error in dB. */
	double PHASE_ERR;								   /*!< IQ Match Phase Error. */
	double PHASE_NOISE_RMS_ALL;                        /*!< Frequency RMS Phase Noise. */
	double SYMBOL_CLK_ERR;							   /*!< Symbol clock error*/

	double AMP_ERR_DB_STREAM[MAX_DATA_STREAM];		   /*!< IQ Match Amplitude Error in dB. per stream*/
	double PHASE_ERR_STREAM[MAX_DATA_STREAM];		   /*!< IQ Match Phase Error. per stream*/

	int	   SPATIAL_STREAM;
	double DATA_RATE;
	double CABLE_LOSS_DB[MAX_DATA_STREAM];             /*! The path loss of test system. */

	char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txVerifyEvmReturn;
#pragma endregion

void ClearTxEvmReturn(void)
{
	l_txVerifyEvmParamMap.clear();
	l_txVerifyEvmReturnMap.clear();
}

#ifndef WIN32
int initTxVerifyEvmContainer = InitializeTXVerifyEvmContainers();
#endif

// These global variables/functions only for WiFi_TX_Verify_Evm.cpp
int CheckTxEvmParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *samplingTimeUs, double *cableLossDb, double *peakToAvgRatio, char* errorMsg );
int WiFi_TX_Verify_Evm_2010ExtTest(void);

//! WiFi TX Verify EVM
/*!
 * Input Parameters
 *
 *  - Mandatory
 *      -# FREQ_MHZ (double): The center frequency (MHz)
 *      -# DATA_RATE (string): The data rate to verify EVM
 *      -# TX_POWER_DBM (double): The power (dBm) DUT is going to transmit at the antenna port
 *
 * Return Values
 *      -# A string that contains all DUT info, such as FW version, driver version, chip revision, etc.
 *      -# A string for error message
 *
 * \return 0 No error occurred
 * \return -1 Error(s) occurred.  Please see the returned error message for details
 */


WIFI_TEST_API int WiFi_TX_Verify_Evm(void)
{
	int    err = ERR_OK;

	bool   analysisOK = false, captureOK = false, enableMultiCapture = false;
	int    avgIteration = 0;
	int    channel = 0, HT40ModeOn = 0, packetFormat = 0;
	int    dummyValue = 0;
	int    wifiMode = 0, wifiStreamNum = 0;
	double samplingTimeUs = 0, peakToAvgRatio = 0, cableLossDb = 0;
	double chainGainDb = 0.0;
	char   vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};


	/*---------------------------------------*
	 * Clear Return Parameters and Container *
	 *---------------------------------------*/
	ClearReturnParameters(l_txVerifyEvmReturnMap);

	/*------------------------*
	 * Respond to QUERY_INPUT *
	 *------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_txVerifyEvmParamMap);
		return err;
	}
	else
	{
		// do nothing
	}

	/*-------------------------*
	 * Respond to QUERY_RETURN *
	 *-------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_RETURN", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryReturn(l_txVerifyEvmReturnMap);
		return err;
	}
	else
	{
		// do nothing
	}

	/*----------------------------------------------------------------------------------------
	 * Declare vectors for storing test results: vector< vector<double> >
	 * The outer vector is for TX streams, and the inner vector is for averaging of measurement
	 *-----------------------------------------------------------------------------------------*/
	vector< vector<double> >    evmAvgAll(MAX_DATA_STREAM, vector<double>(g_WiFiGlobalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    rxRmsPowerDb(MAX_DATA_STREAM, vector<double>(g_WiFiGlobalSettingParam.EVM_AVERAGE));
	vector< double >            evmPk(g_WiFiGlobalSettingParam.EVM_AVERAGE);
	vector< double >            freqErrorHz(g_WiFiGlobalSettingParam.EVM_AVERAGE);
	vector< double >            symbolClockErr(g_WiFiGlobalSettingParam.EVM_AVERAGE);
	vector< vector<double> >    ampErrDb(MAX_DATA_STREAM, vector<double>(g_WiFiGlobalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    phaseErr(MAX_DATA_STREAM, vector<double>(g_WiFiGlobalSettingParam.EVM_AVERAGE));
	vector< double >			phaseNoiseRms(g_WiFiGlobalSettingParam.EVM_AVERAGE);

	try
	{
		/*-----------------------------------------------------------*
		 * Both g_WiFi_Test_ID and g_WiFi_Dut need to be valid (>=0) *
		 *-----------------------------------------------------------*/
		if( g_WiFi_Test_ID<0 || g_WiFi_Dut<0 )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] WiFi_Test_ID or WiFi_Dut not valid. WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_Test_ID, g_WiFi_Dut);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_Test_ID, g_WiFi_Dut);
		}

		TM_ClearReturns(g_WiFi_Test_ID);

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_txVerifyEvmParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Get input parameters return OK.\n");
		}
        /*------------------------------------------------------*
		* check 2010Ext test, run 2010 Ext test and return,		*
		* needed to split WIFI_TX_Verify_Evm into two to	* 
		* avoid a stack check issue.							*
											*
		*------------------------------------------------------*/
		if ( g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE )
		{
			return WiFi_TX_Verify_Evm_2010ExtTest();
		}
		// Error return of this function is irrelevant
		CheckDutTransmitStatus();

#pragma region Prepare input parameters

		err = CheckTxEvmParameters( &channel, &wifiMode, &wifiStreamNum, &samplingTimeUs, &cableLossDb, &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters CheckTxEvmParameters() return OK.\n");
		}

		// Modified /* -cfy@sunnyvale, 2012/3/13- */
		// Considering stream number while calculating chain gain for composite measurement so as to set the proper Rx amplitude in tester
		int antennaNum = l_txVerifyEvmParam.TX1 + l_txVerifyEvmParam.TX2 + l_txVerifyEvmParam.TX3 + l_txVerifyEvmParam.TX4;
		if(antennaNum > 0)
		{
			chainGainDb = 10.0 * log10((double)antennaNum / wifiStreamNum);
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Set Chain Gain of %.2f dB.\n", chainGainDb);
		}
		else
		{
			chainGainDb = 0.0;
		}
		/* <><~~ */

		if (0==strcmp(l_txVerifyEvmParam.PACKET_FORMAT_11N, "MIXED"))
		{
			packetFormat = 1;
		}
		else if (0 == strcmp(l_txVerifyEvmParam.PACKET_FORMAT_11N, "GREENFIELD"))
		{
			packetFormat = 2;
		}
		else
		{
			packetFormat = 0;
		}

#pragma endregion

		/*----------------------------*
		 * Disable VSG output signal  *
		 *----------------------------*/
		if (!g_useLpDut)
		{
			// make sure no signal is generated by the VSG
			err = ::LP_EnableVsgRF(0);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
			}
		}
		/*--------------------*
		 * Setup IQTester VSA *
		 *--------------------*/
		err = ::LP_SetVsaAmplitudeTolerance(g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to set VSA amplitude tolerance in dB.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsaAmplitudeTolerance(%.3f) return OK.\n", g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);
		}

#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to transmit *
		 *---------------------------*/
		// Set DUT RF frquency, tx power, antenna, data rate
		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_Dut);

		if( wifiMode==WIFI_11N_HT40 )
		{
			HT40ModeOn = 1;   // 1: HT40 mode;
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_txVerifyEvmParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_txVerifyEvmParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_txVerifyEvmParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",      l_txVerifyEvmParam.FREQ_MHZ);
		}

		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",		  l_txVerifyEvmParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_txVerifyEvmParam.PACKET_FORMAT_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N", l_txVerifyEvmParam.GUARD_INTERVAL_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",		  l_txVerifyEvmParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",		  l_txVerifyEvmParam.BANDWIDTH);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		  HT40ModeOn);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",				  l_txVerifyEvmParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",				  l_txVerifyEvmParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",				  l_txVerifyEvmParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",				  l_txVerifyEvmParam.TX4);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "SAMPLING_TIME_US",  samplingTimeUs); // -cfy@sunnyvale, 2012/3/13-
		// -cfy@sunnyvale, 2012/3/13-
		// Added GET_TX_TARGET_POWER feature for AR6004. When TX_POWER_DBM is set to -99, default target power will be applied.
		if ( (l_txVerifyEvmParam.TX_POWER_DBM == TX_TARGET_POWER_FLAG) )
		{ // Indicate that the Tx target power will be determined by channel and data rate, rather than the value input by users in IQwizard
			if ( g_dutConfigChanged == true )
			{
				/*-----------*
				 *  Tx Stop  *
				 *-----------*/
				err = ::vDUT_Run(g_WiFi_Dut, "TX_STOP");
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
						throw logMessage;
					}
					else	// Just return normal error message in this case
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_STOP) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					g_vDutTxActived = false;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
				}

				/*----------------------*
				 *  Get Tx Target Power  *
				 *-----------------------*/
				double tx_target_power = 0;
				err = ::vDUT_Run(g_WiFi_Dut, "GET_TX_TARGET_POWER");
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
						throw logMessage;
					}
					else	// Just return normal error message in this case
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(GET_TX_TARGET_POWER) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(GET_TX_TARGET_POWER) return OK.\n");
				}
				vDUT_GetDoubleParameter (g_WiFi_Dut, "TX_TARGET_POWER_DBM", &tx_target_power);
				l_txVerifyEvmParam.TX_POWER_DBM = tx_target_power;
				g_last_TxPower_dBm_Record = l_txVerifyEvmParam.TX_POWER_DBM;
			}
			else
			{
				l_txVerifyEvmParam.TX_POWER_DBM = g_last_TxPower_dBm_Record;
			}
		}
		else
		{
			// do nothing
		}
		/* <><~~ */

		vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM",	  l_txVerifyEvmParam.TX_POWER_DBM);
		TM_SetDoubleParameter(g_WiFi_Test_ID, "TX_POWER_DBM", l_txVerifyEvmParam.TX_POWER_DBM); // -cfy@sunnyvale, 2012/3/13-

		if ( (g_dutConfigChanged==true)||(g_vDutTxActived==false) )
		{
			if ( g_vDutTxActived==true )
			{
				/*-----------*
				 *  Tx Stop  *
				 *-----------*/
				err = ::vDUT_Run(g_WiFi_Dut, "TX_STOP");
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
						throw logMessage;
					}
					else	// Just return normal error message in this case
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_STOP) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					g_vDutTxActived = false;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
				}
			}
			else
			{
				// continue Dut configuration
			}

			err = vDUT_Run(g_WiFi_Dut, "RF_SET_FREQ");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RF_SET_FREQ) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RF_SET_FREQ) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_SET_BW");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_SET_BW) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_SET_BW) return OK.\n");
			}
			err = vDUT_Run(g_WiFi_Dut, "TX_SET_DATA_RATE");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_SET_DATA_RATE) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_SET_DATA_RATE) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_SET_ANTENNA");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_SET_ANTENNA) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_SET_ANTENNA) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_PRE_TX");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_PRE_TX) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_PRE_TX) return OK.\n");
			}

			

			err = vDUT_Run(g_WiFi_Dut, "TX_START");
			if ( ERR_OK!=err )
			{
				g_vDutTxActived = false;
				// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_START) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutTxActived = true;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_START) return OK.\n");
			}

			// Delay for DUT settle
			if (0!=g_WiFiGlobalSettingParam.DUT_TX_SETTLE_TIME_MS)
			{
				Sleep(g_WiFiGlobalSettingParam.DUT_TX_SETTLE_TIME_MS);
			}
			else
			{
				// do nothing
			}
		}
		else
		{
			// g_dutConfigChanged==false, do nothing
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Since g_dutConfigChanged = false, skip Dut control section.\n");
		}
#pragma endregion

		
			// NOT "g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE"
		
#pragma region Setup LP Tester and Capture

			err = ::LP_SetVsa(  l_txVerifyEvmParam.FREQ_MHZ*1e6,
					l_txVerifyEvmParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGainDb,
					g_WiFiGlobalSettingParam.VSA_PORT,
					0,
					g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
					g_WiFiGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000
					);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi] Fail to setup VSA, LP_SetVsa() return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsa() return OK.\n");
			}

			/*------------------*
			 * Start While Loop *
			 *------------------*/
			avgIteration = 0;
			while ( avgIteration<g_WiFiGlobalSettingParam.EVM_AVERAGE )
			{
				analysisOK = false;
				captureOK  = false;

				/*----------------------------*
				 * Perform Normal VSA capture *
				 *----------------------------*/
				HT40ModeOn = 0;
				err = ::LP_VsaDataCapture( samplingTimeUs/1000000, g_WiFiGlobalSettingParam.VSA_TRIGGER_TYPE, 80e6, HT40ModeOn );
				if( ERR_OK!=err )	// capture is failed
				{
					double quickPower = NA_NUMBER;
					err = QuickCaptureRetry( l_txVerifyEvmParam.FREQ_MHZ, samplingTimeUs, g_WiFiGlobalSettingParam.VSA_TRIGGER_TYPE, HT40ModeOn, &quickPower, logMessage);
					if (ERR_OK!=err)	// QuickCaptureRetry() is failed
					{
						// Fail Capture
						if ( quickPower!=NA_NUMBER )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to capture WiFi signal at %d MHz.\nThe DUT power (without path loss) = %.1f dBm and QuickCaptureRetry() return error.\n", l_txVerifyEvmParam.FREQ_MHZ, quickPower);
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to capture WiFi signal at %d MHz, QuickCaptureRetry() return error.\n", l_txVerifyEvmParam.FREQ_MHZ);
						}
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] The QuickCaptureRetry() at %d MHz return OK.\n", l_txVerifyEvmParam.FREQ_MHZ);
					}
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_VsaDataCapture() at %d MHz return OK.\n", l_txVerifyEvmParam.FREQ_MHZ);
				}

				/*--------------*
				 *  Capture OK  *
				 *--------------*/
				captureOK = true;
				if (1==g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
				{
					// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_SaveAlways", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
				}
				else
				{
					// do nothing
				}
#pragma endregion

				/*----------------*
				 *  EVM Analysis  *
				 *----------------*/
#pragma region Analysis_802_11b
				if ( wifiMode==WIFI_11B )
				{   // [Case 01]: 802.11b Analysis
					err = ::LP_Analyze80211b(   g_WiFiGlobalSettingParam.ANALYSIS_11B_EQ_TAPS,
							g_WiFiGlobalSettingParam.ANALYSIS_11B_DC_REMOVE_FLAG,
							g_WiFiGlobalSettingParam.ANALYSIS_11B_METHOD_11B
							);
					if (ERR_OK!=err)
					{	// Fail Analysis, thus save capture (Signal File) for debug
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_Analyze80211b_Failed", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
						WiFiSaveSigFile(sigFileNameBuffer);

						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_Analyze80211b() return error.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_Analyze80211b() return OK.\n");
					}
				}
#pragma endregion
#pragma region Analysis_802_11ag
				else if ( wifiMode==WIFI_11AG )
				{   // [Case 02]: 802.11a/g Analysis
					if (0==strcmp(l_txVerifyEvmParam.BANDWIDTH,"HALF") || 0==strcmp(l_txVerifyEvmParam.BANDWIDTH,"QUAR"))
					{
						err = ::LP_Analyze80211p(  g_WiFiGlobalSettingParam.ANALYSIS_11AG_PH_CORR_MODE,
								g_WiFiGlobalSettingParam.ANALYSIS_11AG_CH_ESTIMATE,
								g_WiFiGlobalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR,
								g_WiFiGlobalSettingParam.ANALYSIS_11AG_FREQ_SYNC,
								g_WiFiGlobalSettingParam.ANALYSIS_11AG_AMPL_TRACK,
								(0==strcmp(l_txVerifyEvmParam.BANDWIDTH,"HALF"))?IQV_OFDM_ASTM_DSRC:IQV_OFDM_QUARTER_RATE
								);
						if ( ERR_OK!=err )
						{
							// Fail Analysis, thus save capture (Signal File) for debug
							sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_Analyze80211p_Failed", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
							WiFiSaveSigFile(sigFileNameBuffer);
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_Analyze80211p() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_Analyze80211p() return OK.\n");
						}
					}
					else
					{
						err = ::LP_Analyze80211ag(  g_WiFiGlobalSettingParam.ANALYSIS_11AG_PH_CORR_MODE,
								g_WiFiGlobalSettingParam.ANALYSIS_11AG_CH_ESTIMATE,
								g_WiFiGlobalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR,
								g_WiFiGlobalSettingParam.ANALYSIS_11AG_FREQ_SYNC,
								g_WiFiGlobalSettingParam.ANALYSIS_11AG_AMPL_TRACK
								);
						if ( ERR_OK!=err )
						{
							// Fail Analysis, thus save capture (Signal File) for debug
							sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_Analyze80211ag_Failed", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
							WiFiSaveSigFile(sigFileNameBuffer);
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_Analyze80211ag() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_Analyze80211ag() return OK.\n");
						}
					}
				}
#pragma endregion
#pragma region Analysis_802_11n
				else if ( (wifiMode==WIFI_11N_HT20)||(wifiMode==WIFI_11N_HT40) )
				{   // [Case 03]: MIMO Analysis
					char referenceFileName[MAX_BUFFER_SIZE], analyzeMimoType[MAX_BUFFER_SIZE], analyzeMimoMode[MAX_BUFFER_SIZE];

					switch( g_Tester_Type )
					{
						case IQ_View:
							sprintf_s(analyzeMimoType, "EWC");
							if ( wifiStreamNum==WIFI_ONE_STREAM )  // MCS0 ~ MCS7
							{
								sprintf_s(analyzeMimoMode, "nxn");
								sprintf_s(referenceFileName, "");
							}
							else // MCS8 ~ MCS15 or more, need MIMO reference File *.ref
							{
								sprintf_s(analyzeMimoMode, "composite");
								err = GetWaveformFileName("EVM",
										"REFERENCE_FILE_NAME",
										wifiMode,
										l_txVerifyEvmParam.BANDWIDTH,
										l_txVerifyEvmParam.DATA_RATE,
										l_txVerifyEvmParam.PREAMBLE,
										l_txVerifyEvmParam.PACKET_FORMAT_11N,
										l_txVerifyEvmParam.GUARD_INTERVAL_11N,
										referenceFileName,
										MAX_BUFFER_SIZE);
								if ( ERR_OK!=err )
								{
									LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get reference file name.\n");
									throw logMessage;
								}
								else
								{
									LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] GetWaveformFileName(REFERENCE_FILE_NAME) return OK.\n");
								}
							}
							break;

						case IQ_nxn:
							sprintf_s(analyzeMimoType, "EWC");
							sprintf_s(analyzeMimoMode, "nxn");
							sprintf_s(referenceFileName, "");
							break;

						default:
							sprintf_s(analyzeMimoType, "EWC");
							sprintf_s(analyzeMimoMode, "nxn");
							sprintf_s(referenceFileName, "");
							break;
					}

					//Set Frequency Correction for 802.11n analysis
					err = ::LP_SetAnalysisParameterInteger("Analyze80211n", "frequencyCorr", g_WiFiGlobalSettingParam.ANALYSIS_11N_FREQUENCY_CORRELATION);
					if ( ERR_OK!=err )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_SetAnalysisParameterInteger() return error.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetAnalysisParameterInteger() return OK.\n");
					}

					err = ::LP_Analyze80211n( analyzeMimoType,
							analyzeMimoMode,
							g_WiFiGlobalSettingParam.ANALYSIS_11N_PHASE_CORR,
							g_WiFiGlobalSettingParam.ANALYSIS_11N_SYM_TIMING_CORR,
							g_WiFiGlobalSettingParam.ANALYSIS_11N_AMPLITUDE_TRACKING,
							g_WiFiGlobalSettingParam.ANALYSIS_11N_DECODE_PSDU,
							g_WiFiGlobalSettingParam.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST,
							referenceFileName,
							packetFormat);
					if ( ERR_OK!=err )
					{
						// Fail Analysis, thus save capture (Signal File) for debug
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_AnalyzeMIMO_Failed", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
						WiFiSaveSigFile(sigFileNameBuffer);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_Analyze80211n() return error.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_Analyze80211n() return OK.\n");
					}
				}
#pragma endregion

#pragma region Retrieve analysis Results
				/*-----------------------------*
				 *  Retrieve analysis Results  *
				 *-----------------------------*/
				char sTestDetail[MAX_BUFFER_SIZE] = {'\0'};
				sprintf_s(sTestDetail, MAX_BUFFER_SIZE, "WiFi_TX_Verify_Evm_%d_%s", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE);

				avgIteration++;
				analysisOK = true;

				// Store the result, includes EVM, power etc.
				if ( (wifiMode==WIFI_11B)||(wifiMode==WIFI_11AG) )        // 802.11a/b/g, but not n mode, only one stream
				{
					// Number of spatial streams
					l_txVerifyEvmReturn.SPATIAL_STREAM = 1;

					// EVM
					evmAvgAll[0][avgIteration-1] = ::LP_GetScalarMeasurement("evmAll",0);

					if ( -99.00 >= evmAvgAll[0][avgIteration-1] )
					{
						analysisOK = false;
						evmAvgAll[0][avgIteration-1] = NA_NUMBER;
						WiFiSaveSigFile(sTestDetail);
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_GetScalarMeasurement(evmAll) return error.\n");
						throw logMessage;
					}


					// Peak EVM
					if (wifiMode==WIFI_11B)
					{
						evmPk[avgIteration-1] = ::LP_GetScalarMeasurement("evmPk", 0);
						if ( -99.00 >= evmPk[avgIteration-1] )
						{
							analysisOK = false;
							evmPk[avgIteration-1] = NA_NUMBER;
							WiFiSaveSigFile(sTestDetail);
							err = -1;
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_GetScalarMeasurement(evmPk) return error.\n");
							throw logMessage;
						}
					}

					// Power
					rxRmsPowerDb[0][avgIteration-1] = ::LP_GetScalarMeasurement("rmsPowerNoGap",0);
					if ( -99.00 >= rxRmsPowerDb[0][avgIteration-1] )
					{
						analysisOK = false;
						rxRmsPowerDb[0][avgIteration-1] = NA_NUMBER;
						WiFiSaveSigFile(sTestDetail);
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_GetScalarMeasurement(rmsPowerNoGap) return error.\n");
						throw logMessage;
					}
					else
					{
						// One stream data rate, should be only one antenna is ON
						int antenaOrder = 0;
						err = CheckAntennaOrderByStream(l_txVerifyEvmReturn.SPATIAL_STREAM, l_txVerifyEvmParam.TX1, l_txVerifyEvmParam.TX2, l_txVerifyEvmParam.TX3, l_txVerifyEvmParam.TX4, &antenaOrder);
						if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] CheckAntennaOrderByStream() return error.\n");
							throw logMessage;
						}

						rxRmsPowerDb[0][avgIteration-1] = rxRmsPowerDb[0][avgIteration-1] + l_txVerifyEvmParam.CABLE_LOSS_DB[antenaOrder-1];
					}

					// Frequency Error
					freqErrorHz[avgIteration-1] = ::LP_GetScalarMeasurement("freqErr", 0);

					// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
					ampErrDb[0][avgIteration-1] = ::LP_GetScalarMeasurement("ampErrDb", 0);

					// IQ Match Phase Error.
					phaseErr[0][avgIteration-1] = ::LP_GetScalarMeasurement("phaseErr", 0);

					// RMS Phase Noise, mappin the value "RMS Phase Noise" in IQsignal
					phaseNoiseRms[avgIteration-1] = ::LP_GetScalarMeasurement("rmsPhaseNoise",0);

					// Symbol clock error for 11a/b/g
					symbolClockErr[avgIteration-1] = ::LP_GetScalarMeasurement("clockErr", 0);

					// Datarate
					if (wifiMode==WIFI_11B)
					{
						l_txVerifyEvmReturn.DATA_RATE = ::LP_GetScalarMeasurement("bitRateInMHz", 0);
					}
					else
					{
						l_txVerifyEvmReturn.DATA_RATE = ::LP_GetScalarMeasurement("dataRate", 0);
					}

				}
				else    // 802.11n, includes more than one stream
				{
					// Number of spatial streams
					l_txVerifyEvmReturn.SPATIAL_STREAM = (int) ::LP_GetScalarMeasurement("rateInfo_spatialStreams", 0);

					if (g_Tester_Type==IQ_View)
					{
						for(int i=0;i<l_txVerifyEvmReturn.SPATIAL_STREAM;i++)
						{
							// Get EVM test result
							evmAvgAll[i][avgIteration-1] = ::LP_GetScalarMeasurement("evmAvgAll", 0); // only can get EVM_ALL, means index always = 0
							if ( -99.00 >= evmAvgAll[i][avgIteration-1] )
							{
								analysisOK = false;
								evmAvgAll[i][avgIteration-1] = NA_NUMBER;
								WiFiSaveSigFile(sTestDetail);
								err = -1;
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_GetScalarMeasurement(evmAvgAll) return error.\n");
								throw logMessage;
							}
							// Get Power test result
							rxRmsPowerDb[i][avgIteration-1] = ::LP_GetScalarMeasurement("rxRmsPowerDb", i);
							if ( -99.00 >= rxRmsPowerDb[i][avgIteration-1] )
							{
								analysisOK = false;
								rxRmsPowerDb[i][avgIteration-1] = NA_NUMBER;
								WiFiSaveSigFile(sTestDetail);
								err = -1;
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_GetScalarMeasurement(rxRmsPowerDb) return error.\n");
								throw logMessage;
							}
							else
							{
								//The current code will work correctly with 4x4 MIMO.
								int antenaOrder = 0;
								err = CheckAntennaOrderByStream(i+1, l_txVerifyEvmParam.TX1, l_txVerifyEvmParam.TX2, l_txVerifyEvmParam.TX3, l_txVerifyEvmParam.TX4, &antenaOrder);
								if ( ERR_OK!=err )
								{
									LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] CheckAntennaOrderByStream() return error.\n");
									throw logMessage;
								}

								rxRmsPowerDb[i][avgIteration-1] = rxRmsPowerDb[i][avgIteration-1] + l_txVerifyEvmParam.CABLE_LOSS_DB[antenaOrder-1];
							}

							// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
							ampErrDb[i][avgIteration-1] = ::LP_GetScalarMeasurement("IQImbal_amplDb", i); // Index always = 0
							phaseErr[i][avgIteration-1] = ::LP_GetScalarMeasurement("IQImbal_phaseDeg",i);


						}
					}
					else	// g_Tester_Type == IQnxn
					{
						for(int i=0;i<l_txVerifyEvmReturn.SPATIAL_STREAM;i++)
						{
							// EVM
							evmAvgAll[i][avgIteration-1] = ::LP_GetScalarMeasurement("evmAvgAll",i);
							if ( -99.00 >= evmAvgAll[i][avgIteration-1] )
							{
								analysisOK = false;
								evmAvgAll[i][avgIteration-1] = NA_NUMBER;
								WiFiSaveSigFile(sTestDetail);
								err = -1;
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_GetScalarMeasurement(evmAvgAll) return error.\n");
								throw logMessage;
							}

							// Power
							rxRmsPowerDb[i][avgIteration-1] = ::LP_GetScalarMeasurement("rxRmsPowerDb", i*(l_txVerifyEvmReturn.SPATIAL_STREAM+1));
							if ( -99.00 >= rxRmsPowerDb[i][avgIteration-1] )
							{
								analysisOK = false;
								rxRmsPowerDb[i][avgIteration-1] = NA_NUMBER;
								WiFiSaveSigFile(sTestDetail);
								err = -1;
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_GetScalarMeasurement(rxRmsPowerDb) return error.\n");
								throw logMessage;
							}
							else
							{
								//The current code will work correctly with 4x4 MIMO.
								int antenaOrder = 0;
								err = CheckAntennaOrderByStream(i+1, l_txVerifyEvmParam.TX1, l_txVerifyEvmParam.TX2, l_txVerifyEvmParam.TX3, l_txVerifyEvmParam.TX4, &antenaOrder);
								if ( ERR_OK!=err )
								{
									LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] CheckAntennaOrderByStream() return error.\n");
									throw logMessage;
								}

								rxRmsPowerDb[i][avgIteration-1] = rxRmsPowerDb[i][avgIteration-1] + l_txVerifyEvmParam.CABLE_LOSS_DB[antenaOrder-1];
							}

							// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
							ampErrDb[i][avgIteration-1] = ::LP_GetScalarMeasurement("IQImbal_amplDb", i); // Index always = 0
							phaseErr[i][avgIteration-1] = ::LP_GetScalarMeasurement("IQImbal_phaseDeg",i);

						}
					}
					// Frequency Error
					freqErrorHz[avgIteration-1] = ::LP_GetScalarMeasurement("freqErrorHz",0);

					// Datarate
					l_txVerifyEvmReturn.DATA_RATE = ::LP_GetScalarMeasurement("rateInfo_dataRateMbps", 0);

					// RMS Phase Noise, mappin the value "RMS Phase Noise" in IQsignal
					phaseNoiseRms[avgIteration-1] = ::LP_GetScalarMeasurement("PhaseNoiseDeg_RmsAll", 0);

					// Symbol clock error for 11n
					symbolClockErr[avgIteration-1] = ::LP_GetScalarMeasurement("symClockErrorPpm", 0);
				}
#pragma endregion
			}   // End - avgIteration

#pragma region Averaging and Saving Test Result
			/*----------------------------------*
			 * Averaging and Saving Test Result *
			 *----------------------------------*/
			if ( (ERR_OK==err) && captureOK && analysisOK )
			{
				double dummyAve, dummyMax, dummyMin;
				//double dummyAvg[MAX_DATA_STREAM];

				for(int i=0;i<l_txVerifyEvmReturn.SPATIAL_STREAM;i++)
				{
					// Average EVM test result
					::AverageTestResult(&evmAvgAll[i][0], avgIteration, LOG_20, l_txVerifyEvmReturn.EVM_AVG[i], l_txVerifyEvmReturn.EVM_MAX[i], l_txVerifyEvmReturn.EVM_MIN[i]);

					// Average Power test result
					::AverageTestResult(&rxRmsPowerDb[i][0], avgIteration, LOG_10, l_txVerifyEvmReturn.POWER_AVG[i], l_txVerifyEvmReturn.POWER_MAX[i], l_txVerifyEvmReturn.POWER_MIN[i]);

					// Average Amp Error test result
					//::AverageTestResult(&ampErrDb[i][0], avgIteration, LOG_20, dummyAvg[i], dummyMax, dummyMin);
					::AverageTestResult(&ampErrDb[i][0], avgIteration, LOG_20, l_txVerifyEvmReturn.AMP_ERR_DB_STREAM[i], dummyMax, dummyMin);

					// Average Phase Error
					::AverageTestResult(&phaseErr[i][0], avgIteration, Linear, l_txVerifyEvmReturn.PHASE_ERR_STREAM[i], dummyMax, dummyMin);

				}
				::AverageTestResult(l_txVerifyEvmReturn.EVM_AVG, l_txVerifyEvmReturn.SPATIAL_STREAM, RMS_LOG_20, l_txVerifyEvmReturn.EVM_AVG_DB, dummyMax, dummyMin);
				::AverageTestResult(l_txVerifyEvmReturn.POWER_AVG, l_txVerifyEvmReturn.SPATIAL_STREAM, LOG_10, l_txVerifyEvmReturn.POWER_AVG_DBM, dummyMax, dummyMin);
				::AverageTestResult(l_txVerifyEvmReturn.AMP_ERR_DB_STREAM, l_txVerifyEvmReturn.SPATIAL_STREAM, LOG_20, l_txVerifyEvmReturn.AMP_ERR_DB, dummyMax, dummyMin);
				::AverageTestResult(l_txVerifyEvmReturn.PHASE_ERR_STREAM,  l_txVerifyEvmReturn.SPATIAL_STREAM, Linear, l_txVerifyEvmReturn.PHASE_ERR, dummyMax, dummyMin);

				//EVM in percentage
				l_txVerifyEvmReturn.EVM_AVG_PERCENT = 100*pow(10, (l_txVerifyEvmReturn.EVM_AVG_DB/20));
				for(int i=0;i<l_txVerifyEvmReturn.SPATIAL_STREAM;i++)
				{
					l_txVerifyEvmReturn.EVM_AVG_STREAM_PERCENT[i] = 100*pow(10, (l_txVerifyEvmReturn.EVM_AVG[i]/20));
					l_txVerifyEvmReturn.EVM_MAX_STREAM_PERCENT[i] = 100*pow(10, (l_txVerifyEvmReturn.EVM_MAX[i]/20));
					l_txVerifyEvmReturn.EVM_MIN_STREAM_PERCENT[i] = 100*pow(10, (l_txVerifyEvmReturn.EVM_MIN[i]/20));

				}
				//END

				if (wifiMode==WIFI_11B)
				{
					// (11b only) Average EVM_PK test result
					::AverageTestResult(&evmPk[0], avgIteration, LOG_20, dummyAve, l_txVerifyEvmReturn.EVM_PK_DB, dummyMin);
				}

				// Average RMS Phase Noise test result
				::AverageTestResult(&phaseNoiseRms[0], avgIteration, Linear, l_txVerifyEvmReturn.PHASE_NOISE_RMS_ALL, dummyMax, dummyMin);

				// Average Freq Error test result
				::AverageTestResult(&freqErrorHz[0], avgIteration, Linear, l_txVerifyEvmReturn.FREQ_ERROR_AVG, l_txVerifyEvmReturn.FREQ_ERROR_MAX, l_txVerifyEvmReturn.FREQ_ERROR_MIN);

				// Average Symbol clock Error test result
				::AverageTestResult(&symbolClockErr[0], avgIteration, Linear, l_txVerifyEvmReturn.SYMBOL_CLK_ERR, dummyMax, dummyMin);

				// Transfer the Freq Error result unit to ppm
				l_txVerifyEvmReturn.FREQ_ERROR_AVG = l_txVerifyEvmReturn.FREQ_ERROR_AVG/l_txVerifyEvmParam.FREQ_MHZ;
				l_txVerifyEvmReturn.FREQ_ERROR_MAX = l_txVerifyEvmReturn.FREQ_ERROR_MAX/l_txVerifyEvmParam.FREQ_MHZ;
				l_txVerifyEvmReturn.FREQ_ERROR_MIN = l_txVerifyEvmReturn.FREQ_ERROR_MIN/l_txVerifyEvmParam.FREQ_MHZ;

				// Added GET_TX_TARGET_POWER feature for AR6004. When TX_POWER_DBM is set to -99, default target power will be applied.
				// TBD: 2010 not support for this
				l_txVerifyEvmReturn.TX_POWER_DBM = l_txVerifyEvmParam.TX_POWER_DBM; // -cfy@sunnyvale, 2012/3/13-
			}
			else
			{
				// do nothing
			}
#pragma endregion
		

		// This is a special case, only if "DUT_KEEP_TRANSMIT=0" then must do TX_STOP manually.
		if ( (g_WiFiGlobalSettingParam.DUT_KEEP_TRANSMIT==0)&&(g_vDutTxActived==true) )
		{
			/*-----------*
			 *  Tx Stop  *
			 *-----------*/
			err = ::vDUT_Run(g_WiFi_Dut, "TX_STOP");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_STOP) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
			}
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err && captureOK && analysisOK )
		{
			sprintf_s(l_txVerifyEvmReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txVerifyEvmReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_txVerifyEvmReturn.ERROR_MESSAGE, msg);

		if ( g_vDutTxActived )
		{
			int err = ERR_OK;
			err = vDUT_Run(g_WiFi_Dut, "TX_STOP");
			if( err == ERR_OK )
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
			}
		}
	}
	catch(...)
	{
		ReturnErrorMessage(l_txVerifyEvmReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
		err = -1;

		if ( g_vDutTxActived )
		{
			int err = ERR_OK;
			err = vDUT_Run(g_WiFi_Dut, "TX_STOP");
			if( err == ERR_OK )
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
			}
		}
	}

	// This is a special case, only when some error occur before the TX_STOP.
	// This case will take care by the error handling, but must do TX_STOP manually.
	//if ( g_vDutTxActived )
	//{
	//	vDUT_Run(g_WiFi_Dut, "TX_STOP");
	//}
	//else
	//{
	//	// do nothing
	//}

	// Free memory
	evmAvgAll.clear();
	evmPk.clear();
	rxRmsPowerDb.clear();
	freqErrorHz.clear();
	ampErrDb.clear();
	phaseErr.clear();
	phaseNoiseRms.clear();


	return err;
}

int WiFi_TX_Verify_Evm_2010ExtTest(void)
{
	int    err = ERR_OK;

	bool   analysisOK = false, captureOK = false, enableMultiCapture = false;
	int    avgIteration = 0;
	int    channel = 0, HT40ModeOn = 0, packetFormat = 0;
	int    dummyValue = 0;
	int    wifiMode = 0, wifiStreamNum = 0;
	double samplingTimeUs = 0, peakToAvgRatio = 0, cableLossDb = 0;
	double chainGainDb = 0.0;
	char   vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};
	try
	{
	    // Error return of this function is irrelevant
		CheckDutTransmitStatus();

#pragma region Prepare input parameters

		err = CheckTxEvmParameters( &channel, &wifiMode, &wifiStreamNum, &samplingTimeUs, &cableLossDb, &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters CheckTxEvmParameters() return OK.\n");
		}

		// Modified /* -cfy@sunnyvale, 2012/3/13- */
		// Considering stream number while calculating chain gain for composite measurement so as to set the proper Rx amplitude in tester
		int antennaNum = l_txVerifyEvmParam.TX1 + l_txVerifyEvmParam.TX2 + l_txVerifyEvmParam.TX3 + l_txVerifyEvmParam.TX4;
		if(antennaNum > 0)
		{
			chainGainDb = 10.0 * log10((double)antennaNum / wifiStreamNum);
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Set Chain Gain of %.2f dB.\n", chainGainDb);
		}
		else
		{
			chainGainDb = 0.0;
		}
		/* <><~~ */

		if (0==strcmp(l_txVerifyEvmParam.PACKET_FORMAT_11N, "MIXED"))
		{
			packetFormat = 1;
		}
		else if (0 == strcmp(l_txVerifyEvmParam.PACKET_FORMAT_11N, "GREENFIELD"))
		{
			packetFormat = 2;
		}
		else
		{
			packetFormat = 0;
		}

#pragma endregion

		/*----------------------------*
		 * Disable VSG output signal  *
		 *----------------------------*/
		if (!g_useLpDut)
		{
			// make sure no signal is generated by the VSG
			err = ::LP_EnableVsgRF(0);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
			}
		}
		/*--------------------*
		 * Setup IQTester VSA *
		 *--------------------*/
		err = ::LP_SetVsaAmplitudeTolerance(g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to set VSA amplitude tolerance in dB.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsaAmplitudeTolerance(%.3f) return OK.\n", g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);
		}

#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to transmit *
		 *---------------------------*/
		// Set DUT RF frquency, tx power, antenna, data rate
		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_Dut);

		if( wifiMode==WIFI_11N_HT40 )
		{
			HT40ModeOn = 1;   // 1: HT40 mode;
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_txVerifyEvmParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_txVerifyEvmParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_txVerifyEvmParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",      l_txVerifyEvmParam.FREQ_MHZ);
		}

		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",		  l_txVerifyEvmParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_txVerifyEvmParam.PACKET_FORMAT_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N", l_txVerifyEvmParam.GUARD_INTERVAL_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",		  l_txVerifyEvmParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",		  l_txVerifyEvmParam.BANDWIDTH);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		  HT40ModeOn);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",				  l_txVerifyEvmParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",				  l_txVerifyEvmParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",				  l_txVerifyEvmParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",				  l_txVerifyEvmParam.TX4);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "SAMPLING_TIME_US",  samplingTimeUs); // -cfy@sunnyvale, 2012/3/13-
		// -cfy@sunnyvale, 2012/3/13-
		// Added GET_TX_TARGET_POWER feature for AR6004. When TX_POWER_DBM is set to -99, default target power will be applied.
		if ( (l_txVerifyEvmParam.TX_POWER_DBM == TX_TARGET_POWER_FLAG) )
		{ // Indicate that the Tx target power will be determined by channel and data rate, rather than the value input by users in IQwizard
			if ( g_dutConfigChanged == true )
			{
				/*-----------*
				 *  Tx Stop  *
				 *-----------*/
				err = ::vDUT_Run(g_WiFi_Dut, "TX_STOP");
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
						throw logMessage;
					}
					else	// Just return normal error message in this case
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_STOP) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					g_vDutTxActived = false;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
				}

				/*----------------------*
				 *  Get Tx Target Power  *
				 *-----------------------*/
				double tx_target_power = 0;
				err = ::vDUT_Run(g_WiFi_Dut, "GET_TX_TARGET_POWER");
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
						throw logMessage;
					}
					else	// Just return normal error message in this case
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(GET_TX_TARGET_POWER) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(GET_TX_TARGET_POWER) return OK.\n");
				}
				vDUT_GetDoubleParameter (g_WiFi_Dut, "TX_TARGET_POWER_DBM", &tx_target_power);
				l_txVerifyEvmParam.TX_POWER_DBM = tx_target_power;
				g_last_TxPower_dBm_Record = l_txVerifyEvmParam.TX_POWER_DBM;
			}
			else
			{
				l_txVerifyEvmParam.TX_POWER_DBM = g_last_TxPower_dBm_Record;
			}
		}
		else
		{
			// do nothing
		}
		/* <><~~ */

		vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM",	  l_txVerifyEvmParam.TX_POWER_DBM);
		TM_SetDoubleParameter(g_WiFi_Test_ID, "TX_POWER_DBM", l_txVerifyEvmParam.TX_POWER_DBM); // -cfy@sunnyvale, 2012/3/13-

		if ( (g_dutConfigChanged==true)||(g_vDutTxActived==false) )
		{
			if ( g_vDutTxActived==true )
			{
				/*-----------*
				 *  Tx Stop  *
				 *-----------*/
				err = ::vDUT_Run(g_WiFi_Dut, "TX_STOP");
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
						throw logMessage;
					}
					else	// Just return normal error message in this case
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_STOP) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					g_vDutTxActived = false;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
				}
			}
			else
			{
				// continue Dut configuration
			}

			err = vDUT_Run(g_WiFi_Dut, "RF_SET_FREQ");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RF_SET_FREQ) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RF_SET_FREQ) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_SET_BW");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_SET_BW) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_SET_BW) return OK.\n");
			}
			err = vDUT_Run(g_WiFi_Dut, "TX_SET_DATA_RATE");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_SET_DATA_RATE) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_SET_DATA_RATE) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_SET_ANTENNA");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_SET_ANTENNA) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_SET_ANTENNA) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_PRE_TX");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_PRE_TX) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_PRE_TX) return OK.\n");
			}

			if ( (g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE)&&(enableMultiCapture==false) )
			{
				/*--------------------------------------*
				 * Set Trigger Timeout for DUT TX_START *
				 *--------------------------------------*/
				err = ::LP_SetVsaTriggerTimeout(g_WiFiGlobalSettingParam.VSA_TRIGGER_TIMEOUT_SEC);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi] Fail to set VSA trigger timeout, LP_SetVsaTriggerTimeout(%d sec) return error.\n", g_WiFiGlobalSettingParam.VSA_TRIGGER_TIMEOUT_SEC);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsaTriggerTimeout(%d sec) return OK.\n", g_WiFiGlobalSettingParam.VSA_TRIGGER_TIMEOUT_SEC);
				}

				/*-------------------------------*
				 * Perform IQ2010Ext VSA capture *
				 *-------------------------------*/
				err = ::LP_IQ2010EXT_InitiateMultiCapture(  l_txVerifyEvmParam.DATA_RATE,
						l_txVerifyEvmParam.FREQ_MHZ,
						l_txVerifyEvmParam.TX_POWER_DBM-cableLossDb+chainGainDb,
						(int)samplingTimeUs,
						g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
						g_WiFiGlobalSettingParam.EVM_AVERAGE,
						g_WiFiGlobalSettingParam.VSA_PORT,
						l_txVerifyEvmParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
						);
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to new initiate MultiCapture at %d MHz.\n", l_txVerifyEvmParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_InitiateMultiCapture() at %d MHz return OK.\n", l_txVerifyEvmParam.FREQ_MHZ);
				}

				enableMultiCapture = true;
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_START");
			if ( ERR_OK!=err )
			{
				g_vDutTxActived = false;
				// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_START) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutTxActived = true;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_START) return OK.\n");
			}

			// Delay for DUT settle
			if (0!=g_WiFiGlobalSettingParam.DUT_TX_SETTLE_TIME_MS)
			{
				Sleep(g_WiFiGlobalSettingParam.DUT_TX_SETTLE_TIME_MS);
			}
			else
			{
				// do nothing
			}
		}
		else
		{
			// g_dutConfigChanged==false, do nothing
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Since g_dutConfigChanged = false, skip Dut control section.\n");
		}
#pragma endregion                                                     
  /*---------------------------*
		 * Start IQ2010_Ext Function *
		 *---------------------------*/
			analysisOK = false;
			captureOK  = false;

			if ( enableMultiCapture==false )
			{
				/*-------------------------------*
				 * Perform IQ2010Ext VSA capture *
				 *-------------------------------*/
				if(g_WiFiGlobalSettingParam.AUTO_GAIN_CONTROL_ENABLE)
				{
					//Capture 1 packet to do power analysis
					err = ::LP_IQ2010EXT_InitiateMultiCapture(  l_txVerifyEvmParam.DATA_RATE,
							l_txVerifyEvmParam.FREQ_MHZ,
							l_txVerifyEvmParam.TX_POWER_DBM-cableLossDb+chainGainDb,
							(int)samplingTimeUs,
							g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
							1,
							g_WiFiGlobalSettingParam.VSA_PORT,
							l_txVerifyEvmParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
							);

				}
				else
				{
					err = ::LP_IQ2010EXT_InitiateMultiCapture(  l_txVerifyEvmParam.DATA_RATE,
							l_txVerifyEvmParam.FREQ_MHZ,
							l_txVerifyEvmParam.TX_POWER_DBM-cableLossDb+chainGainDb,
							(int)samplingTimeUs,
							g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
							g_WiFiGlobalSettingParam.EVM_AVERAGE,
							g_WiFiGlobalSettingParam.VSA_PORT,
							l_txVerifyEvmParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
							);
				}
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to new initiate MultiCapture at %d MHz.\n", l_txVerifyEvmParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_InitiateMultiCapture() at %d MHz return OK.\n", l_txVerifyEvmParam.FREQ_MHZ);
				}

				enableMultiCapture = true;
			}

			err = ::LP_IQ2010EXT_FinishMultiCapture();
			if( ERR_OK!=err )	// capture is failed
			{
				// Fail Capture
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_FinishMultiCapture() return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_FinishMultiCapture() return OK.\n");
			}

			/*--------------*
			 *  Capture OK  *
			 *--------------*/
			captureOK = true;
			if (1==g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
			{
				// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_SaveAlways", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
			}
			else
			{
				// do nothing
			}
#pragma region Auto_Gain_Control
			/*------------------------------
			  Search VSA optimal amplitude
			  Perform Power Analysis
			  --------------------------------*/
			if(g_WiFiGlobalSettingParam.AUTO_GAIN_CONTROL_ENABLE)
			{
				double powerMeasured = NA_NUMBER;
				double dummyDouble   = NA_NUMBER;

				for(int loopIndex = 0; loopIndex < AUTO_GAIN_CONTROL_LOOP; loopIndex ++)
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Doing auto gain coontrol(Loop: %d).\n", loopIndex+1);

					//IQ2010Ext Power Analysis
					err = ::LP_IQ2010EXT_AnalyzePower();
					if ( ERR_OK!=err )
					{
						// Fail Analysis, thus save capture (Signal File) for debug
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_EVM_Analysis_Failed", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
						WiFiSaveSigFile(sigFileNameBuffer);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_AnalyzePower(%d) return error.\n", l_txVerifyEvmParam.FREQ_MHZ);
						throw logMessage;
					}
					else
					{
						//do nothing
					}

					// P_av_no_gap_all_dBm
					err = ::LP_IQ2010EXT_GetDoubleMeasurements( "P_av_no_gap_all_dBm", &powerMeasured,&dummyDouble, &dummyDouble);
					if ( ERR_OK!=err )
					{
						analysisOK = false;
						// Fail Analysis, thus save capture (Signal File) for debug
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_EVM_Analysis_Failed", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
						WiFiSaveSigFile(sigFileNameBuffer);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(P_av_no_gap_all_dBm) return error.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(P_av_no_gap_all_dBm: %2.1f) return OK.\n", (powerMeasured + cableLossDb));
					}

					/*-----------------------------------------------------------------------------
					  Compare the peak power with current VSA amplitude:
					  1) If peak power measured is exceed current VSA ampltude
					  2) If peak power measured is below target power
					  Get average power and add peak to average ratio, then reset the VSA amplitude
					  -----------------------------------------------------------------------------*/
					if( (powerMeasured > (l_txVerifyEvmParam.TX_POWER_DBM + g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB - cableLossDb)) ||
							(powerMeasured < (l_txVerifyEvmParam.TX_POWER_DBM - g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB - cableLossDb))
					  )
					{

						//Skip 0 count and capture 1 packet
						err = ::LP_IQ2010EXT_InitiateMultiCapture(  l_txVerifyEvmParam.DATA_RATE,
								l_txVerifyEvmParam.FREQ_MHZ,
								powerMeasured,
								(int)samplingTimeUs,
								0,
								1,
								g_WiFiGlobalSettingParam.VSA_PORT,
								powerMeasured+peakToAvgRatio+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
								);
						if( ERR_OK!=err )	// capture is failed
						{
							// Fail Capture
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to new initiate MultiCapture at %d MHz.\n", l_txVerifyEvmParam.FREQ_MHZ);
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_InitiateMultiCapture() at %d MHz return OK.\n", l_txVerifyEvmParam.FREQ_MHZ);
						}

						err = ::LP_IQ2010EXT_FinishMultiCapture();
						if( ERR_OK!=err )	// capture is failed
						{
							// Fail Capture
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_FinishMultiCapture() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_FinishMultiCapture() return OK.\n");
						}

					}
					else
					{
						//VSA ampltude is optimal, break the loop
						break;
					}



				}//End the loop of auto gain control

				//After get optimal vsa amplitude, initial capture and finish capture again
				double dutTxPower = NA_NUMBER;
				if(powerMeasured > NA_NUMBER)
				{
					dutTxPower = powerMeasured + cableLossDb - chainGainDb;
				}
				else
				{
					dutTxPower =l_txVerifyEvmParam.TX_POWER_DBM;
				}

				err = ::LP_IQ2010EXT_InitiateMultiCapture(  l_txVerifyEvmParam.DATA_RATE,
						l_txVerifyEvmParam.FREQ_MHZ,
						dutTxPower-cableLossDb+chainGainDb,
						(int)samplingTimeUs,
						g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
						g_WiFiGlobalSettingParam.EVM_AVERAGE,
						g_WiFiGlobalSettingParam.VSA_PORT,
						dutTxPower-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
						);

				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to new initiate MultiCapture at %d MHz.\n", l_txVerifyEvmParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_InitiateMultiCapture() at %d MHz return OK.\n", l_txVerifyEvmParam.FREQ_MHZ);
				}

				err = ::LP_IQ2010EXT_FinishMultiCapture();
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_FinishMultiCapture() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_FinishMultiCapture() return OK.\n");
				}

			}
			else
			{
				//Did not perform auto gain control
			}
#pragma endregion

			/*--------------------------*
			 *  IQ2010Ext EVM Analysis  *
			 *--------------------------*/
#pragma region Analysis_802_11b
			if ( wifiMode==WIFI_11B )
			{   // [Case 01]: 802.11b Analysis
				err = ::LP_IQ2010EXT_Analyze80211b( (IQV_EQ_ENUM)		  g_WiFiGlobalSettingParam.ANALYSIS_11B_EQ_TAPS,
						(IQV_DC_REMOVAL_ENUM) g_WiFiGlobalSettingParam.ANALYSIS_11B_DC_REMOVE_FLAG,
						(IQV_11B_METHOD_ENUM) g_WiFiGlobalSettingParam.ANALYSIS_11B_METHOD_11B
						);
				if (ERR_OK!=err)
				{	// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_IQ2010EXT_Analyze80211b_Failed", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);

					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_Analyze80211b() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_Analyze80211b() return OK.\n");
				}
			}
#pragma endregion
#pragma region Analysis_802_11ag
			else if ( wifiMode==WIFI_11AG )
			{   // [Case 02]: 802.11a/g Analysis
				err = ::LP_IQ2010EXT_Analyze80211ag(  (IQV_PH_CORR_ENUM)	g_WiFiGlobalSettingParam.ANALYSIS_11AG_PH_CORR_MODE,
						(IQV_CH_EST_ENUM)		g_WiFiGlobalSettingParam.ANALYSIS_11AG_CH_ESTIMATE,
						(IQV_SYM_TIM_ENUM)	g_WiFiGlobalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR,
						(IQV_FREQ_SYNC_ENUM)	g_WiFiGlobalSettingParam.ANALYSIS_11AG_FREQ_SYNC,
						(IQV_AMPL_TRACK_ENUM) g_WiFiGlobalSettingParam.ANALYSIS_11AG_AMPL_TRACK
						);
				if ( ERR_OK!=err )
				{
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_IQ2010EXT_Analyze80211ag_Failed", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_Analyze80211ag() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_Analyze80211ag() return OK.\n");
				}
			}
#pragma endregion
#pragma region Analysis_802_11n
			else if ( (wifiMode==WIFI_11N_HT20)||(wifiMode==WIFI_11N_HT40) )
			{   // [Case 03]: MIMO Analysis
				char referenceFileName[MAX_BUFFER_SIZE], analyzeMimoType[MAX_BUFFER_SIZE], analyzeMimoMode[MAX_BUFFER_SIZE];

				//Set Frequency Correction for 802.11n analysis
				err = ::LP_SetAnalysisParameterInteger("Analyze80211n", "frequencyCorr", g_WiFiGlobalSettingParam.ANALYSIS_11N_FREQUENCY_CORRELATION);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_SetAnalysisParameterInteger() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetAnalysisParameterInteger() return OK.\n");
				}

				switch( g_Tester_Type )
				{
					case IQ_View:
						sprintf_s(analyzeMimoType, "EWC");
						if ( wifiStreamNum==WIFI_ONE_STREAM )  // MCS0 ~ MCS7
						{
							sprintf_s(analyzeMimoMode, "nxn");
							sprintf_s(referenceFileName, "");

							err = ::LP_IQ2010EXT_Analyze80211nSISO( g_WiFiGlobalSettingParam.ANALYSIS_11N_PHASE_CORR,
									g_WiFiGlobalSettingParam.ANALYSIS_11N_SYM_TIMING_CORR,
									g_WiFiGlobalSettingParam.ANALYSIS_11N_AMPLITUDE_TRACKING,
									g_WiFiGlobalSettingParam.ANALYSIS_11N_DECODE_PSDU,
									g_WiFiGlobalSettingParam.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST,
									packetFormat);
							if ( ERR_OK!=err )
							{
								// Fail Analysis, thus save capture (Signal File) for debug
								sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_IQ2010EXT_AnalyzeMIMO_Failed", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
								WiFiSaveSigFile(sigFileNameBuffer);
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_Analyze80211nSISO() return error.\n");
								throw logMessage;
							}
							else
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_Analyze80211nSISO() return OK.\n");
							}
						}
						else // MCS8 ~ MCS15 or more, need MIMO reference File *.ref
						{
							sprintf_s(analyzeMimoMode, "composite");
							err = GetWaveformFileName("EVM",
									"REFERENCE_FILE_NAME",
									wifiMode,
									l_txVerifyEvmParam.BANDWIDTH,
									l_txVerifyEvmParam.DATA_RATE,
									l_txVerifyEvmParam.PREAMBLE,
									l_txVerifyEvmParam.PACKET_FORMAT_11N,
									l_txVerifyEvmParam.GUARD_INTERVAL_11N,
									referenceFileName,
									MAX_BUFFER_SIZE);
							if ( ERR_OK!=err )
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get reference file name.\n");
								throw logMessage;
							}
							else
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] GetWaveformFileName(REFERENCE_FILE_NAME) return OK.\n");
							}

							err = ::LP_IQ2010EXT_Analyze80211nComposite( g_WiFiGlobalSettingParam.ANALYSIS_11N_PHASE_CORR,
									g_WiFiGlobalSettingParam.ANALYSIS_11N_SYM_TIMING_CORR,
									g_WiFiGlobalSettingParam.ANALYSIS_11N_AMPLITUDE_TRACKING,
									g_WiFiGlobalSettingParam.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST,
									referenceFileName,
									packetFormat);
							if ( ERR_OK!=err )
							{
								// Fail Analysis, thus save capture (Signal File) for debug
								sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_IQ2010EXT_AnalyzeMIMO_Failed", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
								WiFiSaveSigFile(sigFileNameBuffer);
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_Analyze80211nComposite() return error. (%s) (%d)\n", referenceFileName, err);
								throw logMessage;
							}
							else
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_Analyze80211nComposite() return OK.\n");
							}
						}
						break;

					case IQ_nxn:
						sprintf_s(analyzeMimoType, "EWC");
						sprintf_s(analyzeMimoMode, "nxn");
						sprintf_s(referenceFileName, "");

						// TODO: IQ2010Ext not supported

						break;

					default:
						sprintf_s(analyzeMimoType, "EWC");
						sprintf_s(analyzeMimoMode, "nxn");
						sprintf_s(referenceFileName, "");

						// TODO: IQ2010Ext not supported

						break;
				}
			}
#pragma endregion

#pragma region Retrieve analysis Results
			/*---------------------------------------*
			 *  Retrieve IQ2010EXT analysis Results  *
			 *---------------------------------------*/
			char sTestDetail[MAX_BUFFER_SIZE] = {'\0'};
			sprintf_s(sTestDetail, MAX_BUFFER_SIZE, "WiFi_TX_Verify_Evm_%d_%s", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE);
			analysisOK = true;

			double dummyAvg = 0, dummyMax = 0, dummyMin = 0;
			double bufferReal [MAX_BUFFER_SIZE] = {0};
			double bufferImage[MAX_BUFFER_SIZE] = {0};

			// Store the result, includes EVM, power etc.
			if ( (wifiMode==WIFI_11B)||(wifiMode==WIFI_11AG) )        // 802.11a/b/g, but not n mode, only one stream
			{
				// Number of spatial streams
				l_txVerifyEvmReturn.SPATIAL_STREAM = 1;

				// EVM
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "evmAll", &l_txVerifyEvmReturn.EVM_AVG[0], &l_txVerifyEvmReturn.EVM_MIN[0], &l_txVerifyEvmReturn.EVM_MAX[0] );

				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txVerifyEvmReturn.EVM_AVG_DB = l_txVerifyEvmReturn.EVM_AVG[0] = l_txVerifyEvmReturn.EVM_MIN[0] = l_txVerifyEvmReturn.EVM_MAX[0] = NA_NUMBER;

					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(evmAll) return error.\n");
					throw logMessage;
				}
				else
				{
					l_txVerifyEvmReturn.EVM_AVG_DB = l_txVerifyEvmReturn.EVM_AVG[0];

				}

				// Peak EVM
				if (wifiMode==WIFI_11B)
				{
					err = ::LP_IQ2010EXT_GetDoubleMeasurements( "evmPk", &dummyAvg, &dummyMin, &l_txVerifyEvmReturn.EVM_PK_DB );
					if ( ERR_OK!=err )
					{
						analysisOK = false;
						l_txVerifyEvmReturn.EVM_PK_DB = NA_NUMBER;
						WiFiSaveSigFile(sTestDetail);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(evmPk) return error.\n");
						throw logMessage;
					}
				}

				// Power
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "rmsPowerNoGap", &l_txVerifyEvmReturn.POWER_AVG[0], &l_txVerifyEvmReturn.POWER_MIN[0], &l_txVerifyEvmReturn.POWER_MAX[0] );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txVerifyEvmReturn.POWER_AVG_DBM = l_txVerifyEvmReturn.POWER_AVG[0] = l_txVerifyEvmReturn.POWER_MIN[0] = l_txVerifyEvmReturn.POWER_MAX[0] = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(rmsPowerNoGap) return error.\n");
					throw logMessage;
				}
				else
				{
					// One stream data rate, should be only one antenna is ON
					int antenaOrder = 0;
					err = CheckAntennaOrderByStream(l_txVerifyEvmReturn.SPATIAL_STREAM, l_txVerifyEvmParam.TX1, l_txVerifyEvmParam.TX2, l_txVerifyEvmParam.TX3, l_txVerifyEvmParam.TX4, &antenaOrder);
					if ( ERR_OK!=err )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] CheckAntennaOrderByStream() return error.\n");
						throw logMessage;
					}

					l_txVerifyEvmReturn.POWER_AVG[0]	 = l_txVerifyEvmReturn.POWER_AVG[0] + l_txVerifyEvmParam.CABLE_LOSS_DB[antenaOrder-1];
					l_txVerifyEvmReturn.POWER_MIN[0]	 = l_txVerifyEvmReturn.POWER_MIN[0] + l_txVerifyEvmParam.CABLE_LOSS_DB[antenaOrder-1];
					l_txVerifyEvmReturn.POWER_MAX[0]	 = l_txVerifyEvmReturn.POWER_MAX[0] + l_txVerifyEvmParam.CABLE_LOSS_DB[antenaOrder-1];
					l_txVerifyEvmReturn.POWER_AVG_DBM    = l_txVerifyEvmReturn.POWER_AVG[0];
				}

				// Frequency Error
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "freqErr", &l_txVerifyEvmReturn.FREQ_ERROR_AVG, &l_txVerifyEvmReturn.FREQ_ERROR_MIN, &l_txVerifyEvmReturn.FREQ_ERROR_MAX );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txVerifyEvmReturn.FREQ_ERROR_AVG = l_txVerifyEvmReturn.FREQ_ERROR_MIN = l_txVerifyEvmReturn.FREQ_ERROR_MAX = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(freqErr) return error.\n");
					throw logMessage;
				}
				else
				{
					// Transfer the Freq Error result unit to ppm
					l_txVerifyEvmReturn.FREQ_ERROR_AVG = l_txVerifyEvmReturn.FREQ_ERROR_AVG/l_txVerifyEvmParam.FREQ_MHZ;
					l_txVerifyEvmReturn.FREQ_ERROR_MAX = l_txVerifyEvmReturn.FREQ_ERROR_MAX/l_txVerifyEvmParam.FREQ_MHZ;
					l_txVerifyEvmReturn.FREQ_ERROR_MIN = l_txVerifyEvmReturn.FREQ_ERROR_MIN/l_txVerifyEvmParam.FREQ_MHZ;
				}

				// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "ampErrDb", &l_txVerifyEvmReturn.AMP_ERR_DB, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txVerifyEvmReturn.AMP_ERR_DB = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(ampErrDb) return error.\n");
					throw logMessage;
				}

				// IQ Match Phase Error.
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "phaseErr", &l_txVerifyEvmReturn.PHASE_ERR, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txVerifyEvmReturn.PHASE_ERR = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(phaseErr) return error.\n");
					throw logMessage;
				}

				// RMS Phase Noise, mappin the value "RMS Phase Noise" in IQsignal
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "rmsPhaseNoise", &l_txVerifyEvmReturn.PHASE_NOISE_RMS_ALL, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txVerifyEvmReturn.PHASE_NOISE_RMS_ALL = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(rmsPhaseNoise) return error.\n");
					throw logMessage;
				}

				// Symbol Clock Error for 11a/b/g
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "clockErr", &l_txVerifyEvmReturn.SYMBOL_CLK_ERR, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txVerifyEvmReturn.SYMBOL_CLK_ERR = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(clockErr) return error.\n");
					throw logMessage;
				}

				// Datarate
				if (wifiMode==WIFI_11B)
				{
					double dummyAvg = 0, dummyMax = 0, dummyMin = 0;
					//int dummyInt = (int)NA_NUMBER;
					//err = ::LP_IQ2010EXT_GetIntMeasurement( "bitRateInMHz", &dummyInt, 0 );
					err = ::LP_IQ2010EXT_GetDoubleMeasurements( "bitRateInMHz", &dummyAvg, &dummyMin, &dummyMax );
					if ( ERR_OK!=err )
					{
						analysisOK = false;
						l_txVerifyEvmReturn.DATA_RATE = NA_NUMBER;
						WiFiSaveSigFile(sTestDetail);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetIntMeasurement(bitRateInMHz) return error.\n");
						throw logMessage;
					}
					else
					{
						l_txVerifyEvmReturn.DATA_RATE = dummyAvg;
					}
				}
				else
				{
					int dummyInt = (int)NA_NUMBER;
					err = ::LP_IQ2010EXT_GetIntMeasurement( "dataRate", &dummyInt, 0 );
					if ( ERR_OK!=err )
					{
						analysisOK = false;
						l_txVerifyEvmReturn.DATA_RATE = NA_NUMBER;
						WiFiSaveSigFile(sTestDetail);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetIntMeasurement(dataRate) return error.\n");
						throw logMessage;
					}
					else
					{
						l_txVerifyEvmReturn.DATA_RATE = dummyInt;
					}
				}

			}
			else    // 802.11n, includes more than one stream
			{
				// TODO:  Now the IQ2010Ext only support to MCS 0 ~ 7, so index should be always = 0.

				// Number of spatial streams
				double	dummyDoule = NA_NUMBER;
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "rateInfo_spatialStreams", &dummyDoule, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txVerifyEvmReturn.SPATIAL_STREAM = (int)NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(rateInfo_spatialStreams) return error.\n");
					throw logMessage;
				}
				else
				{
					l_txVerifyEvmReturn.SPATIAL_STREAM = (int)dummyDoule;
				}


				for(int i=0;i<l_txVerifyEvmReturn.SPATIAL_STREAM;i++)
				{
					// EVM  ( only can get EVM_ALL, means index always = 0 )
					err = ::LP_IQ2010EXT_GetDoubleMeasurements( "evmAvgAll", &l_txVerifyEvmReturn.EVM_AVG[i], &l_txVerifyEvmReturn.EVM_MIN[i], &l_txVerifyEvmReturn.EVM_MAX[i] );

					if ( ERR_OK!=err )
					{
						analysisOK = false;
						l_txVerifyEvmReturn.EVM_AVG_DB = l_txVerifyEvmReturn.EVM_AVG[i] = l_txVerifyEvmReturn.EVM_MIN[i] = l_txVerifyEvmReturn.EVM_MAX[i] = NA_NUMBER;

						WiFiSaveSigFile(sTestDetail);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(evmAvgAll) return error.\n");
						throw logMessage;
					}
					else
					{
						l_txVerifyEvmReturn.EVM_AVG_DB = l_txVerifyEvmReturn.EVM_AVG[i];

					}

					// Power
					err = ::LP_IQ2010EXT_GetDoubleMeasurements( "rxRmsPowerDb", &l_txVerifyEvmReturn.POWER_AVG[i], &l_txVerifyEvmReturn.POWER_MIN[i], &l_txVerifyEvmReturn.POWER_MAX[i] );
					if ( ERR_OK!=err )
					{
						analysisOK = false;
						l_txVerifyEvmReturn.POWER_AVG_DBM = l_txVerifyEvmReturn.POWER_AVG[i] = l_txVerifyEvmReturn.POWER_MIN[i] = l_txVerifyEvmReturn.POWER_MAX[i] = NA_NUMBER;
						WiFiSaveSigFile(sTestDetail);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(rxRmsPowerDb) return error.\n");
						throw logMessage;
					}
					else
					{
						int antenaOrder = 0;
						err = CheckAntennaOrderByStream(i+1, l_txVerifyEvmParam.TX1, l_txVerifyEvmParam.TX2, l_txVerifyEvmParam.TX3, l_txVerifyEvmParam.TX4, &antenaOrder);
						if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] CheckAntennaOrderByStream() return error.\n");
							throw logMessage;
						}

						l_txVerifyEvmReturn.POWER_AVG[i]	 = l_txVerifyEvmReturn.POWER_AVG[i] + l_txVerifyEvmParam.CABLE_LOSS_DB[antenaOrder-1];
						l_txVerifyEvmReturn.POWER_MIN[i]	 = l_txVerifyEvmReturn.POWER_MIN[i] + l_txVerifyEvmParam.CABLE_LOSS_DB[antenaOrder-1];
						l_txVerifyEvmReturn.POWER_MAX[i]	 = l_txVerifyEvmReturn.POWER_MAX[i] + l_txVerifyEvmParam.CABLE_LOSS_DB[antenaOrder-1];
						l_txVerifyEvmReturn.POWER_AVG_DBM    = l_txVerifyEvmReturn.POWER_AVG[i];
					}

				}

				// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "IQImbal_amplDb", &l_txVerifyEvmReturn.AMP_ERR_DB, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txVerifyEvmReturn.AMP_ERR_DB = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(IQImbal_amplDb) return error.\n");
					throw logMessage;
				}

				// IQ Match Phase Error.
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "IQImbal_phaseDeg", &l_txVerifyEvmReturn.PHASE_ERR, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txVerifyEvmReturn.PHASE_ERR = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(IQImbal_phaseDeg) return error.\n");
					throw logMessage;
				}

				// Frequency Error
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "freqErrorHz", &l_txVerifyEvmReturn.FREQ_ERROR_AVG, &l_txVerifyEvmReturn.FREQ_ERROR_MIN, &l_txVerifyEvmReturn.FREQ_ERROR_MAX );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txVerifyEvmReturn.FREQ_ERROR_AVG = l_txVerifyEvmReturn.FREQ_ERROR_MIN = l_txVerifyEvmReturn.FREQ_ERROR_MAX = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(freqErrorHz) return error.\n");
					throw logMessage;
				}
				else
				{
					// Transfer the Freq Error result unit to ppm
					l_txVerifyEvmReturn.FREQ_ERROR_AVG = l_txVerifyEvmReturn.FREQ_ERROR_AVG/l_txVerifyEvmParam.FREQ_MHZ;
					l_txVerifyEvmReturn.FREQ_ERROR_MAX = l_txVerifyEvmReturn.FREQ_ERROR_MAX/l_txVerifyEvmParam.FREQ_MHZ;
					l_txVerifyEvmReturn.FREQ_ERROR_MIN = l_txVerifyEvmReturn.FREQ_ERROR_MIN/l_txVerifyEvmParam.FREQ_MHZ;
				}

				// RMS Phase Noise, mappin the value "RMS Phase Noise" in IQsignal
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "PhaseNoiseDeg_RmsAll", &l_txVerifyEvmReturn.PHASE_NOISE_RMS_ALL, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txVerifyEvmReturn.PHASE_NOISE_RMS_ALL = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(PhaseNoiseDeg_RmsAll) return error.\n");
					throw logMessage;
				}

				// Datarate
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "rateInfo_dataRateMbps", &l_txVerifyEvmReturn.DATA_RATE, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txVerifyEvmReturn.DATA_RATE = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(rateInfo_dataRateMbps) return error.\n");
					throw logMessage;
				}

				// Symbol Clock Error
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "symClockErrorPpm", &l_txVerifyEvmReturn.SYMBOL_CLK_ERR, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txVerifyEvmReturn.SYMBOL_CLK_ERR = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(symClockErrorPpm) return error.\n");
					throw logMessage;
				}

			}
#pragma endregion

		
// This is a special case, only if "DUT_KEEP_TRANSMIT=0" then must do TX_STOP manually.
		if ( (g_WiFiGlobalSettingParam.DUT_KEEP_TRANSMIT==0)&&(g_vDutTxActived==true) )
		{
			/*-----------*
			 *  Tx Stop  *
			 *-----------*/
			err = ::vDUT_Run(g_WiFi_Dut, "TX_STOP");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_STOP) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
			}
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err && captureOK && analysisOK )
		{
			sprintf_s(l_txVerifyEvmReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txVerifyEvmReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_txVerifyEvmReturn.ERROR_MESSAGE, msg);

		if ( g_vDutTxActived )
		{
			int err = ERR_OK;
			err = vDUT_Run(g_WiFi_Dut, "TX_STOP");
			if( err == ERR_OK )
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
			}
		}
	}
	catch(...)
	{
		ReturnErrorMessage(l_txVerifyEvmReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
		err = -1;

		if ( g_vDutTxActived )
		{
			int err = ERR_OK;
			err = vDUT_Run(g_WiFi_Dut, "TX_STOP");
			if( err == ERR_OK )
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
			}
		}
	}

	// This is a special case, only when some error occur before the TX_STOP.
	// This case will take care by the error handling, but must do TX_STOP manually.
	//if ( g_vDutTxActived )
	//{
	//	vDUT_Run(g_WiFi_Dut, "TX_STOP");
	//}
	//else
	//{
	//	// do nothing
	//}

	


	return err;
}

int InitializeTXVerifyEvmContainers(void)
{
	/*------------------*
	 * Input Parameters  *
	 *------------------*/
	l_txVerifyEvmParamMap.clear();

	WIFI_SETTING_STRUCT setting;

	strcpy_s(l_txVerifyEvmParam.BANDWIDTH, MAX_BUFFER_SIZE, "HT20");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.BANDWIDTH))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyEvmParam.BANDWIDTH;
		setting.unit        = "";
		setting.helpText    = "Channel bandwidth\r\nValid options: HT20 or HT40 or HALF or QUAR";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BANDWIDTH", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_txVerifyEvmParam.DATA_RATE, MAX_BUFFER_SIZE, "OFDM-54");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.DATA_RATE))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyEvmParam.DATA_RATE;
		setting.unit        = "";
		setting.helpText    = "Data rate names:\r\nDSSS-1,DSSS-2,CCK-5_5,CCK-11\r\nOFDM-6,OFDM-9,OFDM-12,OFDM-18,OFDM-24,OFDM-36,OFDM-48,OFDM-54\r\nMCS0, MCS15, etc.";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_txVerifyEvmParam.PREAMBLE, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.PREAMBLE))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyEvmParam.PREAMBLE;
		setting.unit        = "";
		setting.helpText    = "The preamble type of 11B(only), can be SHORT or LONG, Default=LONG.";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PREAMBLE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_txVerifyEvmParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, "MIXED");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.PACKET_FORMAT_11N))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyEvmParam.PACKET_FORMAT_11N;
		setting.unit        = "";
		setting.helpText    = "The packet format of 11N(only), can be MIXED or GREENFIELD, Default=MIXED.";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT_11N", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_txVerifyEvmParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.GUARD_INTERVAL_11N))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyEvmParam.GUARD_INTERVAL_11N;
		setting.unit        = "";
		setting.helpText    = "The guard interval format of 11N(only), can be LONG or SHORT, Default=LONG.";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL_11N", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyEvmParam.FREQ_MHZ = 2412;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyEvmParam.FREQ_MHZ))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmParam.FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "Channel center frequency in MHz";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_MHZ", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyEvmParam.SAMPLING_TIME_US = 0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEvmParam.SAMPLING_TIME_US))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmParam.SAMPLING_TIME_US;
		setting.unit        = "us";
		setting.helpText    = "Capture time in micro-seconds";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("SAMPLING_TIME_US", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyEvmParam.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_txVerifyEvmParam.TX_POWER_DBM = 15.0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEvmParam.TX_POWER_DBM))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmParam.TX_POWER_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected power level at DUT antenna port (For AR6004, set -99 for default target power.)";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyEvmParam.TX1 = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyEvmParam.TX1))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmParam.TX1;
		setting.unit        = "";
		setting.helpText    = "DUT TX path 1 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX1", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyEvmParam.TX2 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyEvmParam.TX2))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmParam.TX2;
		setting.unit        = "";
		setting.helpText    = "DUT TX path 2 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX2", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyEvmParam.TX3 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyEvmParam.TX3))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmParam.TX3;
		setting.unit        = "";
		setting.helpText    = "DUT TX path 3 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX3", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyEvmParam.TX4 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyEvmParam.TX4))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmParam.TX4;
		setting.unit        = "";
		setting.helpText    = "DUT TX path 4 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX4", setting) );
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
	l_txVerifyEvmReturnMap.clear();

	// -cfy@sunnyvale, 2012/3/13-
	l_txVerifyEvmReturn.TX_POWER_DBM = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEvmReturn.TX_POWER_DBM))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmReturn.TX_POWER_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected power level at DUT antenna port";
		l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	/* <><~~ */

	l_txVerifyEvmReturn.EVM_AVG_DB = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_AVG_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmReturn.EVM_AVG_DB;
		setting.unit        = "dB";
		setting.helpText    = "EVM average over captured packets and all data streams(MIMO)";
		l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("EVM_AVG_DB", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyEvmReturn.EVM_AVG_PERCENT = 0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_AVG_PERCENT))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmReturn.EVM_AVG_PERCENT;
		setting.unit        = "%";
		setting.helpText    = "EVM average in percentage over captured packets and all data streams(MIMO)";
		l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("EVM_AVG_PERCENT", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmReturn.EVM_AVG[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_AVG[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.EVM_AVG[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "EVM_AVG_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Average EVM over captured packets on individual streams(MIMO)";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}


	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmReturn.EVM_AVG_STREAM_PERCENT[i] = 0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_AVG_STREAM_PERCENT[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.EVM_AVG_STREAM_PERCENT[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "EVM_AVG_STREAM_PERCENT_%d", i+1);
			setting.unit        = "%";
			setting.helpText    = "Average EVM in percentage over captured packets on individual streams(MIMO)";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}


	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmReturn.EVM_MAX[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_MAX[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.EVM_MAX[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "EVM_MAX_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Maximum EVM over captured packets on individual streams(MIMO)";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}


	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmReturn.EVM_MAX_STREAM_PERCENT[i] = 0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_MAX_STREAM_PERCENT[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.EVM_MAX_STREAM_PERCENT[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "EVM_MAX_STREAM_PERCENT_%d", i+1);
			setting.unit        = "%";
			setting.helpText    = "Maximum EVM in percentage over captured packets on individual streams(MIMO)";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}


	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmReturn.EVM_MIN[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_MIN[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.EVM_MIN[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "EVM_MIN_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Minimum EVM over captured packets on individual streams(MIMO)";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmReturn.EVM_MIN_STREAM_PERCENT[i] = 0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_MIN_STREAM_PERCENT[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.EVM_MIN_STREAM_PERCENT[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "EVM_MIN_STREAM_PERCENT_%d", i+1);
			setting.unit        = "%";
			setting.helpText    = "Minimum EVM in percentage over captured packets on individual streams(MIMO)";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_txVerifyEvmReturn.EVM_PK_DB = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_PK_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmReturn.EVM_PK_DB;
		setting.unit        = "dB";
		setting.helpText    = "(11b only)Peak EVM over captured packets.";
		l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("EVM_PK_DB", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyEvmReturn.EVM_PK_PERCENT = 0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_PK_PERCENT))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmReturn.EVM_PK_PERCENT;
		setting.unit        = "%";
		setting.helpText    = "(11b only)Peak EVM in percentage over captured packets.";
		l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("EVM_PK_PERCENT", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_txVerifyEvmReturn.AMP_ERR_DB = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEvmReturn.AMP_ERR_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmReturn.AMP_ERR_DB;
		setting.unit        = "dB";
		setting.helpText    = "IQ Match Amplitude Error in dB.";
		l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("AMP_ERR_DB", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for(int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmReturn.AMP_ERR_DB_STREAM[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.AMP_ERR_DB_STREAM[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.AMP_ERR_DB_STREAM[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "AMP_ERR_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "IQ Match Amplitude Error in dB.(per stream)";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_txVerifyEvmReturn.PHASE_ERR = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEvmReturn.PHASE_ERR))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmReturn.PHASE_ERR;
		setting.unit        = "Degree";
		setting.helpText    = "IQ Match Phase Error.";
		l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("PHASE_ERR", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyEvmReturn.SYMBOL_CLK_ERR = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEvmReturn.SYMBOL_CLK_ERR))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmReturn.SYMBOL_CLK_ERR;
		setting.unit        = "ppm";
		setting.helpText    = "Symbol Clock Error";
		l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SYMBOL_CLK_ERR", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for(int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmReturn.PHASE_ERR_STREAM[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.PHASE_ERR_STREAM[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.PHASE_ERR_STREAM[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "PHASE_ERR_%d", i+1);
			setting.unit        = "Degree";
			setting.helpText    = "IQ Match Phase Error.(per stream)";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	//PHASE_NOISE_RMS_ALL
	l_txVerifyEvmReturn.PHASE_NOISE_RMS_ALL = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEvmReturn.PHASE_NOISE_RMS_ALL))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmReturn.PHASE_NOISE_RMS_ALL;
		setting.unit        = "Degree";
		setting.helpText    = "Frequency RMS Phase Noise.";
		l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("PHASE_NOISE_RMS_ALL", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyEvmReturn.POWER_AVG_DBM = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEvmReturn.POWER_AVG_DBM))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmReturn.POWER_AVG_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Average Power over captured packets";
		l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_AVG_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmReturn.POWER_AVG[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.POWER_AVG[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.POWER_AVG[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "POWER_AVG_%d", i+1);
			setting.unit        = "dBm";
			setting.helpText    = "Average Power over captured packets on individual streams";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmReturn.POWER_MAX[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.POWER_MAX[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.POWER_MAX[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "POWER_MAX_%d", i+1);
			setting.unit        = "dBm";
			setting.helpText    = "Maximum Power over captured packets on individual streams";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmReturn.POWER_MIN[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.POWER_MIN[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.POWER_MIN[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "POWER_MIN_%d", i+1);
			setting.unit        = "dBm";
			setting.helpText    = "Minimum Power over captured packets on individual streams";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_txVerifyEvmReturn.FREQ_ERROR_AVG = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEvmReturn.FREQ_ERROR_AVG))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmReturn.FREQ_ERROR_AVG;
		setting.unit        = "ppm";
		setting.helpText    = "Average frequency error over captured packets";
		l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_ERROR_AVG", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyEvmReturn.FREQ_ERROR_MAX = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEvmReturn.FREQ_ERROR_MAX))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmReturn.FREQ_ERROR_MAX;
		setting.unit        = "ppm";
		setting.helpText    = "Maximum frequency error over captured packets";
		l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_ERROR_MAX", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyEvmReturn.FREQ_ERROR_MIN = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEvmReturn.FREQ_ERROR_MIN))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmReturn.FREQ_ERROR_MIN;
		setting.unit        = "ppm";
		setting.helpText    = "Minimum frequency error over captured packets";
		l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_ERROR_MIN", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyEvmReturn.DATA_RATE = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEvmReturn.DATA_RATE))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmReturn.DATA_RATE;
		setting.unit        = "Mbps";
		setting.helpText    = "Data rate in Mbps, reported by IQAPI.";
		l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyEvmReturn.SPATIAL_STREAM = (int)NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyEvmReturn.SPATIAL_STREAM))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmReturn.SPATIAL_STREAM;
		setting.unit        = "";
		setting.helpText    = "Number of spatial stream, reported by IQAPI.";
		l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SPATIAL_STREAM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmReturn.CABLE_LOSS_DB[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_txVerifyEvmReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyEvmReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	return 0;
}

double CheckSamplingTime(int wifiMode, char *preamble11B, char *dataRate, char *packetFormat11N)
{
	double samplingTimeUs = 300;

	if ( wifiMode==WIFI_11B )
	{
		if (0==strcmp(preamble11B, "LONG"))
		{
			samplingTimeUs = g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11B_L_US;
		}
		else	// 802.11b preamble = SHORT
		{
			if (0==strcmp(dataRate, "DSSS-1"))	//  for 1 Mbps (Long Preamble only)
			{
				samplingTimeUs = g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11B_L_US;
			}
			else	// Using default sampling time (short preamble)
			{
				samplingTimeUs = g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11B_S_US;
			}
		}
	}
	else if ( wifiMode==WIFI_11AG )
	{
		if (0==strcmp(l_txVerifyEvmParam.BANDWIDTH,"HALF")) {
			samplingTimeUs = g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11P_HALF_US;
		} else if  (0==strcmp(l_txVerifyEvmParam.BANDWIDTH,"QUAR")) {
			samplingTimeUs = g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11P_QUAR_US;
		} else {
			samplingTimeUs = g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11AG_US;
		}
	}
	else	// 802.11n
	{
		if (0==strcmp(packetFormat11N, "GREENFIELD"))
		{
			samplingTimeUs = g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11N_GREENFIELD_US;
		}
		else	// 802.11n Mixed Mode
		{
			samplingTimeUs = g_WiFiGlobalSettingParam.EVM_CAPTURE_TIME_11N_MIXED_US;
		}
	}

	return samplingTimeUs;
}

//-------------------------------------------------------------------------------------
// This is a function for checking the input parameters before the test.
//
//-------------------------------------------------------------------------------------
int CheckTxEvmParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *samplingTimeUs, double *cableLossDb, double *peakToAvgRatio, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		// Checking the Input Parameters
		err = TM_WiFiConvertFrequencyToChannel(l_txVerifyEvmParam.FREQ_MHZ, channel);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_txVerifyEvmParam.FREQ_MHZ);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}

		err = TM_WiFiConvertDataRateNameToIndex(l_txVerifyEvmParam.DATA_RATE, &dummyInt);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_txVerifyEvmParam.DATA_RATE);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}

		if ( 0!=strcmp(l_txVerifyEvmParam.BANDWIDTH, "HT20") && 0!=strcmp(l_txVerifyEvmParam.BANDWIDTH, "HT40")
				&& 0!=strcmp(l_txVerifyEvmParam.BANDWIDTH, "HALF") && 0!=strcmp(l_txVerifyEvmParam.BANDWIDTH, "QUAR"))
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown BANDWIDTH, WiFi bandwidth %s not supported.\n", l_txVerifyEvmParam.BANDWIDTH);
			throw logMessage;
		}
		else
		{
			//do nothing
		}
		if ( 0!=strcmp(l_txVerifyEvmParam.PREAMBLE, "SHORT") && 0!=strcmp(l_txVerifyEvmParam.PREAMBLE, "LONG") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_txVerifyEvmParam.PREAMBLE);
			throw logMessage;
		}
		else
		{
			//do nothing
		}
		if ( 0!=strcmp(l_txVerifyEvmParam.PACKET_FORMAT_11N, "MIXED") && 0!=strcmp(l_txVerifyEvmParam.PACKET_FORMAT_11N, "GREENFIELD") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PACKET_FORMAT_11N, WiFi 11n packet format %s not supported.\n", l_txVerifyEvmParam.PACKET_FORMAT_11N);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txVerifyEvmParam.GUARD_INTERVAL_11N, "LONG") && 0!=strcmp(l_txVerifyEvmParam.GUARD_INTERVAL_11N, "SHORT") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown GUARD_INTERVAL_11N, WiFi 11n guard interval format %s not supported.\n", l_txVerifyEvmParam.GUARD_INTERVAL_11N);
			throw logMessage;
		}
		else
		{
			//do nothing
		}

		// Convert parameter
		err = WiFiTestMode(l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH, wifiMode, wifiStreamNum);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Convert WiFi test mode function failed.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] WiFiTestMode() return OK.\n");
		}

		// Check path loss (by ant and freq)
		err = CheckPathLossTableExt( g_WiFi_Test_ID,
				l_txVerifyEvmParam.FREQ_MHZ,
				l_txVerifyEvmParam.TX1,
				l_txVerifyEvmParam.TX2,
				l_txVerifyEvmParam.TX3,
				l_txVerifyEvmParam.TX4,
				l_txVerifyEvmParam.CABLE_LOSS_DB,
				l_txVerifyEvmReturn.CABLE_LOSS_DB,
				cableLossDb,
				TX_TABLE
				);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get TX_CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] CheckPathLossTableExt(TX_TABLE) return OK.\n");
		}

		double timeUs = CheckSamplingTime(*wifiMode, l_txVerifyEvmParam.PREAMBLE, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.PACKET_FORMAT_11N);
		// Check sampling time
		if (0==l_txVerifyEvmParam.SAMPLING_TIME_US)
		{
			*samplingTimeUs = timeUs;
		}
		else	// SAMPLING_TIME_US != 0
		{
			if ( l_txVerifyEvmParam.SAMPLING_TIME_US>=timeUs )
			{
				*samplingTimeUs = l_txVerifyEvmParam.SAMPLING_TIME_US;
			}
			else
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] The SAMPLING_TIME_US of EVM is too short, IQlite recommend %.1f us", timeUs );
				throw logMessage;
			}
		}

		if ( *wifiMode==WIFI_11B )
		{
			if ( *channel==14 )
			{
				*peakToAvgRatio = g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_CH14;    // CCK (Channel 14)
			}
			else
			{
				*peakToAvgRatio = g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_11M;    // CCK
			}
		}
		else
		{
			*peakToAvgRatio = g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_54M;   // OFDM
		}

		// Check Dut configuration changed or not
		if (  g_WiFiGlobalSettingParam.DUT_KEEP_TRANSMIT==0	||		// means need to configure DUT everytime, thus set g_dutConfigChanged = true , always.
				0!=strcmp(l_txVerifyEvmParam.BANDWIDTH, g_RecordedParam.BANDWIDTH) ||
				0!=strcmp(l_txVerifyEvmParam.DATA_RATE, g_RecordedParam.DATA_RATE) ||
				0!=strcmp(l_txVerifyEvmParam.PREAMBLE,  g_RecordedParam.PREAMBLE) ||
				0!=strcmp(l_txVerifyEvmParam.PACKET_FORMAT_11N, g_RecordedParam.PACKET_FORMAT_11N) ||
				0!=strcmp(l_txVerifyEvmParam.GUARD_INTERVAL_11N, g_RecordedParam.GUARD_INTERVAL_11N) ||
				l_txVerifyEvmParam.CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
				l_txVerifyEvmParam.CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
				l_txVerifyEvmParam.CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
				l_txVerifyEvmParam.CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
				l_txVerifyEvmParam.FREQ_MHZ!=g_RecordedParam.FREQ_MHZ ||
				l_txVerifyEvmParam.TX_POWER_DBM!=g_RecordedParam.POWER_DBM ||
				l_txVerifyEvmParam.TX1!=g_RecordedParam.ANT1 ||
				l_txVerifyEvmParam.TX2!=g_RecordedParam.ANT2 ||
				l_txVerifyEvmParam.TX3!=g_RecordedParam.ANT3 ||
				l_txVerifyEvmParam.TX4!=g_RecordedParam.ANT4)
		{
			g_dutConfigChanged = true;
		}
		else
		{
			g_dutConfigChanged = false;
		}

		// Save the current setup
		g_RecordedParam.ANT1					= l_txVerifyEvmParam.TX1;
		g_RecordedParam.ANT2					= l_txVerifyEvmParam.TX2;
		g_RecordedParam.ANT3					= l_txVerifyEvmParam.TX3;
		g_RecordedParam.ANT4					= l_txVerifyEvmParam.TX4;
		g_RecordedParam.CABLE_LOSS_DB[0]		= l_txVerifyEvmParam.CABLE_LOSS_DB[0];
		g_RecordedParam.CABLE_LOSS_DB[1]		= l_txVerifyEvmParam.CABLE_LOSS_DB[1];
		g_RecordedParam.CABLE_LOSS_DB[2]		= l_txVerifyEvmParam.CABLE_LOSS_DB[2];
		g_RecordedParam.CABLE_LOSS_DB[3]		= l_txVerifyEvmParam.CABLE_LOSS_DB[3];
		g_RecordedParam.FREQ_MHZ				= l_txVerifyEvmParam.FREQ_MHZ;
		g_RecordedParam.POWER_DBM				= l_txVerifyEvmParam.TX_POWER_DBM;

		sprintf_s(g_RecordedParam.BANDWIDTH, MAX_BUFFER_SIZE, l_txVerifyEvmParam.BANDWIDTH);
		sprintf_s(g_RecordedParam.DATA_RATE, MAX_BUFFER_SIZE, l_txVerifyEvmParam.DATA_RATE);
		sprintf_s(g_RecordedParam.PREAMBLE,  MAX_BUFFER_SIZE, l_txVerifyEvmParam.PREAMBLE);
		sprintf_s(g_RecordedParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, l_txVerifyEvmParam.PACKET_FORMAT_11N);
		sprintf_s(g_RecordedParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, l_txVerifyEvmParam.GUARD_INTERVAL_11N);


		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] CheckTxEvmParameters() Confirmed.\n");
	}
	catch(char *msg)
	{
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, msg);
	}
	catch(...)
	{
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] Unknown Error!\n");
		err = -1;
	}


	return err;
}
