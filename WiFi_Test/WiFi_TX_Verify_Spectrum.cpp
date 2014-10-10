#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_Test.h"
#include "WiFi_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include <math.h> // fmod on mac

using namespace std;

// Carrier number
#define CARRIER_0    0
#define CARRIER_1    1
#define CARRIER_2    2
#define CARRIER_16   16
#define CARRIER_17   17
#define CARRIER_26   26
#define CARRIER_28   28
#define CARRIER_42   42
#define CARRIER_43   43
#define CARRIER_58   58

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_txVerifyFlatnessParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_txVerifyFlatnessReturnMap;

struct tagParam
{
	// Mandatory Parameters
	int    FREQ_MHZ;                                /*! The center frequency (MHz). */
	char   BANDWIDTH[MAX_BUFFER_SIZE];              /*! The RF bandwidth to verify Flatness. */
	char   DATA_RATE[MAX_BUFFER_SIZE];              /*! The data rate to verify Flatness. */
	char   PREAMBLE[MAX_BUFFER_SIZE];               /*! The preamble type of 11B(only). */
	char   PACKET_FORMAT_11N[MAX_BUFFER_SIZE];      /*! The packet format of 11N(only). */
	char   GUARD_INTERVAL_11N[MAX_BUFFER_SIZE];     /*! The guard interval format of 11N(only). */
	double TX_POWER_DBM;                            /*! The output power to verify Flatness. */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
	double SAMPLING_TIME_US;                        /*! The sampling time to verify Flatness. */
	double CORRECTION_FACTOR_11B;                   /*! The 802.11b(only) LO leakage correction factor. */

	// DUT Parameters
	int    TX1;                                     /*!< DUT TX1 on/off. Default=1(on)  */
	int    TX2;                                     /*!< DUT TX2 on/off. Default=0(off) */
	int    TX3;                                     /*!< DUT TX3 on/off. Default=0(off) */
	int    TX4;                                     /*!< DUT TX4 on/off. Default=0(off) */
} l_txVerifyFlatnessParam;

struct tagReturn
{
	double TX_POWER_DBM;                            /*!< TX power dBm setting */

	// Flatness Test Result
	double MARGIN_DB;              /*!< Margin to the flatness, normally should be a plus number */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
	double VALUE_DB[MAX_DATA_STREAM];               /*!< Actually margin value to the average spectral center power. */
	double LO_LEAKAGE_DB;
	int    LEAST_MARGIN_SUBCARRIER;

	int    SUBCARRIER_LO_A;		/*!< Margin at which carrier in lower section A. */
	int    SUBCARRIER_LO_B;		/*!< Margin at which carrier in lower section B. */
	int    SUBCARRIER_UP_A;		/*!< Margin at which carrier in upper section A. */
	int    SUBCARRIER_UP_B;		/*!< Margin at which carrier in upper section B. */

	double VALUE_DB_LO_A;       /*!< Actually margin value to the average spectral center power in lower section A. */
	double VALUE_DB_LO_B;       /*!< Actually margin value to the average spectral center power in lower section B. */
	double VALUE_DB_UP_A;       /*!< Actually margin value to the average spectral center power in upper section A. */
	double VALUE_DB_UP_B;       /*!< Actually margin value to the average spectral center power in upper section B. */

	char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txVerifyFlatnessReturn;
#pragma endregion

void ClearTxSpectrumReturn(void)
{
	l_txVerifyFlatnessParamMap.clear();
	l_txVerifyFlatnessReturnMap.clear();
}

#ifndef WIN32
int initTXVerifyFlatnessContainers = InitializeTXVerifyFlatnessContainers();
#endif

// These global variables/functions only for WiFi_TX_Verify_Spectrum.cpp
int CheckTxSpectrumParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *samplingTimeUs, double *cableLossDb, double *peakToAvgRatio, char* errorMsg );
void CheckFlatnessViolation(double deltaPower, int carrier, double lowLimit, double highLimit, double *sectionPower, int *sectionCarrier, double *failMargin, int *sectionCarrier2);
int WiFi_TX_Verify_Spectrum_2010ExtTest(void);

//! WiFi TX Verify Flatness
/*!
 * Input Parameters
 *
 *  - Mandatory
 *      -# FREQ_MHZ (double): The center frequency (MHz)
 *      -# DATA_RATE (string): The data rate to verify Flatness
 *      -# TX_POWER_DBM (double): The power (dBm) DUT is going to transmit at the antenna port
 *
 * Return Values
 *      -# A string that contains all DUT info, such as FW version, driver version, chip revision, etc.
 *      -# A string for error message
 *
 * \return 0 No error occurred
 * \return -1 Error(s) occurred.  Please see the returned error message for details
 */


WIFI_TEST_API int WiFi_TX_Verify_Spectrum(void)
{
	int    err = ERR_OK;

	bool   analysisOK = false, captureOK = false, enableMultiCapture = false;
	int    channel = 0, HT40ModeOn = 0, packetFormat = 0;
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
	ClearReturnParameters(l_txVerifyFlatnessReturnMap);

	/*------------------------*
	 * Respond to QUERY_INPUT *
	 *------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_txVerifyFlatnessParamMap);
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
		RespondToQueryReturn(l_txVerifyFlatnessReturnMap);
		return err;
	}
	else
	{
		// do nothing
	}

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

		/*--------------------------*
		 * Get mandatory parameters *
		 *--------------------------*/
		err = GetInputParameters(l_txVerifyFlatnessParamMap);
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
		* needed to split WIFI_TX_Multi_Verifcation into two to	* 
		* avoid a stack check issue.							*
										*
		*------------------------------------------------------*/
		if ( g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE )
		{
			return WiFi_TX_Verify_Spectrum_2010ExtTest();
		}

		// Error return of this function is irrelevant
		CheckDutTransmitStatus();

#pragma region Prepare input parameters

		err = CheckTxSpectrumParameters( &channel, &wifiMode, &wifiStreamNum, &samplingTimeUs, &cableLossDb, &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters CheckTxSpectrumParameters() return OK.\n");
		}

		// Modified /* -cfy@sunnyvale, 2012/3/13- */
		// Considering stream number while calculating chain gain for composite measurement so as to set the proper Rx amplitude in tester
		int antennaNum = l_txVerifyFlatnessParam.TX1 + l_txVerifyFlatnessParam.TX2 + l_txVerifyFlatnessParam.TX3 + l_txVerifyFlatnessParam.TX4;
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

		if (0==strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT_11N, "MIXED"))
			packetFormat = 1;
		else if (0 == strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT_11N, "GREENFIELD"))
			packetFormat = 2;
		else
			packetFormat = 0;

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
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_txVerifyFlatnessParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_txVerifyFlatnessParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_txVerifyFlatnessParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",      l_txVerifyFlatnessParam.FREQ_MHZ);
		}

		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",		  l_txVerifyFlatnessParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_txVerifyFlatnessParam.PACKET_FORMAT_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N", l_txVerifyFlatnessParam.GUARD_INTERVAL_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",		  l_txVerifyFlatnessParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",		  l_txVerifyFlatnessParam.BANDWIDTH);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		  HT40ModeOn);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",				  l_txVerifyFlatnessParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",				  l_txVerifyFlatnessParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",				  l_txVerifyFlatnessParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",				  l_txVerifyFlatnessParam.TX4);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "SAMPLING_TIME_US",  samplingTimeUs); // -cfy@sunnyvale, 2012/3/13-
		// -cfy@sunnyvale, 2012/3/13-
		// Added GET_TX_TARGET_POWER feature for AR6004. When TX_POWER_DBM is set to -99, default target power will be applied.
		if ( (l_txVerifyFlatnessParam.TX_POWER_DBM == TX_TARGET_POWER_FLAG) )
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
				l_txVerifyFlatnessParam.TX_POWER_DBM = tx_target_power;
				g_last_TxPower_dBm_Record = l_txVerifyFlatnessParam.TX_POWER_DBM;
			}
			else
			{
				l_txVerifyFlatnessParam.TX_POWER_DBM = g_last_TxPower_dBm_Record;
			}
		}
		else
		{
			// do nothing
		}
		/* <><~~ */

		vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM",	  l_txVerifyFlatnessParam.TX_POWER_DBM);
		TM_SetDoubleParameter(g_WiFi_Test_ID, "TX_POWER_DBM", l_txVerifyFlatnessParam.TX_POWER_DBM); // -cfy@sunnyvale, 2012/3/13-

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

#pragma region Setup LP Tester and Capture

		
			// NOT "g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE"
		
