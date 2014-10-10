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
map<string, WIFI_SETTING_STRUCT> l_txVerifyPowerParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_txVerifyPowerReturnMap;

struct tagParam
{
	// Mandatory Parameters
	int    FREQ_MHZ;                                /*! The center frequency (MHz). */
	char   BANDWIDTH[MAX_BUFFER_SIZE];              /*! The RF bandwidth to verify POWER. */
	char   DATA_RATE[MAX_BUFFER_SIZE];				/*! The data rate to verify POWER. */
	char   PREAMBLE[MAX_BUFFER_SIZE];               /*! The preamble type of 11B(only). */
	char   PACKET_FORMAT_11N[MAX_BUFFER_SIZE];      /*! The packet format of 11N(only). */
	char   GUARD_INTERVAL_11N[MAX_BUFFER_SIZE];     /*! The guard interval format of 11N(only). */
	double TX_POWER_DBM;                            /*! The output power to verify Power. */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
	double SAMPLING_TIME_US;                        /*! The sampling time to verify Power. */
	//double T_INTERVAL;                              /*! This field is used to specify the interval that is used to determine if power is present or not. */
	//double MAX_POW_DIFF_DB;                         /*! This field is used to specify the maximum power difference between packets that are expected to be detected. */

	// DUT Parameters
	int    TX1;                                     /*!< DUT TX1 on/off. Default=1(on) */
	int    TX2;                                     /*!< DUT TX2 on/off. Default=0(off) */
	int    TX3;                                     /*!< DUT TX3 on/off. Default=0(off) */
	int    TX4;                                     /*!< DUT TX4 on/off. Default=0(off) */
} l_txVerifyPowerParam;

struct tagReturn
{
	double TX_POWER_DBM;							/*!< TX power dBm setting */
	// POWER Test Result
	double POWER_AVERAGE_DBM;                       /*!< (Average) Average power in dBm. */
	double POWER_AVERAGE_MAX_DBM;                   /*!< (Maximum) Average power in dBm. */
	double POWER_AVERAGE_MIN_DBM;                   /*!< (Minimum) Average power in dBm. */

	double POWER_PEAK_DBM;                          /*!< (Average) Peak power in dBm. */
	double POWER_PEAK_MAX_DBM;					    /*!< (Maximum) Peak power in dBm. */
	double POWER_PEAK_MIN_DBM;                      /*!< (Minimum) Peak power in dBm. */

	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
	char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txVerifyPowerReturn;
#pragma endregion

void ClearTxPowerReturn(void)
{
	l_txVerifyPowerParamMap.clear();
	l_txVerifyPowerReturnMap.clear();
}

#ifndef WIN32
int initTXVerifyPowerContainers = InitializeTXVerifyPowerContainers();
#endif

// These global variables/functions only for WiFi_TX_Verify_Power.cpp
int CheckTxPowerParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *samplingTimeUs, double *cableLossDb, double *peakToAvgRatio, char* errorMsg );
int WiFi_TX_Verify_Power_2010ExtTest(void);

//! WiFi TX Verify POWER
/*!
 * Input Parameters
 *
 *  - Mandatory
 *      -# FREQ_MHZ (double): The center frequency (MHz)
 *      -# DATA_RATE (string): The data rate to verify POWER
 *      -# TX_POWER (double): The power (dBm) DUT is going to transmit at the antenna port
 *
 * Return Values
 *      -# A string for error message
 *
 * \return 0 No error occurred
 * \return -1 Error(s) occurred.  Please see the returned error message for details
 */


WIFI_TEST_API int WiFi_TX_Verify_Power(void)
{
	int    err = ERR_OK;

	bool   analysisOK = false, captureOK = false, enableMultiCapture = false;
	int    avgIteration = 0;
	int    channel = 0, HT40ModeOn = 0;
	int    dummyValue = 0;
	int    wifiMode = 0, wifiStreamNum = 0;
	double samplingTimeUs = 0, peakToAvgRatio = 0, cableLossDb = 0;
	double chainGainDb = 0.0;
	char   vErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};


	/*---------------------------------------*
	 * Clear Return Parameters and Container *
	 *---------------------------------------*/
	ClearReturnParameters(l_txVerifyPowerReturnMap);

	/*------------------------*
	 * Respond to QUERY_INPUT *
	 *------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_txVerifyPowerParamMap);
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
		RespondToQueryReturn(l_txVerifyPowerReturnMap);
		return err;
	}
	else
	{
		// do nothing
	}

	/*----------------------------------------------------------------------------------------
	 * Declare vectors for storing test results: vector< double >
	 *-----------------------------------------------------------------------------------------*/
	vector< double >    powerAvEachBurst(g_WiFiGlobalSettingParam.PM_AVERAGE);
	vector< double >    powerPkEachBurst(g_WiFiGlobalSettingParam.PM_AVERAGE);

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
		err = GetInputParameters(l_txVerifyPowerParamMap);
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
		* needed to split WIFI_TX_Verify_Power into two to	* 
		* avoid a stack check issue.							*
		* 								*
		*------------------------------------------------------*/
		if ( g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE )
		{
			return WiFi_TX_Verify_Power_2010ExtTest();
		}
		// Error return of this function is irrelevant
		CheckDutTransmitStatus();