#pragma region Setup LP Tester and Capture

			err = ::LP_SetVsa(  l_txVerifyFlatnessParam.FREQ_MHZ*1e6,
					l_txVerifyFlatnessParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGainDb,
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

			/*----------------------------*
			 * Perform Normal VSA capture *
			 *----------------------------*/
			double sampleFreqHz = 80e6;
			HT40ModeOn = 0;
			err = ::LP_VsaDataCapture( samplingTimeUs/1000000, g_WiFiGlobalSettingParam.VSA_TRIGGER_TYPE, sampleFreqHz, HT40ModeOn );
			if( ERR_OK!=err )	// capture is failed
			{
				double quickPower = NA_NUMBER;
				err = QuickCaptureRetry( l_txVerifyFlatnessParam.FREQ_MHZ, samplingTimeUs, g_WiFiGlobalSettingParam.VSA_TRIGGER_TYPE, HT40ModeOn, &quickPower, logMessage);
				if (ERR_OK!=err)	// QuickCaptureRetry() is failed
				{
					// Fail Capture
					if ( quickPower!=NA_NUMBER )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to capture WiFi signal at %d MHz.\nThe DUT power (without path loss) = %.1f dBm and QuickCaptureRetry() return error.\n", l_txVerifyFlatnessParam.FREQ_MHZ, quickPower);
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to capture WiFi signal at %d MHz, QuickCaptureRetry() return error.\n", l_txVerifyFlatnessParam.FREQ_MHZ);
					}
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] The QuickCaptureRetry() at %d MHz return OK.\n", l_txVerifyFlatnessParam.FREQ_MHZ);
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_VsaDataCapture() at %d MHz return OK.\n", l_txVerifyFlatnessParam.FREQ_MHZ);
			}
#pragma endregion

			/*--------------*
			 *  Capture OK  *
			 *--------------*/
			captureOK = true;
			if (1==g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)	// 1: Turn On
			{
				// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_SaveAlways", l_txVerifyFlatnessParam.FREQ_MHZ, l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
			}
			else
			{
				// do nothing
			}

			/*------------------*
			 *  Start Analysis  *
			 *------------------*/
#pragma region Analysis_802_11b
			if ( wifiMode==WIFI_11B )
			{   // [Case 01]: 802.11b Analysis
				// TODO: The official way is to have a continuously transmitting DUT, scrambler disabled, 2 Mbps, 01 data sequence.
				if ( (0==strcmp(l_txVerifyFlatnessParam.DATA_RATE, "DSSS-2")) && (1==g_WiFiGlobalSettingParam.ANALYSIS_11B_FIXED_01_DATA_SEQUENCE) )
				{
					// Perform FFT analysis
					err = ::LP_AnalyzeFFT();
					if ( ERR_OK!=err )
					{
						// Fail Analysis, thus save capture (Signal File) for debug
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_Analysis_Failed", l_txVerifyFlatnessParam.FREQ_MHZ, l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.BANDWIDTH);
						WiFiSaveSigFile(sigFileNameBuffer);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_AnalyzeFFT() return error.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_AnalyzeFFT() return OK.\n");
					}
				}
				else
				{
					// First, Check 11B correction factor
					if (0==l_txVerifyFlatnessParam.CORRECTION_FACTOR_11B)
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Need correction factor (non-zero) for 802.11B LO leakage measurement.\n");
					}
					else
					{
						// Settings: No equalization, DC removal off, classic EVM method
						err = ::LP_Analyze80211b(   1,	//IQV_EQ_OFF,
								0,  //IQV_DC_REMOVAL_OFF,
								1	//IQV_11B_STANDARD_TX_ACC
								);
						if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_Analyze80211b() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_Analyze80211b() return OK.\n");
						}
					}
				}
			}
#pragma endregion
#pragma region Analysis_802_11ag
			else if ( wifiMode==WIFI_11AG )
			{   // [Case 02]: 802.11a/g Analysis
				if (0==strcmp(l_txVerifyFlatnessParam.BANDWIDTH,"HALF") || 0==strcmp(l_txVerifyFlatnessParam.BANDWIDTH,"QUAR"))
				{
					err = ::LP_Analyze80211p(  g_WiFiGlobalSettingParam.ANALYSIS_11AG_PH_CORR_MODE,
							g_WiFiGlobalSettingParam.ANALYSIS_11AG_CH_ESTIMATE,
							g_WiFiGlobalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR,
							g_WiFiGlobalSettingParam.ANALYSIS_11AG_FREQ_SYNC,
							g_WiFiGlobalSettingParam.ANALYSIS_11AG_AMPL_TRACK,
							(0==strcmp(l_txVerifyFlatnessParam.BANDWIDTH,"HALF"))?IQV_OFDM_ASTM_DSRC:IQV_OFDM_QUARTER_RATE
							);
					if ( ERR_OK!=err )
					{
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
				char referenceFileName[MAX_BUFFER_SIZE] = {'\0'}, analyzeMimoType[MAX_BUFFER_SIZE] = {'\0'}, analyzeMimoMode[MAX_BUFFER_SIZE] = {'\0'};

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
									l_txVerifyFlatnessParam.BANDWIDTH,
									l_txVerifyFlatnessParam.DATA_RATE,
									l_txVerifyFlatnessParam.PREAMBLE,
									l_txVerifyFlatnessParam.PACKET_FORMAT_11N,
									l_txVerifyFlatnessParam.GUARD_INTERVAL_11N,
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
			analysisOK = true;

#pragma region Analysis_802_11b
			if ( wifiMode==WIFI_11B )
			{   // [Case 01]: 802.11b Analysis
				if ( (0==strcmp(l_txVerifyFlatnessParam.DATA_RATE, "DSSS-2")) &&
						(1==g_WiFiGlobalSettingParam.ANALYSIS_11B_FIXED_01_DATA_SEQUENCE) )
				{
					// Store the result
					double bufferRealX[MAX_BUFFER_SIZE], bufferImagX[MAX_BUFFER_SIZE];
					double bufferRealY[MAX_BUFFER_SIZE], bufferImagY[MAX_BUFFER_SIZE];
					int    bufferSizeX = ::LP_GetVectorMeasurement("x", bufferRealX, bufferImagX, MAX_BUFFER_SIZE);
					int    bufferSizeY = ::LP_GetVectorMeasurement("y", bufferRealY, bufferImagY, MAX_BUFFER_SIZE);

					if ( (0>=bufferSizeX)||(0>=bufferSizeY) )
					{
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Can not retrieve FFT result, result length = 0.\n");
						throw logMessage;
					}
					else
					{
						/*--------------------------*
						 *  Looking for peak power  *
						 *--------------------------*/
						double peakPower = NA_NUMBER;
						for ( int i=0;i<bufferSizeY;i++ )
						{
							if ( bufferRealY[i]>peakPower )
							{   // store the current peak power and keep searching
								peakPower = bufferRealY[i];
							}
							else
							{
								// keep searching...
							}
						}

						if ( NA_NUMBER>=peakPower )
						{
							err = -1;
							peakPower = NA_NUMBER;
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Can not find out the peak power.\n");
							throw logMessage;
						}
						else
						{
							// Verify DC Leakage
							int zeroIndex = (int)(bufferSizeX/2) - 3;
							while(true)
							{
								if (bufferRealX[zeroIndex]==0)
								{
									// this is the offset "ZERO", break the loop.
									break;
								}
								else
								{
									zeroIndex++;
								}

								if ( zeroIndex>((int)(bufferSizeX/2)+3) )
								{
									err = -1;
									LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to find out the offset zero of the FFT results.\n");
									throw logMessage;
								}
							}

							double deltaPower = bufferRealY[zeroIndex] - peakPower;
							if (0!=deltaPower)
							{
								l_txVerifyFlatnessReturn.LO_LEAKAGE_DB =  deltaPower;
							}
							else
							{
								err = -1;
								l_txVerifyFlatnessReturn.LO_LEAKAGE_DB = 0;
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to retrieve 11B loLeakageDb.\n");
								throw logMessage;
							}
						}
					}
				}
				else
				{
					// Store the result
					double bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
					int    bufferSize = ::LP_GetVectorMeasurement("loLeakageDb", bufferReal, bufferImag, MAX_BUFFER_SIZE);

					if (bufferSize>0)
					{
						l_txVerifyFlatnessReturn.LO_LEAKAGE_DB = bufferReal[0] + l_txVerifyFlatnessParam.CORRECTION_FACTOR_11B;
					}
					else
					{
						err = -1;
						l_txVerifyFlatnessReturn.LO_LEAKAGE_DB = 0;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to retrieve 11B loLeakageDb.\n");
						throw logMessage;
					}
				}
			}
#pragma endregion
#pragma region Analysis_802_11ag
			else if ( wifiMode==WIFI_11AG )
			{   // [Case 02]: 802.11a/g Analysis
				err = LoResultSpectralFlatness11AG( &l_txVerifyFlatnessReturn.SUBCARRIER_UP_A,
						&l_txVerifyFlatnessReturn.SUBCARRIER_UP_B,
						&l_txVerifyFlatnessReturn.SUBCARRIER_LO_A,
						&l_txVerifyFlatnessReturn.SUBCARRIER_LO_B,
						&l_txVerifyFlatnessReturn.MARGIN_DB,
						&l_txVerifyFlatnessReturn.LEAST_MARGIN_SUBCARRIER,
						&l_txVerifyFlatnessReturn.LO_LEAKAGE_DB,
						&l_txVerifyFlatnessReturn.VALUE_DB_UP_A,
						&l_txVerifyFlatnessReturn.VALUE_DB_UP_B,
						&l_txVerifyFlatnessReturn.VALUE_DB_LO_A,
						&l_txVerifyFlatnessReturn.VALUE_DB_LO_B
						);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LoResultSpectralFlatness11AG() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LoResultSpectralFlatness11AG() return OK.\n");
				}
			}
#pragma endregion
#pragma region Analysis_802_11n
			else if ( (wifiMode==WIFI_11N_HT20)||(wifiMode==WIFI_11N_HT40) )
			{
				err = LoResultSpectralFlatness11N(  1,
						wifiMode,
						&l_txVerifyFlatnessReturn.SUBCARRIER_UP_A,
						&l_txVerifyFlatnessReturn.SUBCARRIER_UP_B,
						&l_txVerifyFlatnessReturn.SUBCARRIER_LO_A,
						&l_txVerifyFlatnessReturn.SUBCARRIER_LO_B,
						&l_txVerifyFlatnessReturn.MARGIN_DB,
						&l_txVerifyFlatnessReturn.LEAST_MARGIN_SUBCARRIER,
						&l_txVerifyFlatnessReturn.LO_LEAKAGE_DB,
						&l_txVerifyFlatnessReturn.VALUE_DB_UP_A,
						&l_txVerifyFlatnessReturn.VALUE_DB_UP_B,
						&l_txVerifyFlatnessReturn.VALUE_DB_LO_A,
						&l_txVerifyFlatnessReturn.VALUE_DB_LO_B,
						0
						);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LoResultSpectralFlatness11N() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LoResultSpectralFlatness11N() return OK.\n");
				}
			}
#pragma endregion

#pragma region Averaging and Saving Test Result // Modified /* -cfy@sunnyvale, 2012/3/13- */
			/*----------------------------------*
			 * Averaging and Saving Test Result *
			 *----------------------------------*/
			if ( (ERR_OK==err) && captureOK && analysisOK )
			{
				// Added GET_TX_TARGET_POWER feature for AR6004. When TX_POWER_DBM is set to -99, default target power will be applied.
				// TBD: 2010 not support for this
				l_txVerifyFlatnessReturn.TX_POWER_DBM = l_txVerifyFlatnessParam.TX_POWER_DBM;
			}
			else
			{
				// do nothing
			}
#pragma endregion /* <><~~ */

		// NOT "g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE"

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
			sprintf_s(l_txVerifyFlatnessReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txVerifyFlatnessReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_txVerifyFlatnessReturn.ERROR_MESSAGE, msg);

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
		ReturnErrorMessage(l_txVerifyFlatnessReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
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

int InitializeTXVerifyFlatnessContainers(void)
{
	/*------------------*
	 * Input Parameters  *
	 *------------------*/
	l_txVerifyFlatnessParamMap.clear();

	WIFI_SETTING_STRUCT setting;

	sprintf_s(l_txVerifyFlatnessParam.BANDWIDTH, MAX_BUFFER_SIZE, "%s", "HT20");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.BANDWIDTH))    // Type_Checking
	{
		setting.value = (void*)l_txVerifyFlatnessParam.BANDWIDTH;
		setting.unit        = "";
		setting.helpText    = "Channel bandwidth. Valid options: QUAR, HALF, HT20 or HT40";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BANDWIDTH", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	sprintf_s(l_txVerifyFlatnessParam.DATA_RATE, MAX_BUFFER_SIZE, "%s", "OFDM-54");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.DATA_RATE))    // Type_Checking
	{
		setting.value = (void*)l_txVerifyFlatnessParam.DATA_RATE;
		setting.unit        = "";
		setting.helpText    = "Data rate names, such as DSSS-1, CCK-5_5, CCK-11, OFDM-54, MCS0, MCS15";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_txVerifyFlatnessParam.PREAMBLE, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.PREAMBLE))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyFlatnessParam.PREAMBLE;
		setting.unit        = "";
		setting.helpText    = "The preamble type of 11B(only), can be SHORT or LONG, Default=LONG.";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PREAMBLE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_txVerifyFlatnessParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, "MIXED");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.PACKET_FORMAT_11N))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyFlatnessParam.PACKET_FORMAT_11N;
		setting.unit        = "";
		setting.helpText    = "The packet format of 11N(only), can be MIXED or GREENFIELD, Default=MIXED.";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT_11N", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_txVerifyFlatnessParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.GUARD_INTERVAL_11N))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyFlatnessParam.GUARD_INTERVAL_11N;
		setting.unit        = "";
		setting.helpText    = "The guard interval format of 11N(only), can be LONG or SHORT, Default=LONG.";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL_11N", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_txVerifyFlatnessParam.FREQ_MHZ = 2412;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyFlatnessParam.FREQ_MHZ))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessParam.FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "Channel center frequency in MHz";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_MHZ", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyFlatnessParam.SAMPLING_TIME_US = 0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyFlatnessParam.SAMPLING_TIME_US))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessParam.SAMPLING_TIME_US;
		setting.unit        = "us";
		setting.helpText    = "Capture time in micro-seconds";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("SAMPLING_TIME_US", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyFlatnessParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyFlatnessParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyFlatnessParam.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_txVerifyFlatnessParam.TX_POWER_DBM = 15.0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyFlatnessParam.TX_POWER_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessParam.TX_POWER_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected power level at DUT antenna port (For AR6004, set -99 for default target power.)";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyFlatnessParam.TX1 = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyFlatnessParam.TX1))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessParam.TX1;
		setting.unit        = "";
		setting.helpText    = "DUT TX path 1 on/off. 1:on; 0:off";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX1", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyFlatnessParam.TX2 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyFlatnessParam.TX2))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessParam.TX2;
		setting.unit        = "";
		setting.helpText    = "DUT TX path 2 on/off. 1:on; 0:off";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX2", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyFlatnessParam.TX3 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyFlatnessParam.TX3))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessParam.TX3;
		setting.unit        = "";
		setting.helpText    = "DUT TX path 3 on/off. 1:on; 0:off";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX3", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyFlatnessParam.TX4 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyFlatnessParam.TX4))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessParam.TX4;
		setting.unit        = "";
		setting.helpText    = "DUT TX path 4 on/off. 1:on; 0:off";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX4", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyFlatnessParam.CORRECTION_FACTOR_11B = 9.2;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyFlatnessParam.CORRECTION_FACTOR_11B))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessParam.CORRECTION_FACTOR_11B;
		setting.unit  = "dB";
		setting.helpText  = "802.11b(only) LO leakage correction factor. Chipset specific.";
		l_txVerifyFlatnessParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("CORRECTION_FACTOR_11B", setting) );
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
	l_txVerifyFlatnessReturnMap.clear();

	// -cfy@sunnyvale, 2012/3/13-
	l_txVerifyFlatnessReturn.TX_POWER_DBM = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyFlatnessReturn.TX_POWER_DBM))   // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessReturn.TX_POWER_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected power level at DUT antenna port";
		l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	/* <><~~ */

	// -cfy@sunnyvale, 2012/3/13-
	l_txVerifyFlatnessReturn.LEAST_MARGIN_SUBCARRIER = NA_INTEGER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyFlatnessReturn.LEAST_MARGIN_SUBCARRIER))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessReturn.LEAST_MARGIN_SUBCARRIER;
		setting.unit        = "";
		setting.helpText    = "Carrier no. with least margin";
		l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("LEAST_MARGIN_SUBCARRIER", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyFlatnessReturn.SUBCARRIER_LO_A = NA_INTEGER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyFlatnessReturn.SUBCARRIER_LO_A))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessReturn.SUBCARRIER_LO_A;
		setting.unit        = "";
		setting.helpText    = "Margin at which carrier of lower section A in each VSA.";
		l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SUBCARRIER_LO_A", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyFlatnessReturn.SUBCARRIER_LO_B = NA_INTEGER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyFlatnessReturn.SUBCARRIER_LO_B))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessReturn.SUBCARRIER_LO_B;
		setting.unit        = "";
		setting.helpText    = "Margin at which carrier of lower section B in each VSA.";
		l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SUBCARRIER_LO_B", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyFlatnessReturn.SUBCARRIER_UP_A = NA_INTEGER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyFlatnessReturn.SUBCARRIER_UP_A))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessReturn.SUBCARRIER_UP_A;
		setting.unit        = "";
		setting.helpText    = "Margin at which carrier of upper section A in each VSA.";
		l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SUBCARRIER_UP_A", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyFlatnessReturn.SUBCARRIER_UP_B = NA_INTEGER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyFlatnessReturn.SUBCARRIER_UP_B))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessReturn.SUBCARRIER_UP_B;
		setting.unit        = "";
		setting.helpText    = "Margin at which carrier of upper section B in each VSA.";
		l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SUBCARRIER_UP_B", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	/* <><~~ */

	l_txVerifyFlatnessReturn.MARGIN_DB = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyFlatnessReturn.MARGIN_DB))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessReturn.MARGIN_DB;
		setting.unit        = "dB";
		setting.helpText    = "Margin to the average spectral center power in each VSA.";
		l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("MARGIN_DB", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}



	l_txVerifyFlatnessReturn.LO_LEAKAGE_DB = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyFlatnessReturn.LO_LEAKAGE_DB))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessReturn.LO_LEAKAGE_DB;
		setting.unit        = "dBc";
		setting.helpText    = "Lo leakage in dBc, normally should be a negative number in VSA, if pass.";
		l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("LO_LEAKAGE_DB", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyFlatnessReturn.VALUE_DB_LO_A = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyFlatnessReturn.VALUE_DB_LO_A))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessReturn.VALUE_DB_LO_A;
		setting.unit        = "dB";
		setting.helpText    = "Actually margin value to the average spectral center power of lower section A in each VSA.";
		l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("VALUE_DB_LO_A", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyFlatnessReturn.VALUE_DB_LO_B = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyFlatnessReturn.VALUE_DB_LO_B))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessReturn.VALUE_DB_LO_B;
		setting.unit        = "dB";
		setting.helpText    = "Actually margin value to the average spectral center power of lower section B in each VSA.";
		l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("VALUE_DB_LO_B", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyFlatnessReturn.VALUE_DB_UP_A = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyFlatnessReturn.VALUE_DB_UP_A))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessReturn.VALUE_DB_UP_A;
		setting.unit        = "dB";
		setting.helpText    = "Actually margin value to the average spectral center power of upper section A in each VSA.";
		l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("VALUE_DB_UP_A", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyFlatnessReturn.VALUE_DB_UP_B = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyFlatnessReturn.VALUE_DB_UP_B))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyFlatnessReturn.VALUE_DB_UP_B;
		setting.unit        = "dB";
		setting.helpText    = "Actually margin value to the average spectral center power of upper section B in each VSA.";
		l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("VALUE_DB_UP_B", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}



	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyFlatnessReturn.CABLE_LOSS_DB[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyFlatnessReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyFlatnessReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_txVerifyFlatnessReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value = (void*)l_txVerifyFlatnessReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	return 0;
}