#pragma region Prepare input parameters

		err = CheckTxPowerParameters( &channel, &wifiMode, &wifiStreamNum, &samplingTimeUs, &cableLossDb, &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters CheckTxPowerParameters() return OK.\n");
		}

		// Modified /* -cfy@sunnyvale, 2012/3/13- */
		// Considering stream number while calculating chain gain for composite measurement so as to set the proper Rx amplitude in tester
		int antennaNum = l_txVerifyPowerParam.TX1 + l_txVerifyPowerParam.TX2 + l_txVerifyPowerParam.TX3 + l_txVerifyPowerParam.TX4;
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
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_txVerifyPowerParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_txVerifyPowerParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_txVerifyPowerParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",      l_txVerifyPowerParam.FREQ_MHZ);
		}

		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",		  l_txVerifyPowerParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_txVerifyPowerParam.PACKET_FORMAT_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N", l_txVerifyPowerParam.GUARD_INTERVAL_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",		  l_txVerifyPowerParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",		  l_txVerifyPowerParam.BANDWIDTH);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		  HT40ModeOn);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",				  l_txVerifyPowerParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",				  l_txVerifyPowerParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",				  l_txVerifyPowerParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",				  l_txVerifyPowerParam.TX4);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "SAMPLING_TIME_US",  samplingTimeUs); // -cfy@sunnyvale, 2012/3/13-

		// -cfy@sunnyvale, 2012/3/13-
		// Added GET_TX_TARGET_POWER feature for AR6004. When TX_POWER_DBM is set to -99, default target power will be applied.
		if ( (l_txVerifyPowerParam.TX_POWER_DBM == TX_TARGET_POWER_FLAG) )
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
				l_txVerifyPowerParam.TX_POWER_DBM = tx_target_power;
				g_last_TxPower_dBm_Record = l_txVerifyPowerParam.TX_POWER_DBM;
			}
			else
			{
				l_txVerifyPowerParam.TX_POWER_DBM = g_last_TxPower_dBm_Record;
			}
		}
		else
		{
			// do nothing
		}
		/* <><~~ */

		vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM",	  l_txVerifyPowerParam.TX_POWER_DBM);
		TM_SetDoubleParameter(g_WiFi_Test_ID, "TX_POWER_DBM", l_txVerifyPowerParam.TX_POWER_DBM); // -cfy@sunnyvale, 2012/3/13-

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

			err = ::LP_SetVsa(  l_txVerifyPowerParam.FREQ_MHZ*1e6,
					l_txVerifyPowerParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGainDb,
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

			/*------------------------------*
			 * Start while loop for average *
			 *------------------------------*/
			avgIteration = 0;
			while ( avgIteration<g_WiFiGlobalSettingParam.PM_AVERAGE )
			{
				analysisOK = false;
				captureOK  = false;

				/*----------------------------*
				 * Perform Normal VSA capture *
				 *----------------------------*/
				double sampleFreqHz = 80e6;
				HT40ModeOn = 0;
				err = ::LP_VsaDataCapture( samplingTimeUs/1000000, g_WiFiGlobalSettingParam.VSA_TRIGGER_TYPE, sampleFreqHz, HT40ModeOn );
				if( ERR_OK!=err )	// capture is failed
				{
					double quickPower = NA_NUMBER;
					err = QuickCaptureRetry( l_txVerifyPowerParam.FREQ_MHZ, samplingTimeUs, g_WiFiGlobalSettingParam.VSA_TRIGGER_TYPE, HT40ModeOn, &quickPower, logMessage);
					if (ERR_OK!=err)	// QuickCaptureRetry() is failed
					{
						// Fail Capture
						if ( quickPower!=NA_NUMBER )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to capture WiFi signal at %d MHz.\nThe DUT power (without path loss) = %.1f dBm and QuickCaptureRetry() return error.\n", l_txVerifyPowerParam.FREQ_MHZ, quickPower);
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to capture WiFi signal at %d MHz, QuickCaptureRetry() return error.\n", l_txVerifyPowerParam.FREQ_MHZ);
						}
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] The QuickCaptureRetry() at %d MHz return OK.\n", l_txVerifyPowerParam.FREQ_MHZ);
					}
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_VsaDataCapture() at %d MHz return OK.\n", l_txVerifyPowerParam.FREQ_MHZ);
				}