int LoResultSpectralFlatness11AG( int* carrierNoUpA,
		int* carrierNoUpB,
		int* carrierNoLoA,
		int* carrierNoLoB,
		double* failMargin,
		int* leastMarginCarrier,
		double* loLeakage,
		double* valueDbUpSectionA,
		double* valueDbUpSectionB,
		double* valueDbLoSectionA,
		double* valueDbLoSectionB)
{
	int     result = ERR_OK;

	int     i, lengthIQ = 0;
	double  *dataQ, *dataI, *data;
	double  avg_center_power = 0, avg_power = 0;
	int     numberOfEst = 0;
	int     carrierFirst, carrierSecond, carrierThird, carrierFourth;
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	bool    flatnessPass = true;


	/* for non-analysis cas, initialize margin and loLeakage to NaN*/
	*failMargin    = NA_NUMBER;
	*loLeakage = NA_NUMBER;

	*carrierNoUpA = NA_INTEGER;
	*carrierNoUpB = NA_INTEGER;
	*carrierNoLoA = NA_INTEGER;
	*carrierNoLoB = NA_INTEGER;

	/* for non-analysis cas, initialize valueDbUpSectionA/B and valudDbLoSectionA/B to NaN*/
	*valueDbUpSectionA = NA_NUMBER;
	*valueDbUpSectionB = NA_NUMBER;
	*valueDbLoSectionA = NA_NUMBER;
	*valueDbLoSectionB = NA_NUMBER;

	numberOfEst = ::LP_GetVectorMeasurement("hhEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
	if ( numberOfEst!=64 )	// IQ data length, should be 64
	{
		return -1;
	}
	else
	{
		lengthIQ = numberOfEst;
	}

	dataI    = (double *) malloc (lengthIQ * sizeof (double));
	dataQ    = (double *) malloc (lengthIQ * sizeof (double));
	data     = (double *) malloc (lengthIQ * sizeof (double));

	if (dataQ && dataI && data)
	{
		// Get IQ data.
		for (int carrier=0;carrier<numberOfEst;carrier++)
		{
			dataI[carrier] = bufferReal[carrier];
			dataQ[carrier] = bufferImag[carrier];

			// 1. Convert I and Q to power: P = I*I + Q*Q (the formula is not 100% correct, but log will take out the ratio)
			// 2. Average P
			// 3. avg_power = 10 * log (avg_P)

			// Calculate the power at each point from I & Q data.
			data[carrier] = CalculateIQtoP( dataI[carrier], dataQ[carrier] );
		}

		// IEEE 802.11a/g Transmitter flatness
		// The average energy of the constellations in each of the spectral lines ¡V16.. ¡V1 and +1.. +16 will deviate no
		// more than ¡Ó 2 dB from their average energy. The average energy of the constellations in each of the spectral
		// lines ¡V26.. ¡V17 and +17.. +26 will deviate no more than +2/¡V4 dB from the average energy of spectral lines
		// ¡V16.. ¡V1 and +1.. +16. The data for this test shall be derived from the channel estimation step.

		carrierFirst  = CARRIER_1;
		carrierSecond = CARRIER_16;
		carrierThird  = CARRIER_17;
		carrierFourth = CARRIER_26;

		// Calculate the "avg_center_power" from carriers of -16 to -1 and +1 to +16.
		// Calculate the "avg_power"        from carriers of -26 to -1 and +1 to +26.

		// Calculate the average energy for the spectral lines
		for (i=carrierFirst;i<=carrierSecond;i++)
		{
			avg_center_power += data[i] + data[lengthIQ-i];
		}
		// Continue, calculating the (side) average energy. This one must add the average energy of avg_center_power.
		for (i=carrierThird;i<=carrierFourth;i++)
		{
			avg_power += data[i] + data[lengthIQ-i];
		}
		avg_power = avg_power + avg_center_power;

		avg_center_power = avg_center_power/((carrierSecond-carrierFirst+1)*2);
		if (0!=avg_center_power)
		{
			avg_center_power = 10.0 * log10 (avg_center_power);
		}
		else
		{
			avg_center_power = 0;
		}
		avg_power = avg_power/((carrierFourth-carrierFirst+1)*2);
		if (0!=avg_power)
		{
			avg_power = 10.0 * log10 (avg_power);
		}
		else
		{
			avg_power = 0;
		}

		for (int carrier=0;carrier<lengthIQ;carrier++)
		{
			if (0!=data[carrier])
			{
				data[carrier] = 10.0 * log10 (data[carrier]);
			}
			else
			{
				data[carrier] = 0;
			}
		}

		double deltaPower = 0;
		double highLimit = 2, lowLimit = -2;
		double sectionMargin[4] = {-99, -99, -99, -99};
		int sectionCarrierNo[4];

		// Check the flatness for carrier -16 to -1 and +1 to +16.
		for (i=carrierFirst;i<=carrierSecond;i++)
		{
			deltaPower = data[i]-avg_center_power;
			CheckFlatnessViolation(deltaPower, i, lowLimit, highLimit, valueDbUpSectionA, carrierNoUpA, &sectionMargin[0], &sectionCarrierNo[0]);
			deltaPower = data[lengthIQ-i]-avg_center_power;
			CheckFlatnessViolation(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionA, carrierNoLoA, &sectionMargin[1], &sectionCarrierNo[1]);
		}

		// Continue, Check the flatness -26 to -17 and +17 to +26.
		lowLimit = -4;
		double sectionBFailMargin = 0;
		for (i=carrierThird;i<=carrierFourth;i++)
		{
			deltaPower = data[i]-avg_center_power;
			CheckFlatnessViolation(deltaPower, i, lowLimit, highLimit, valueDbUpSectionB, carrierNoUpB, &sectionMargin[2], &sectionCarrierNo[2]);
			deltaPower = data[lengthIQ-i]-avg_center_power;
			CheckFlatnessViolation(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionB, carrierNoLoB, &sectionMargin[3], &sectionCarrierNo[3]);
		}

		// get the highest fail margin
		*failMargin = -99;
		for (i=0 ; i<4 ; i++)
		{
			if (sectionMargin[i]>*failMargin)
			{
				*failMargin = sectionMargin[i];
				*leastMarginCarrier = sectionCarrierNo[i];
			}
		}

		// Get the center frequency leakage, data[0] is the center frequency power
		// minus "10 * log10((double)((carrierFourth-carrierFirst+1)*2))" for calculate dBc measurement
		*loLeakage = data[0] - avg_power - 10 * log10((double)((carrierFourth-carrierFirst+1)*2));

#if defined(_DEBUG)
		// print out the Mask into file for debug
		FILE *fp;
		fopen_s(&fp, "log/Log_Flatness_Result_11AG.csv", "w");
		if (fp)
		{
			fprintf(fp, "WiFi Flatness Carrier,Signal,Avg_Center_Power,Size: %d\n", lengthIQ);    // print to log file
			for (int carrier=0;carrier<lengthIQ;carrier++)
			{
				fprintf( fp, "%d,%8.2f,%8.2f\n", carrier, data[carrier], avg_center_power);    // print to log file
			}
			fclose(fp);
		}
		else
		{
			// fopen failed
		}
#endif
	}
	else
	{
		// if (dataQ && dataI && data)
		result = -1;
	}

	if (dataQ)	   free (dataQ);
	if (dataI)	   free (dataI);
	if (data)	   free (data);

	return result;
}

int  LoResultSpectralFlatness11N(int basedOnVsa,
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
		int streamIndex)
// streamIndex = 0, 1, 2, 3 (zero based)
{
	int     result = ERR_OK;

	int     i, lengthIQ;
	double  avg_center_power = 0, avg_power = 0;
	int     numberOfEst = 0, numberOfStream = 0, bandwidthMhz = 20;
	int     carrierFirst, carrierSecond, carrierThird, carrierFourth;
	double  *dataRaw, *data;
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	bool    flatnessPass = true;
	int		vsaIndex = 0;


	/* for non-analysis cas, initialize margin and loLeakage to NaN*/
	*failMargin = NA_NUMBER;
	*loLeakage = NA_NUMBER;

	*carrierNoUpA = NA_INTEGER;
	*carrierNoUpB = NA_INTEGER;
	*carrierNoLoA = NA_INTEGER;
	*carrierNoLoB = NA_INTEGER;

	/* for non-analysis cas, initialize valuedbUpSectionA/B to NaN*/
	*valueDbUpSectionA = NA_NUMBER;
	*valueDbUpSectionB = NA_NUMBER;
	*valueDbLoSectionA = NA_NUMBER;
	*valueDbLoSectionB = NA_NUMBER;

	vsaIndex = streamIndex;

	//NOTE: Nstreams x Ntones x NRx
	numberOfEst = ::LP_GetVectorMeasurement("channelEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
	if (numberOfEst <= 0)
	{
		return -1;
	}

	numberOfStream = (int)::LP_GetScalarMeasurement("rateInfo_spatialStreams", 0);

	if ((streamIndex >= numberOfStream) && !basedOnVsa)
	{
		return 0;	// special case, need return, but not error.
	}


	bandwidthMhz = (int)::LP_GetScalarMeasurement("rateInfo_bandwidthMhz", 0);
	if ( (wifiMode==WIFI_11N_HT20)&&(bandwidthMhz!=20) )
	{
		return -1;
	}
	else if ( (wifiMode==WIFI_11N_HT40)&&(bandwidthMhz!=40) )
	{
		return -1;
	}
	else
	{
		// IQ data length, should be 64 or 128
		if (bandwidthMhz==20)
		{
			lengthIQ = 64;
		}
		else
		{
			lengthIQ = 128;
		}
	}

	dataRaw  = (double *) malloc (numberOfEst * sizeof (double));
	data     = (double *) malloc (lengthIQ * sizeof (double));

	if (dataRaw && data)
	{
		// 1. Convert I and Q to power: P = I*I + Q*Q (the formula is not 100% correct, but log will take out the ratio)
		// 2. Average P
		// 3. avg_power = 10 * log (avg_P)

		// Calculate the power at each point from I & Q raw data.
		for (int carrier=0;carrier<numberOfEst;carrier++)
		{
			dataRaw[carrier] = CalculateIQtoP( bufferReal[carrier], bufferImag[carrier] );
		}

		// Initial the data[] array
		for (int carrier=0;carrier<lengthIQ;carrier++)
		{
			data[carrier] = 0;
		}

		if(!basedOnVsa)
		{
			// Get data by stream and each VSA.
			for (int carrier=0;carrier<lengthIQ;carrier++)
			{
				for (int i=0;i<g_Tester_Number;i++)
				{
					int numberOfIndex = (lengthIQ*numberOfStream*i)+(numberOfStream*carrier)+streamIndex;
					if (numberOfIndex<numberOfEst)
					{
						data[carrier] = data[carrier] + dataRaw[numberOfIndex];
					}
					else
					{
						break;
					}
				}
			}
		}
		else // based on Vsa
		{
			for (int carrier=0;carrier<lengthIQ;carrier++)
			{
				for (int i=0;i<numberOfStream;i++)
				{
					int numberOfIndex = (lengthIQ*numberOfStream*vsaIndex)+(numberOfStream*carrier)+i;
					if (numberOfIndex<numberOfEst)
					{
						data[carrier] = data[carrier] + dataRaw[numberOfIndex];
					}
					else
					{
						break;
					}
				}
			}

		}




		// IEEE std 802.11n Transmitter flatness
		// HT20
		// In a 20 MHz channel and in corresponding 20 MHz transmission in a 40 MHz channel, the average energy
		// of the constellations in each of the spectral lines ¡V16 to ¡V1 and +1 to +16 shall deviate no more than ¡Ó 2 dB
		// from their average energy. The average energy of the constellations in each of the spectral lines ¡V28 to ¡V17
		// and +17 to +28 shall deviate no more than +2/¡V4 dB from the average energy of spectral lines ¡V16 to ¡V1 and +1 to +16.

		// HT40
		// In a 40 MHz transmission (excluding HT duplicate format and non-HT duplicate format) the average energy
		// of the constellations in each of the spectral lines ¡V42 to ¡V2 and +2 to +42 shall deviate no more than ¡Ó 2 dB
		// from their average energy. The average energy of the constellations in each of the spectral lines ¡V43 to ¡V58
		// and +43 to +58 shall deviate no more than +2/¡V4 dB from the average energy of spectral lines ¡V42 to ¡V2 and +2 to +42.

		// In HT duplicate format and non-HT duplicate format the average energy of the constellations in each of the
		// spectral lines -42 to -33, -31 to -6, +6 to +31, and +33 to +42 shall deviate no more than ¡Ó 2 dB from their
		// average energy. The average energy of the constellations in each of the spectral lines -43 to -58 and +43 to
		// +58 shall deviate no more than +2/-4 dB from the average energy of spectral lines -42 to -33, -31 to -6, +6
		// to +31, and +33 to +42.

		if (lengthIQ==64)	// HT20 mode
		{
			carrierFirst  = CARRIER_1;
			carrierSecond = CARRIER_16;
			carrierThird  = CARRIER_17;
			carrierFourth = CARRIER_28;
		}
		else				// HT40 mode, lengthIQ=128
		{
			carrierFirst  = CARRIER_2;
			carrierSecond = CARRIER_42;
			carrierThird  = CARRIER_43;
			carrierFourth = CARRIER_58;
		}


		/* Carrier number report order:
		 * 20MHz: +1 ~ +32, -32 ~ -1
		 * 40MHz: +1 ~ +64, -64 ~ -1
		 */
		// Calculate the average energy for the spectral lines
		for ( i=carrierFirst;i<=carrierSecond;i++)
		{
			avg_center_power = avg_center_power + data[i] + data[lengthIQ-i];
		}
		// Continue, calculating the (side) average energy. This one must add the average energy of avg_center_power.
		for (i=carrierThird;i<=carrierFourth;i++)
		{
			avg_power = avg_power + data[i] + data[lengthIQ-i];
		}
		avg_power = avg_power + avg_center_power;

		avg_center_power = avg_center_power/((carrierSecond-carrierFirst+1)*2);
		if (0!=avg_center_power)
		{
			avg_center_power = 10.0 * log10 (avg_center_power);
		}
		else
		{
			avg_center_power= 0;
		}
		avg_power = avg_power/((carrierFourth-carrierFirst+1)*2);
		if (0!=avg_power)
		{
			avg_power = 10.0 * log10 (avg_power);
		}
		else
		{
			avg_power = 0;
		}

		for (int carrier=0;carrier<lengthIQ;carrier++)
		{
			if (0!=data[carrier])
			{
				data[carrier] = 10.0 * log10 (data[carrier]);
			}
			else
			{
				data[carrier] = 0;
			}
		}

		// Check the flatness
		double deltaPower = 0;
		double highLimit = 2, lowLimit = -2;
		double sectionMargin[4] = {-99, -99, -99, -99};
		int sectionCarrierNo[4];

		for (i=carrierFirst;i<=carrierSecond;i++)
		{
			//UPPER SECTION A
			deltaPower = data[i]-avg_center_power;
			CheckFlatnessViolation(deltaPower, i, lowLimit, highLimit, valueDbUpSectionA, carrierNoUpA, &sectionMargin[0], &sectionCarrierNo[0]);

			//LOWER SECTION A
			deltaPower = data[lengthIQ-i]-avg_center_power;
			CheckFlatnessViolation(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionA, carrierNoLoA, &sectionMargin[1], &sectionCarrierNo[1]);
		}

		// Continue, Check the flatness. (side)
		lowLimit = -4;
		for (i=carrierThird;i<=carrierFourth;i++)
		{
			//UPPER SECTION B
			deltaPower = data[i]-avg_center_power;
			CheckFlatnessViolation(deltaPower, i, lowLimit, highLimit, valueDbUpSectionB, carrierNoUpB, &sectionMargin[2], &sectionCarrierNo[2]);

			//LOWER SECTION B
			deltaPower = data[lengthIQ-i]-avg_center_power;
			CheckFlatnessViolation(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionB, carrierNoLoB, &sectionMargin[3], &sectionCarrierNo[3]);
		}

		// get the highest fail margin
		*failMargin = -99;
		for (i=0 ; i<4 ; i++)
		{
			if (sectionMargin[i]>*failMargin)
			{
				*failMargin = sectionMargin[i];
				*leastMarginCarrier = sectionCarrierNo[i];
			}
		}

		// Get the center frequency leakage
		*loLeakage = data[0] - avg_power - 10 * log10((double)((carrierFourth-carrierFirst+1)*2));

#if defined(_DEBUG)
		// print out the Mask into file for debug
		FILE *fp;
		char sigFileName[MAX_BUFFER_SIZE];
		sprintf_s(sigFileName, "log/Log_Flatness_Result_11N_Stream%02d.csv", streamIndex+1);
		fopen_s(&fp, sigFileName, "w");
		if (fp)
		{
			fprintf(fp, "WiFi Flatness Carrier,Signal,Avg_Center_Power,Size: %d\n", lengthIQ);    // print to log file
			for (int carrier=0;carrier<lengthIQ;carrier++)
			{
				fprintf( fp, "%d,%8.2f,%8.2f\n", carrier, data[carrier], avg_center_power);    // print to log file
			}
			fclose(fp);
		}
		else
		{
			// fopen failed
		}
#endif
	}
	else
	{
		// if (dataQ && dataI && data && maxData && minData)
		result = -1;
	}

	if (dataRaw)   free (dataRaw);
	if (data)	   free (data);

	return result;
}

double CalculateIQtoP(double data_i, double data_q)
{
	return (data_i*data_i + data_q*data_q);
}


//-------------------------------------------------------------------------------------
// This is a function for checking the input parameters before the test.
//
//-------------------------------------------------------------------------------------
int CheckTxSpectrumParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *samplingTimeUs, double *cableLossDb, double *peakToAvgRatio, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		// Checking the Input Parameters
		err = TM_WiFiConvertFrequencyToChannel(l_txVerifyFlatnessParam.FREQ_MHZ, channel);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_txVerifyFlatnessParam.FREQ_MHZ);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}

		err = TM_WiFiConvertDataRateNameToIndex(l_txVerifyFlatnessParam.DATA_RATE, &dummyInt);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_txVerifyFlatnessParam.DATA_RATE);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}

		if ( 0!=strcmp(l_txVerifyFlatnessParam.BANDWIDTH, "HT20") && 0!=strcmp(l_txVerifyFlatnessParam.BANDWIDTH, "HT40")
				&& 0!=strcmp(l_txVerifyFlatnessParam.BANDWIDTH, "QUAR") && 0!=strcmp(l_txVerifyFlatnessParam.BANDWIDTH, "HALF") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown BANDWIDTH, WiFi bandwidth %s not supported.\n", l_txVerifyFlatnessParam.BANDWIDTH);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txVerifyFlatnessParam.PREAMBLE, "SHORT") && 0!=strcmp(l_txVerifyFlatnessParam.PREAMBLE, "LONG") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_txVerifyFlatnessParam.PREAMBLE);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT_11N, "MIXED") && 0!=strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT_11N, "GREENFIELD") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PACKET_FORMAT_11N, WiFi 11n packet format %s not supported.\n", l_txVerifyFlatnessParam.PACKET_FORMAT_11N);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txVerifyFlatnessParam.GUARD_INTERVAL_11N, "LONG") && 0!=strcmp(l_txVerifyFlatnessParam.GUARD_INTERVAL_11N, "SHORT") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown GUARD_INTERVAL_11N, WiFi 11n guard interval format %s not supported.\n", l_txVerifyFlatnessParam.GUARD_INTERVAL_11N);
			throw logMessage;
		}
		else
		{
			// do nothing
		}

		// Convert parameter
		err = WiFiTestMode(l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.BANDWIDTH, wifiMode, wifiStreamNum);
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
				l_txVerifyFlatnessParam.FREQ_MHZ,
				l_txVerifyFlatnessParam.TX1,
				l_txVerifyFlatnessParam.TX2,
				l_txVerifyFlatnessParam.TX3,
				l_txVerifyFlatnessParam.TX4,
				l_txVerifyFlatnessParam.CABLE_LOSS_DB,
				l_txVerifyFlatnessReturn.CABLE_LOSS_DB,
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
		if (0==l_txVerifyFlatnessParam.SAMPLING_TIME_US)
		{
			if ( *wifiMode==WIFI_11B )
			{
				*samplingTimeUs = g_WiFiGlobalSettingParam.SPECTRUM_DSSS_SAMPLE_INTERVAL_US;
			}
			else if ( *wifiMode==WIFI_11AG )
			{
				if (0==strcmp(l_txVerifyFlatnessParam.BANDWIDTH,"HALF")) {
					*samplingTimeUs = g_WiFiGlobalSettingParam.SPECTRUM_HALF_SAMPLE_INTERVAL_US;
				} else if (0==strcmp(l_txVerifyFlatnessParam.BANDWIDTH,"QUAR")) {
					*samplingTimeUs = g_WiFiGlobalSettingParam.SPECTRUM_QUAR_SAMPLE_INTERVAL_US;
				} else {
					*samplingTimeUs = g_WiFiGlobalSettingParam.SPECTRUM_OFDM_SAMPLE_INTERVAL_US;
				}
			}
			else	// 802.11n
			{
				// TODO
				*samplingTimeUs = g_WiFiGlobalSettingParam.SPECTRUM_OFDM_SAMPLE_INTERVAL_US;
			}
		}
		else	// SAMPLING_TIME_US != 0
		{
			*samplingTimeUs = l_txVerifyFlatnessParam.SAMPLING_TIME_US;
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
				0!=strcmp(l_txVerifyFlatnessParam.BANDWIDTH, g_RecordedParam.BANDWIDTH) ||
				0!=strcmp(l_txVerifyFlatnessParam.DATA_RATE, g_RecordedParam.DATA_RATE) ||
				0!=strcmp(l_txVerifyFlatnessParam.PREAMBLE,  g_RecordedParam.PREAMBLE) ||
				0!=strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT_11N, g_RecordedParam.PACKET_FORMAT_11N) ||
				0!=strcmp(l_txVerifyFlatnessParam.GUARD_INTERVAL_11N, g_RecordedParam.GUARD_INTERVAL_11N) ||
				l_txVerifyFlatnessParam.CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
				l_txVerifyFlatnessParam.CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
				l_txVerifyFlatnessParam.CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
				l_txVerifyFlatnessParam.CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
				l_txVerifyFlatnessParam.FREQ_MHZ!=g_RecordedParam.FREQ_MHZ ||
				l_txVerifyFlatnessParam.TX_POWER_DBM!=g_RecordedParam.POWER_DBM ||
				l_txVerifyFlatnessParam.TX1!=g_RecordedParam.ANT1 ||
				l_txVerifyFlatnessParam.TX2!=g_RecordedParam.ANT2 ||
				l_txVerifyFlatnessParam.TX3!=g_RecordedParam.ANT3 ||
				l_txVerifyFlatnessParam.TX4!=g_RecordedParam.ANT4)
		{
			g_dutConfigChanged = true;
		}
		else
		{
			g_dutConfigChanged = false;
		}

		// Save the current setup
		g_RecordedParam.ANT1					= l_txVerifyFlatnessParam.TX1;
		g_RecordedParam.ANT2					= l_txVerifyFlatnessParam.TX2;
		g_RecordedParam.ANT3					= l_txVerifyFlatnessParam.TX3;
		g_RecordedParam.ANT4					= l_txVerifyFlatnessParam.TX4;
		g_RecordedParam.CABLE_LOSS_DB[0]		= l_txVerifyFlatnessParam.CABLE_LOSS_DB[0];
		g_RecordedParam.CABLE_LOSS_DB[1]		= l_txVerifyFlatnessParam.CABLE_LOSS_DB[1];
		g_RecordedParam.CABLE_LOSS_DB[2]		= l_txVerifyFlatnessParam.CABLE_LOSS_DB[2];
		g_RecordedParam.CABLE_LOSS_DB[3]		= l_txVerifyFlatnessParam.CABLE_LOSS_DB[3];
		g_RecordedParam.FREQ_MHZ				= l_txVerifyFlatnessParam.FREQ_MHZ;
		g_RecordedParam.POWER_DBM				= l_txVerifyFlatnessParam.TX_POWER_DBM;

		sprintf_s(g_RecordedParam.BANDWIDTH, MAX_BUFFER_SIZE, l_txVerifyFlatnessParam.BANDWIDTH);
		sprintf_s(g_RecordedParam.DATA_RATE, MAX_BUFFER_SIZE, l_txVerifyFlatnessParam.DATA_RATE);
		sprintf_s(g_RecordedParam.PREAMBLE,  MAX_BUFFER_SIZE, l_txVerifyFlatnessParam.PREAMBLE);
		sprintf_s(g_RecordedParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, l_txVerifyFlatnessParam.PACKET_FORMAT_11N);
		sprintf_s(g_RecordedParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, l_txVerifyFlatnessParam.GUARD_INTERVAL_11N);


		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] CheckTxSpectrumParameters() Confirmed.\n");
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

int WiFi_TX_Verify_Spectrum_2010ExtTest()
{
	int    err = ERR_OK;

	bool   analysisOK = false, captureOK = false, enableMultiCapture = false;
	int    channel = 0, HT40ModeOn = 0, packetFormat = 0;
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

		err = CheckTxSpectrumParameters( &channel, &wifiMode, &wifiStreamNum, &samplingTimeUs, &cableLossDb, &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters CheckTxSpectrumParameters() return OK.\n");
		}

		// Modified /* -cfy@sunnyvale, 2012/3/13- */
		// Considering stream number while calculating chain gain for composite measurement so as to set the proper Rx amplitude in tester
		int antennaNum = l_txVerifyFlatnessParam.TX1 + l_txVerifyFlatnessParam.TX2 + l_txVerifyFlatnessParam.TX3 + l_txVerifyFlatnessParam.TX4;
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

		if (0==strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT_11N, "MIXED"))
			packetFormat = 1;
		else if (0 == strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT_11N, "GREENFIELD"))
			packetFormat = 2;
		else
			packetFormat = 0;

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
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_txVerifyFlatnessParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_txVerifyFlatnessParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_txVerifyFlatnessParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",      l_txVerifyFlatnessParam.FREQ_MHZ);
		}

		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",		  l_txVerifyFlatnessParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_txVerifyFlatnessParam.PACKET_FORMAT_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N", l_txVerifyFlatnessParam.GUARD_INTERVAL_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",		  l_txVerifyFlatnessParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",		  l_txVerifyFlatnessParam.BANDWIDTH);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		  HT40ModeOn);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",				  l_txVerifyFlatnessParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",				  l_txVerifyFlatnessParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",				  l_txVerifyFlatnessParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",				  l_txVerifyFlatnessParam.TX4);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "SAMPLING_TIME_US",  samplingTimeUs); // -cfy@sunnyvale, 2012/3/13-
		// -cfy@sunnyvale, 2012/3/13-
		// Added GET_TX_TARGET_POWER feature for AR6004. When TX_POWER_DBM is set to -99, default target power will be applied.
		if ( (l_txVerifyFlatnessParam.TX_POWER_DBM == TX_TARGET_POWER_FLAG) )
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
				l_txVerifyFlatnessParam.TX_POWER_DBM = tx_target_power;
				g_last_TxPower_dBm_Record = l_txVerifyFlatnessParam.TX_POWER_DBM;
			}
			else
			{
				l_txVerifyFlatnessParam.TX_POWER_DBM = g_last_TxPower_dBm_Record;
			}
		}
		else
		{
			// do nothing
		}
		/* <><~~ */

		vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM",	  l_txVerifyFlatnessParam.TX_POWER_DBM);
		TM_SetDoubleParameter(g_WiFi_Test_ID, "TX_POWER_DBM", l_txVerifyFlatnessParam.TX_POWER_DBM); // -cfy@sunnyvale, 2012/3/13-

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
				err = ::LP_IQ2010EXT_InitiateMultiCapture(  l_txVerifyFlatnessParam.DATA_RATE,
						l_txVerifyFlatnessParam.FREQ_MHZ,
						l_txVerifyFlatnessParam.TX_POWER_DBM-cableLossDb+chainGainDb,
						(int)samplingTimeUs,
						g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
						1,
						g_WiFiGlobalSettingParam.VSA_PORT,
						l_txVerifyFlatnessParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
						);
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to new initiate MultiCapture at %d MHz.\n", l_txVerifyFlatnessParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_InitiateMultiCapture() at %d MHz return OK.\n", l_txVerifyFlatnessParam.FREQ_MHZ);
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