#pragma endregion
				/*--------------*
				 *  Capture OK  *
				 *--------------*/
				captureOK = true;
				if (1==g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
				{
					// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_SaveAlways", l_txVerifyPowerParam.FREQ_MHZ, l_txVerifyPowerParam.DATA_RATE, l_txVerifyPowerParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
				}
				else
				{
					// do nothing
				}

				/*------------------*
				 *  Power Analysis  *
				 *------------------*/
				double dummy_T_INTERVAL      = 3.2;
				double dummy_MAX_POW_DIFF_DB = 15.0;
				err = ::LP_AnalyzePower( dummy_T_INTERVAL/1000000, dummy_MAX_POW_DIFF_DB );
				if ( ERR_OK!=err )
				{
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_Analysis_Failed", l_txVerifyPowerParam.FREQ_MHZ, l_txVerifyPowerParam.DATA_RATE, l_txVerifyPowerParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_AnalyzePower() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_AnalyzePower() return OK.\n");
				}

#pragma region Retrieve analysis Results
				/*-----------------------------*
				 *  Retrieve analysis Results  *
				 *-----------------------------*/
				avgIteration++;
				analysisOK = true;

				// P_av_no_gap_all_dBm
				powerAvEachBurst[avgIteration-1] = ::LP_GetScalarMeasurement("P_av_no_gap_all_dBm", 0);
				if ( -99.00 >= powerAvEachBurst[avgIteration-1] )
				{
					analysisOK = false;
					powerAvEachBurst[avgIteration-1] = NA_NUMBER;
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_Analysis_Failed", l_txVerifyPowerParam.FREQ_MHZ, l_txVerifyPowerParam.DATA_RATE, l_txVerifyPowerParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(P_av_no_gap_all_dBm) return error.\n");
					throw logMessage;
				}
				else
				{
					// Since the limitation, we assume that all path loss value are very close.
					int antenaOrder = 0;
					err = CheckAntennaOrderByStream(1, l_txVerifyPowerParam.TX1, l_txVerifyPowerParam.TX2, l_txVerifyPowerParam.TX3, l_txVerifyPowerParam.TX4, &antenaOrder);
					if ( ERR_OK!=err )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] CheckAntennaOrderByStream() return error.\n");
						throw logMessage;
					}

					powerAvEachBurst[avgIteration-1] = powerAvEachBurst[avgIteration-1] + l_txVerifyPowerParam.CABLE_LOSS_DB[antenaOrder-1];
				}

				// P_pk_each_burst_dBm
				powerPkEachBurst[avgIteration-1] = ::LP_GetScalarMeasurement("P_pk_each_burst_dBm",0);
				if ( -99.00 >= powerPkEachBurst[avgIteration-1] )
				{
					analysisOK = false;
					powerPkEachBurst[avgIteration-1] = NA_NUMBER;
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_Analysis_Failed", l_txVerifyPowerParam.FREQ_MHZ, l_txVerifyPowerParam.DATA_RATE, l_txVerifyPowerParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(P_pk_each_burst_dBm) return error.\n");
					throw logMessage;
				}
				else
				{
					// Since the limitation, we assume that all path loss value are very close.
					int antenaOrder = 0;
					err = CheckAntennaOrderByStream(1, l_txVerifyPowerParam.TX1, l_txVerifyPowerParam.TX2, l_txVerifyPowerParam.TX3, l_txVerifyPowerParam.TX4, &antenaOrder);
					if ( ERR_OK!=err )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] CheckAntennaOrderByStream() return error.\n");
						throw logMessage;
					}

					powerPkEachBurst[avgIteration-1] = powerPkEachBurst[avgIteration-1] + l_txVerifyPowerParam.CABLE_LOSS_DB[antenaOrder-1];
				}
#pragma endregion
			}   // End - avgIteration