#pragma region Setup LP Tester and Capture

/*---------------------------*
		 * Start IQ2010_Ext Function *
		 *---------------------------*/
		
			if ( enableMultiCapture==false )
			{
				/*-------------------------------*
				 * Perform IQ2010Ext VSA capture *
				 *-------------------------------*/
				err = ::LP_IQ2010EXT_InitiateMultiCapture(  l_txVerifyFlatnessParam.DATA_RATE,
						l_txVerifyFlatnessParam.FREQ_MHZ,
						l_txVerifyFlatnessParam.TX_POWER_DBM-cableLossDb+chainGainDb,
						(int)samplingTimeUs,
						g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
						1,
						g_WiFiGlobalSettingParam.VSA_PORT,
						l_txVerifyFlatnessParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
						);
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to new initiate MultiCapture at %d MHz.\n", l_txVerifyFlatnessParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_InitiateMultiCapture() at %d MHz return OK.\n", l_txVerifyFlatnessParam.FREQ_MHZ);
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
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_SaveAlways", l_txVerifyFlatnessParam.FREQ_MHZ, l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.BANDWIDTH);
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
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Verify_Spectrum_Failed", l_txVerifyFlatnessParam.FREQ_MHZ, l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.BANDWIDTH);
						WiFiSaveSigFile(sigFileNameBuffer);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_AnalyzePower(%d) return error.\n", l_txVerifyFlatnessParam.FREQ_MHZ);
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
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Verify_Spectrum_Failed", l_txVerifyFlatnessParam.FREQ_MHZ, l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.BANDWIDTH);
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
					if( (powerMeasured > (l_txVerifyFlatnessParam.TX_POWER_DBM + g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB - cableLossDb)) ||
							(powerMeasured < (l_txVerifyFlatnessParam.TX_POWER_DBM - g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB - cableLossDb))
					  )
					{

						//Skip 0 count and capture 1 packet
						err = ::LP_IQ2010EXT_InitiateMultiCapture(  l_txVerifyFlatnessParam.DATA_RATE,
								l_txVerifyFlatnessParam.FREQ_MHZ,
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
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to new initiate MultiCapture at %d MHz.\n", l_txVerifyFlatnessParam.FREQ_MHZ);
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_InitiateMultiCapture() at %d MHz return OK.\n", l_txVerifyFlatnessParam.FREQ_MHZ);
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
					dutTxPower = l_txVerifyFlatnessParam.TX_POWER_DBM;
				}

				err = ::LP_IQ2010EXT_InitiateMultiCapture(  l_txVerifyFlatnessParam.DATA_RATE,
						l_txVerifyFlatnessParam.FREQ_MHZ,
						dutTxPower-cableLossDb+chainGainDb,
						(int)samplingTimeUs,
						g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
						1,
						g_WiFiGlobalSettingParam.VSA_PORT,
						dutTxPower-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
						);

				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to new initiate MultiCapture at %d MHz.\n", l_txVerifyFlatnessParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_InitiateMultiCapture() at %d MHz return OK.\n", l_txVerifyFlatnessParam.FREQ_MHZ);
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