#pragma region Averaging and Saving Test Result
			/*----------------------------------*
			 * Averaging and Saving Test Result *
			 *----------------------------------*/
			if ( (ERR_OK==err) && captureOK && analysisOK )
			{
				// Average Power test result
				::AverageTestResult(&powerAvEachBurst[0], avgIteration, LOG_10, l_txVerifyPowerReturn.POWER_AVERAGE_DBM, l_txVerifyPowerReturn.POWER_AVERAGE_MAX_DBM, l_txVerifyPowerReturn.POWER_AVERAGE_MIN_DBM);

				// Peak Power test result
				::AverageTestResult(&powerPkEachBurst[0], avgIteration, LOG_10, l_txVerifyPowerReturn.POWER_PEAK_DBM, l_txVerifyPowerReturn.POWER_PEAK_MAX_DBM, l_txVerifyPowerReturn.POWER_PEAK_MIN_DBM);

				// Added GET_TX_TARGET_POWER feature for AR6004. When TX_POWER_DBM is set to -99, default target power will be applied.
				// TBD: 2010 not support for this
				l_txVerifyPowerReturn.TX_POWER_DBM = l_txVerifyPowerParam.TX_POWER_DBM; // -cfy@sunnyvale, 2012/3/13-

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
			sprintf_s(l_txVerifyPowerReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txVerifyPowerReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_txVerifyPowerReturn.ERROR_MESSAGE, msg);

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
		ReturnErrorMessage(l_txVerifyPowerReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
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
	powerAvEachBurst.clear();
	powerPkEachBurst.clear();

	return err;
}

int WiFi_TX_Verify_Power_2010ExtTest(void)
{
	int    err = ERR_OK;

	bool   analysisOK = false, captureOK = false, enableMultiCapture = false;
	int    avgIteration = 0;
	int    channel = 0, HT40ModeOn = 0;
	int    dummyValue = 0;
	int    wifiMode = 0, wifiStreamNum = 0;
	double samplingTimeUs = 0, peakToAvgRatio = 0, cableLossDb = 0;
	double chainGainDb = 0.0;
	char   vErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};
	try
	{
		// Error return of this function is irrelevant
		CheckDutTransmitStatus();

#pragma region Prepare input parameters

		err = CheckTxPowerParameters( &channel, &wifiMode, &wifiStreamNum, &samplingTimeUs, &cableLossDb, &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters CheckTxPowerParameters() return OK.\n");
		}

		// Modified /* -cfy@sunnyvale, 2012/3/13- */
		// Considering stream number while calculating chain gain for composite measurement so as to set the proper Rx amplitude in tester
		int antennaNum = l_txVerifyPowerParam.TX1 + l_txVerifyPowerParam.TX2 + l_txVerifyPowerParam.TX3 + l_txVerifyPowerParam.TX4;
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
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_txVerifyPowerParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_txVerifyPowerParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_txVerifyPowerParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",      l_txVerifyPowerParam.FREQ_MHZ);
		}

		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",		  l_txVerifyPowerParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_txVerifyPowerParam.PACKET_FORMAT_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N", l_txVerifyPowerParam.GUARD_INTERVAL_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",		  l_txVerifyPowerParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",		  l_txVerifyPowerParam.BANDWIDTH);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		  HT40ModeOn);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",				  l_txVerifyPowerParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",				  l_txVerifyPowerParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",				  l_txVerifyPowerParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",				  l_txVerifyPowerParam.TX4);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "SAMPLING_TIME_US",  samplingTimeUs); // -cfy@sunnyvale, 2012/3/13-

		// -cfy@sunnyvale, 2012/3/13-
		// Added GET_TX_TARGET_POWER feature for AR6004. When TX_POWER_DBM is set to -99, default target power will be applied.
		if ( (l_txVerifyPowerParam.TX_POWER_DBM == TX_TARGET_POWER_FLAG) )
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
				l_txVerifyPowerParam.TX_POWER_DBM = tx_target_power;
				g_last_TxPower_dBm_Record = l_txVerifyPowerParam.TX_POWER_DBM;
			}
			else
			{
				l_txVerifyPowerParam.TX_POWER_DBM = g_last_TxPower_dBm_Record;
			}
		}
		else
		{
			// do nothing
		}
		/* <><~~ */

		vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM",	  l_txVerifyPowerParam.TX_POWER_DBM);
		TM_SetDoubleParameter(g_WiFi_Test_ID, "TX_POWER_DBM", l_txVerifyPowerParam.TX_POWER_DBM); // -cfy@sunnyvale, 2012/3/13-

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
				err = ::LP_IQ2010EXT_InitiateMultiCapture(  l_txVerifyPowerParam.DATA_RATE,
						l_txVerifyPowerParam.FREQ_MHZ,
						l_txVerifyPowerParam.TX_POWER_DBM-cableLossDb+chainGainDb,
						(int)samplingTimeUs,
						g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
						g_WiFiGlobalSettingParam.PM_AVERAGE,
						g_WiFiGlobalSettingParam.VSA_PORT,
						l_txVerifyPowerParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
						);
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to new initiate MultiCapture at %d MHz.\n", l_txVerifyPowerParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_InitiateMultiCapture() at %d MHz return OK.\n", l_txVerifyPowerParam.FREQ_MHZ);
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
				err = ::LP_IQ2010EXT_InitiateMultiCapture(  l_txVerifyPowerParam.DATA_RATE,
						l_txVerifyPowerParam.FREQ_MHZ,
						l_txVerifyPowerParam.TX_POWER_DBM-cableLossDb+chainGainDb,
						(int)samplingTimeUs,
						g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
						g_WiFiGlobalSettingParam.PM_AVERAGE,
						g_WiFiGlobalSettingParam.VSA_PORT,
						l_txVerifyPowerParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
						);
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to new initiate MultiCapture at %d MHz.\n", l_txVerifyPowerParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_InitiateMultiCapture() at %d MHz return OK.\n", l_txVerifyPowerParam.FREQ_MHZ);
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
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_SaveAlways", l_txVerifyPowerParam.FREQ_MHZ, l_txVerifyPowerParam.DATA_RATE, l_txVerifyPowerParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
			}
			else
			{
				// do nothing
			}

			/*----------------------------*
			 *  IQ2010Ext Power Analysis  *
			 *----------------------------*/
			err = ::LP_IQ2010EXT_AnalyzePower();
			if ( ERR_OK!=err )
			{
				// Fail Analysis, thus save capture (Signal File) for debug
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_Analysis_Failed", l_txVerifyPowerParam.FREQ_MHZ, l_txVerifyPowerParam.DATA_RATE, l_txVerifyPowerParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_AnalyzePower(%d) return error.\n", l_txVerifyPowerParam.FREQ_MHZ);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_AnalyzePower() return OK.\n");
			}

#pragma region Retrieve analysis Results
			/*---------------------------------------*
			 *  Retrieve IQ2010EXT analysis Results  *
			 *---------------------------------------*/
			double dummyAvg = 0, dummyMax = 0, dummyMin = 0;
			double bufferReal [MAX_BUFFER_SIZE] = {0};
			double bufferImage[MAX_BUFFER_SIZE] = {0};

			analysisOK = true;

			// P_av_no_gap_all_dBm
			err = ::LP_IQ2010EXT_GetDoubleMeasurements( "P_av_no_gap_all_dBm", &l_txVerifyPowerReturn.POWER_AVERAGE_DBM, &l_txVerifyPowerReturn.POWER_AVERAGE_MIN_DBM, &l_txVerifyPowerReturn.POWER_AVERAGE_MAX_DBM );
			if ( ERR_OK!=err )
			{
				analysisOK = false;
				l_txVerifyPowerReturn.POWER_AVERAGE_DBM = l_txVerifyPowerReturn.POWER_AVERAGE_MIN_DBM = l_txVerifyPowerReturn.POWER_AVERAGE_MAX_DBM = NA_NUMBER;
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_Analysis_Failed", l_txVerifyPowerParam.FREQ_MHZ, l_txVerifyPowerParam.DATA_RATE, l_txVerifyPowerParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetDoubleMeasurements(P_av_no_gap_all_dBm) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(P_av_no_gap_all_dBm) return OK.\n");
				// Since the limitation, we assume that all path loss value are very close.
				int antenaOrder = 0;
				err = CheckAntennaOrderByStream(1, l_txVerifyPowerParam.TX1, l_txVerifyPowerParam.TX2, l_txVerifyPowerParam.TX3, l_txVerifyPowerParam.TX4, &antenaOrder);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] CheckAntennaOrderByStream() return error.\n");
					throw logMessage;
				}

				l_txVerifyPowerReturn.POWER_AVERAGE_DBM	    = l_txVerifyPowerReturn.POWER_AVERAGE_DBM     + l_txVerifyPowerParam.CABLE_LOSS_DB[antenaOrder-1];
				l_txVerifyPowerReturn.POWER_AVERAGE_MIN_DBM = l_txVerifyPowerReturn.POWER_AVERAGE_MIN_DBM + l_txVerifyPowerParam.CABLE_LOSS_DB[antenaOrder-1];
				l_txVerifyPowerReturn.POWER_AVERAGE_MAX_DBM = l_txVerifyPowerReturn.POWER_AVERAGE_MAX_DBM + l_txVerifyPowerParam.CABLE_LOSS_DB[antenaOrder-1];
			}

			// P_pk_each_burst_dBm
			err = ::LP_IQ2010EXT_GetDoubleMeasurements( "P_pk_each_burst_dBm", &l_txVerifyPowerReturn.POWER_PEAK_DBM, &l_txVerifyPowerReturn.POWER_PEAK_MIN_DBM, &l_txVerifyPowerReturn.POWER_PEAK_MAX_DBM );
			if ( ERR_OK!=err )
			{
				analysisOK = false;
				l_txVerifyPowerReturn.POWER_PEAK_DBM = l_txVerifyPowerReturn.POWER_PEAK_MIN_DBM = l_txVerifyPowerReturn.POWER_PEAK_MAX_DBM = NA_NUMBER;
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_Analysis_Failed", l_txVerifyPowerParam.FREQ_MHZ, l_txVerifyPowerParam.DATA_RATE, l_txVerifyPowerParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetDoubleMeasurements(P_pk_each_burst_dBm) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(P_pk_each_burst_dBm) return OK.\n");
				// Since the limitation, we assume that all path loss value are very close.
				int antenaOrder = 0;
				err = CheckAntennaOrderByStream(1, l_txVerifyPowerParam.TX1, l_txVerifyPowerParam.TX2, l_txVerifyPowerParam.TX3, l_txVerifyPowerParam.TX4, &antenaOrder);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] CheckAntennaOrderByStream() return error.\n");
					throw logMessage;
				}

				l_txVerifyPowerReturn.POWER_PEAK_DBM	 = l_txVerifyPowerReturn.POWER_PEAK_DBM     + l_txVerifyPowerParam.CABLE_LOSS_DB[antenaOrder-1];
				l_txVerifyPowerReturn.POWER_PEAK_MIN_DBM = l_txVerifyPowerReturn.POWER_PEAK_MIN_DBM + l_txVerifyPowerParam.CABLE_LOSS_DB[antenaOrder-1];
				l_txVerifyPowerReturn.POWER_PEAK_MAX_DBM = l_txVerifyPowerReturn.POWER_PEAK_MAX_DBM + l_txVerifyPowerParam.CABLE_LOSS_DB[antenaOrder-1];
			