#pragma region Analysis
			/*-------------------------------*
			 *  IQ2010Ext Spectrum Analysis  *
			 *-------------------------------*/
			double dloLeakageMin = 0.0, dloLeakageMax = 0.0;
			char cFunctName[MAX_BUFFER_SIZE] = {'\0'};
			if( wifiMode == WIFI_11B )
			{
				//Run ::LP_IQ2010EXT_Analyze80211b to get loLeakageDb
				err = ::LP_IQ2010EXT_Analyze80211b( (IQV_EQ_ENUM)		  g_WiFiGlobalSettingParam.ANALYSIS_11B_EQ_TAPS,
						(IQV_DC_REMOVAL_ENUM) g_WiFiGlobalSettingParam.ANALYSIS_11B_DC_REMOVE_FLAG,
						(IQV_11B_METHOD_ENUM) g_WiFiGlobalSettingParam.ANALYSIS_11B_METHOD_11B
						);
				if ( ERR_OK!=err )
				{
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_Analysis_Failed", l_txVerifyFlatnessParam.FREQ_MHZ, l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_Analyze80211b() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_Analyze80211b() return OK.\n");
				}

			}
			else
			{
				// spectralType can be (0):WIFI_11AG, (1):WIFI_11N_HT20, (2):WIFI_11N_HT40 or (3):WIFI_11N
				err = ::LP_IQ2010EXT_AnalyzeSpectralMeasurement( wifiMode-1 );
				if ( ERR_OK!=err )
				{
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_Analysis_Failed", l_txVerifyFlatnessParam.FREQ_MHZ, l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_AnalyzeSpectralMeasurement(%d) return error.\n", wifiMode-1);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_AnalyzeSpectralMeasurement(%d) return OK.\n", wifiMode-1);
				}
			}