#pragma endregion

		}
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
			sprintf_s(l_txVerifyPowerReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txVerifyPowerReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_txVerifyPowerReturn.ERROR_MESSAGE, msg);

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
		ReturnErrorMessage(l_txVerifyPowerReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
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


	return err;
}

int InitializeTXVerifyPowerContainers(void)
{
	/*------------------*
	 * Input Parameters  *
	 *------------------*/
	l_txVerifyPowerParamMap.clear();

	WIFI_SETTING_STRUCT setting;

	strcpy_s(l_txVerifyPowerParam.BANDWIDTH, MAX_BUFFER_SIZE, "HT20");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyPowerParam.BANDWIDTH))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyPowerParam.BANDWIDTH;
		setting.unit        = "";
		setting.helpText    = "Channel bandwidth\r\nValid options: HT20 or HT40";
		l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BANDWIDTH", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_txVerifyPowerParam.DATA_RATE, MAX_BUFFER_SIZE, "OFDM-54");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyPowerParam.DATA_RATE))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyPowerParam.DATA_RATE;
		setting.unit        = "";
		setting.helpText    = "Data rate names:\r\nDSSS-1,DSSS-2,CCK-5_5,CCK-11\r\nOFDM-6,OFDM-9,OFDM-12,OFDM-18,OFDM-24,OFDM-36,OFDM-48,OFDM-54\r\nMCS0, MCS15, etc.";
		l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_txVerifyPowerParam.PREAMBLE, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyPowerParam.PREAMBLE))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyPowerParam.PREAMBLE;
		setting.unit        = "";
		setting.helpText    = "The preamble type of 11B(only), can be SHORT or LONG, Default=LONG.";
		l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PREAMBLE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_txVerifyPowerParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, "MIXED");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyPowerParam.PACKET_FORMAT_11N))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyPowerParam.PACKET_FORMAT_11N;
		setting.unit        = "";
		setting.helpText    = "The packet format of 11N(only), can be MIXED or GREENFIELD, Default=MIXED.";
		l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT_11N", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_txVerifyPowerParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyPowerParam.GUARD_INTERVAL_11N))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyPowerParam.GUARD_INTERVAL_11N;
		setting.unit        = "";
		setting.helpText    = "The guard interval format of 11N(only), can be LONG or SHORT, Default=LONG.";
		l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL_11N", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyPowerParam.FREQ_MHZ = 2412;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyPowerParam.FREQ_MHZ))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyPowerParam.FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "Channel center frequency in MHz";
		l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_MHZ", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	//l_txVerifyPowerParam.T_INTERVAL = 3.2;
	//setting.type = WIFI_SETTING_TYPE_DOUBLE;
	//if (sizeof(double)==sizeof(l_txVerifyPowerParam.T_INTERVAL))    // Type_Checking
	//{
	//    setting.value       = (void*)&l_txVerifyPowerParam.T_INTERVAL;
	//    setting.unit        = "uS";
	//    setting.helpText    = "Capture time in micro-seconds";
	//    l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("T_INTERVAL", setting) );
	//}
	//else
	//{
	//    printf("Parameter Type Error!\n");
	//    exit(1);
	//}

	//l_txVerifyPowerParam.MAX_POW_DIFF_DB = 15.0;
	//setting.type = WIFI_SETTING_TYPE_DOUBLE;
	//if (sizeof(double)==sizeof(l_txVerifyPowerParam.MAX_POW_DIFF_DB))    // Type_Checking
	//{
	//    setting.value       = (void*)&l_txVerifyPowerParam.MAX_POW_DIFF_DB;
	//    setting.unit        = "dBm";
	//    setting.helpText    = "TThe maximum power difference between packets that are expected to be detected.";
	//    l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("MAX_POW_DIFF_DB", setting) );
	//}
	//else
	//{
	//    printf("Parameter Type Error!\n");
	//    exit(1);
	//}

	l_txVerifyPowerParam.TX_POWER_DBM = 15.0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyPowerParam.TX_POWER_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyPowerParam.TX_POWER_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected power level at DUT antenna port (For AR6004, set -99 for default target power.)";
		l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyPowerParam.SAMPLING_TIME_US = 0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyPowerParam.SAMPLING_TIME_US))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyPowerParam.SAMPLING_TIME_US;
		setting.unit        = "us";
		setting.helpText    = "Capture time in micro-seconds";
		l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("SAMPLING_TIME_US", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyPowerParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyPowerParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyPowerParam.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_txVerifyPowerParam.TX1 = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyPowerParam.TX1))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyPowerParam.TX1;
		setting.unit        = "";
		setting.helpText    = "DUT TX path 1 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
		l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX1", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyPowerParam.TX2 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyPowerParam.TX2))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyPowerParam.TX2;
		setting.unit        = "";
		setting.helpText    = "DUT TX path 2 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
		l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX2", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyPowerParam.TX3 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyPowerParam.TX3))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyPowerParam.TX3;
		setting.unit        = "";
		setting.helpText    = "DUT TX path 3 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
		l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX3", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyPowerParam.TX4 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyPowerParam.TX4))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyPowerParam.TX4;
		setting.unit        = "";
		setting.helpText    = "DUT TX path 4 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
		l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX4", setting) );
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
	l_txVerifyPowerReturnMap.clear();

	// -cfy@sunnyvale, 2012/3/13-
	l_txVerifyPowerReturn.TX_POWER_DBM = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyPowerReturn.TX_POWER_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyPowerReturn.TX_POWER_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected power level at DUT antenna port.";
		l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	/* <><~~ */

	l_txVerifyPowerReturn.POWER_AVERAGE_DBM = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_AVERAGE_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyPowerReturn.POWER_AVERAGE_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Average power of each burst in dBm.";
		l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_AVERAGE_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyPowerReturn.POWER_AVERAGE_MAX_DBM = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_AVERAGE_MAX_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyPowerReturn.POWER_AVERAGE_MAX_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "(Maximum) Average power of each burst in dBm.";
		l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_AVERAGE_MAX_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyPowerReturn.POWER_AVERAGE_MIN_DBM = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_AVERAGE_MIN_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyPowerReturn.POWER_AVERAGE_MIN_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "(Minimum) Average power of each burst in dBm.";
		l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_AVERAGE_MIN_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyPowerReturn.POWER_PEAK_DBM = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_PEAK_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyPowerReturn.POWER_PEAK_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Peak power of each burst in dBm.";
		l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_PEAK_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyPowerReturn.POWER_PEAK_MAX_DBM = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_PEAK_MAX_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyPowerReturn.POWER_PEAK_MAX_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "(Maximum) Peak power of each burst in dBm.";
		l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_PEAK_MAX_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyPowerReturn.POWER_PEAK_MIN_DBM = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_PEAK_MIN_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyPowerReturn.POWER_PEAK_MIN_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "(Minimum) Peak power of each burst in dBm.";
		l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_PEAK_MIN_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyPowerReturn.CABLE_LOSS_DB[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyPowerReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyPowerReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_txVerifyPowerReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyPowerReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyPowerReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	return 0;
}


//-------------------------------------------------------------------------------------
// This is a function for checking the input parameters before the test.
//
//-------------------------------------------------------------------------------------
int CheckTxPowerParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *samplingTimeUs, double *cableLossDb, double *peakToAvgRatio, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		// Checking the Input Parameters
		err = TM_WiFiConvertFrequencyToChannel(l_txVerifyPowerParam.FREQ_MHZ, channel);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_txVerifyPowerParam.FREQ_MHZ);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}

		err = TM_WiFiConvertDataRateNameToIndex(l_txVerifyPowerParam.DATA_RATE, &dummyInt);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_txVerifyPowerParam.DATA_RATE);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}

		if ( 0!=strcmp(l_txVerifyPowerParam.BANDWIDTH, "HT20") && 0!=strcmp(l_txVerifyPowerParam.BANDWIDTH, "HT40")
				&& 0!=strcmp(l_txVerifyPowerParam.BANDWIDTH, "QUAR") && 0!=strcmp(l_txVerifyPowerParam.BANDWIDTH, "HALF") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown BANDWIDTH, WiFi bandwidth %s not supported.\n", l_txVerifyPowerParam.BANDWIDTH);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txVerifyPowerParam.PREAMBLE, "SHORT") && 0!=strcmp(l_txVerifyPowerParam.PREAMBLE, "LONG") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_txVerifyPowerParam.PREAMBLE);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txVerifyPowerParam.PACKET_FORMAT_11N, "MIXED") && 0!=strcmp(l_txVerifyPowerParam.PACKET_FORMAT_11N, "GREENFIELD") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PACKET_FORMAT_11N, WiFi 11n packet format %s not supported.\n", l_txVerifyPowerParam.PACKET_FORMAT_11N);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txVerifyPowerParam.GUARD_INTERVAL_11N, "LONG") && 0!=strcmp(l_txVerifyPowerParam.GUARD_INTERVAL_11N, "SHORT") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown GUARD_INTERVAL_11N, WiFi 11n packet format %s not supported.\n", l_txVerifyPowerParam.GUARD_INTERVAL_11N);
			throw logMessage;
		}


		// Convert parameter
		err = WiFiTestMode(l_txVerifyPowerParam.DATA_RATE, l_txVerifyPowerParam.BANDWIDTH, wifiMode, wifiStreamNum);
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
				l_txVerifyPowerParam.FREQ_MHZ,
				l_txVerifyPowerParam.TX1,
				l_txVerifyPowerParam.TX2,
				l_txVerifyPowerParam.TX3,
				l_txVerifyPowerParam.TX4,
				l_txVerifyPowerParam.CABLE_LOSS_DB,
				l_txVerifyPowerReturn.CABLE_LOSS_DB,
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

		// Check Capture Time
		if (0==l_txVerifyPowerParam.SAMPLING_TIME_US)
		{
			if ( *wifiMode==WIFI_11B )
			{
				*samplingTimeUs = g_WiFiGlobalSettingParam.PM_DSSS_SAMPLE_INTERVAL_US;
			}
			else if ( *wifiMode==WIFI_11AG )
			{
				if (0==strcmp(l_txVerifyPowerParam.BANDWIDTH,"HALF")) {
					*samplingTimeUs = g_WiFiGlobalSettingParam.PM_HALF_SAMPLE_INTERVAL_US;
				} else if (0==strcmp(l_txVerifyPowerParam.BANDWIDTH,"QUAR")) {
					*samplingTimeUs = g_WiFiGlobalSettingParam.PM_QUAR_SAMPLE_INTERVAL_US;
				} else {
					*samplingTimeUs = g_WiFiGlobalSettingParam.PM_OFDM_SAMPLE_INTERVAL_US;
				}
			}
			else	// 802.11n
			{
				// TODO
				*samplingTimeUs = g_WiFiGlobalSettingParam.PM_OFDM_SAMPLE_INTERVAL_US;
			}
		}
		else	// SAMPLING_TIME_US != 0
		{
			*samplingTimeUs = l_txVerifyPowerParam.SAMPLING_TIME_US;
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
				0!=strcmp(l_txVerifyPowerParam.BANDWIDTH, g_RecordedParam.BANDWIDTH) ||
				0!=strcmp(l_txVerifyPowerParam.DATA_RATE, g_RecordedParam.DATA_RATE) ||
				0!=strcmp(l_txVerifyPowerParam.PREAMBLE,  g_RecordedParam.PREAMBLE) ||
				0!=strcmp(l_txVerifyPowerParam.PACKET_FORMAT_11N, g_RecordedParam.PACKET_FORMAT_11N) ||
				0!=strcmp(l_txVerifyPowerParam.GUARD_INTERVAL_11N, g_RecordedParam.GUARD_INTERVAL_11N) ||
				l_txVerifyPowerParam.CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
				l_txVerifyPowerParam.CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
				l_txVerifyPowerParam.CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
				l_txVerifyPowerParam.CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
				l_txVerifyPowerParam.FREQ_MHZ!=g_RecordedParam.FREQ_MHZ ||
				l_txVerifyPowerParam.TX_POWER_DBM!=g_RecordedParam.POWER_DBM ||
				l_txVerifyPowerParam.TX1!=g_RecordedParam.ANT1 ||
				l_txVerifyPowerParam.TX2!=g_RecordedParam.ANT2 ||
				l_txVerifyPowerParam.TX3!=g_RecordedParam.ANT3 ||
				l_txVerifyPowerParam.TX4!=g_RecordedParam.ANT4)
		{
			g_dutConfigChanged = true;
		}
		else
		{
			g_dutConfigChanged = false;
		}

		// Save the current setup
		g_RecordedParam.ANT1					= l_txVerifyPowerParam.TX1;
		g_RecordedParam.ANT2					= l_txVerifyPowerParam.TX2;
		g_RecordedParam.ANT3					= l_txVerifyPowerParam.TX3;
		g_RecordedParam.ANT4					= l_txVerifyPowerParam.TX4;
		g_RecordedParam.CABLE_LOSS_DB[0]		= l_txVerifyPowerParam.CABLE_LOSS_DB[0];
		g_RecordedParam.CABLE_LOSS_DB[1]		= l_txVerifyPowerParam.CABLE_LOSS_DB[1];
		g_RecordedParam.CABLE_LOSS_DB[2]		= l_txVerifyPowerParam.CABLE_LOSS_DB[2];
		g_RecordedParam.CABLE_LOSS_DB[3]		= l_txVerifyPowerParam.CABLE_LOSS_DB[3];
		g_RecordedParam.FREQ_MHZ				= l_txVerifyPowerParam.FREQ_MHZ;
		g_RecordedParam.POWER_DBM				= l_txVerifyPowerParam.TX_POWER_DBM;

		sprintf_s(g_RecordedParam.BANDWIDTH, MAX_BUFFER_SIZE, l_txVerifyPowerParam.BANDWIDTH);
		sprintf_s(g_RecordedParam.DATA_RATE, MAX_BUFFER_SIZE, l_txVerifyPowerParam.DATA_RATE);
		sprintf_s(g_RecordedParam.PREAMBLE,  MAX_BUFFER_SIZE, l_txVerifyPowerParam.PREAMBLE);
		sprintf_s(g_RecordedParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, l_txVerifyPowerParam.PACKET_FORMAT_11N);
		sprintf_s(g_RecordedParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, l_txVerifyPowerParam.GUARD_INTERVAL_11N);



		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] CheckTxPowerParameters() Confirmed.\n");
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