#pragma endregion

#pragma region Retrieve analysis Results
			/*---------------------------------------*
			 *  Retrieve IQ2010EXT Spectrum Results  *
			 *---------------------------------------*/
			double dummyAvg = 0, dummyMax = 0, dummyMin = 0;
			double bufferReal [MAX_BUFFER_SIZE] = {0};
			double bufferImage[MAX_BUFFER_SIZE] = {0};

			analysisOK = true;

			if( wifiMode == WIFI_11B )
			{
				double dloLeakage = NA_NUMBER;
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "loLeakageDb", &dloLeakage, &dummyMin, &dummyMax );

				if ( ERR_OK != err )
				{
					analysisOK = false;
					l_txVerifyFlatnessReturn.LO_LEAKAGE_DB = NA_NUMBER;
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_Analysis_Failed", l_txVerifyFlatnessParam.FREQ_MHZ, l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(loLeakage) return error.\n");
					throw logMessage;
				}
				else
					l_txVerifyFlatnessReturn.LO_LEAKAGE_DB = dloLeakage;
			}
			else
			{
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "LO_LEAKAGE_DBR_VSA1", &l_txVerifyFlatnessReturn.LO_LEAKAGE_DB, &dummyMin, &dummyMax );

				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txVerifyFlatnessReturn.LO_LEAKAGE_DB = NA_NUMBER;
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_Analysis_Failed", l_txVerifyFlatnessParam.FREQ_MHZ, l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetDoubleMeasurements(LO_LEAKAGE_DBR_VSA1) return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(LO_LEAKAGE_DBR_VSA1) return OK.\n");
				}

				err  = ::LP_IQ2010EXT_GetIntMeasurement( "SUBCARRIER_LO_A_VSA1", &l_txVerifyFlatnessReturn.SUBCARRIER_LO_A, 0 ); // Modified /* -cfy@sunnyvale, 2012/3/13- */
				err += ::LP_IQ2010EXT_GetIntMeasurement( "SUBCARRIER_LO_B_VSA1", &l_txVerifyFlatnessReturn.SUBCARRIER_LO_B, 0 ); // Modified /* -cfy@sunnyvale, 2012/3/13- */
				err += ::LP_IQ2010EXT_GetIntMeasurement( "SUBCARRIER_UP_A_VSA1", &l_txVerifyFlatnessReturn.SUBCARRIER_UP_A, 0 ); // Modified /* -cfy@sunnyvale, 2012/3/13- */
				err += ::LP_IQ2010EXT_GetIntMeasurement( "SUBCARRIER_UP_B_VSA1", &l_txVerifyFlatnessReturn.SUBCARRIER_UP_B, 0 ); // Modified /* -cfy@sunnyvale, 2012/3/13- */
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_Analysis_Failed", l_txVerifyFlatnessParam.FREQ_MHZ, l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetIntMeasurement(SUBCARRIER_VSA1) return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetIntMeasurement(SUBCARRIER_VSA1) return OK.\n");
				}

				err  = ::LP_IQ2010EXT_GetDoubleMeasurements( "VALUE_DB_LO_A_VSA1", &l_txVerifyFlatnessReturn.VALUE_DB_LO_A, &dummyMin, &dummyMax ); // Modified /* -cfy@sunnyvale, 2012/3/13- */
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "VALUE_DB_LO_B_VSA1", &l_txVerifyFlatnessReturn.VALUE_DB_LO_B, &dummyMin, &dummyMax ); // Modified /* -cfy@sunnyvale, 2012/3/13- */
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "VALUE_DB_UP_A_VSA1", &l_txVerifyFlatnessReturn.VALUE_DB_UP_A, &dummyMin, &dummyMax ); // Modified /* -cfy@sunnyvale, 2012/3/13- */
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "VALUE_DB_UP_B_VSA1", &l_txVerifyFlatnessReturn.VALUE_DB_UP_B, &dummyMin, &dummyMax ); // Modified /* -cfy@sunnyvale, 2012/3/13- */
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_Analysis_Failed", l_txVerifyFlatnessParam.FREQ_MHZ, l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetDoubleMeasurements(VALUE_DB_VSA1) return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(VALUE_DB_VSA1) return OK.\n");
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
			sprintf_s(l_txVerifyFlatnessReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txVerifyFlatnessReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_txVerifyFlatnessReturn.ERROR_MESSAGE, msg);

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
		ReturnErrorMessage(l_txVerifyFlatnessReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
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

void CheckFlatnessViolation(double deltaPower, int carrier, double lowLimit, double highLimit, double *sectionPower, int *sectionCarrier, double *failMargin, int *sectionCarrier2)
{
	if ( deltaPower>=0 )
	{
		// No matter Failed or OK, we need to save the margin
		if ((deltaPower-highLimit)>(*failMargin))	// Save the margin
		{
			*failMargin = deltaPower-highLimit;
			*sectionPower = deltaPower;
			*sectionCarrier = carrier;
			*sectionCarrier2 = carrier;
		}
	}
	else
	{
		// No matter Failed or OK, we need to save the margin
		if ((lowLimit-deltaPower)>(*failMargin))	// Save the margin
		{
			*failMargin = lowLimit-deltaPower;
			*sectionPower = deltaPower;
			*sectionCarrier = carrier;
			*sectionCarrier2 = carrier;
		}
	}
}
