#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_Test.h"
#include "WiFi_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include <math.h> // fmod on mac

using namespace std;


typedef struct tagDefaultMask
{
	double  freqOffset;
	double  deltaMaskPwr;
} DEFAULT_MASK_STRUCT;

vector<DEFAULT_MASK_STRUCT> l_wifiMask_11b;
vector<DEFAULT_MASK_STRUCT> l_wifiMask_11ag;
vector<DEFAULT_MASK_STRUCT> l_wifiMask_11nHt20;
vector<DEFAULT_MASK_STRUCT> l_wifiMask_11nHt40;

vector<DEFAULT_MASK_STRUCT> l_wifiMask_11pDSRC10A;
vector<DEFAULT_MASK_STRUCT> l_wifiMask_11pDSRC10B;
vector<DEFAULT_MASK_STRUCT> l_wifiMask_11pDSRC10C;
vector<DEFAULT_MASK_STRUCT> l_wifiMask_11pDSRC10D;

vector<DEFAULT_MASK_STRUCT> l_wifiMask_11pQUAR5A;
vector<DEFAULT_MASK_STRUCT> l_wifiMask_11pQUAR5B;
vector<DEFAULT_MASK_STRUCT> l_wifiMask_11pQUAR5C;
vector<DEFAULT_MASK_STRUCT> l_wifiMask_11pQUAR5D;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_txVerifyMaskParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_txVerifyMaskReturnMap;

struct tagParam
{
	// Mandatory Parameters
	int    FREQ_MHZ;                                 /*! The center frequency (MHz). */
	char   BANDWIDTH[MAX_BUFFER_SIZE];               /*! The RF bandwidth to verify Mask. */
	char   DATA_RATE[MAX_BUFFER_SIZE];               /*! The data rate to verify Mask. */
	char   PREAMBLE[MAX_BUFFER_SIZE];                /*! The preamble type of 11B(only). */
	char   PACKET_FORMAT_11N[MAX_BUFFER_SIZE];       /*! The packet format of 11N(only).*/
	char   TX_POWER_CLASS[MAX_BUFFER_SIZE];							 /*! The power class of 11p(only).*/
	char   GUARD_INTERVAL_11N[MAX_BUFFER_SIZE];     /*! The guard interval of 11N(only). */
	double TX_POWER_DBM;                             /*! The output power to verify Mask. */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];           /*! The path loss of test system. */
	double SAMPLING_TIME_US;                         /*! The sampling time to verify Mask. */
	double OBW_PERCENTAGE;							 /*! The percentage of OBW, default = 99% */

	// DUT Parameters
	int    TX1;                                      /*!< DUT TX1 on/off. Default=1(on)  */
	int    TX2;                                      /*!< DUT TX2 on/off. Default=0(off) */
	int    TX3;                                      /*!< DUT TX3 on/off. Default=0(off) */
	int    TX4;                                      /*!< DUT TX4 on/off. Default=0(off) */

	int    ARRAY_HANDLING_METHOD;                   /*! The flag to handle array result. Default: 0; 0: Do nothing, 1: Display result on screen, 2: Display result and log to file */

	char   MASK_TEMPLATE[MAX_BUFFER_SIZE];           /*! Mask template */
} l_txVerifyMaskParam;

struct tagReturn
{
	double TX_POWER_DBM;									/*!< TX power dBm setting */

	// POWER Test Result
	double POWER_AVERAGE_DBM;									/*!< (Average) Average power in dBm. */

	// Mask Test Result
	double MARGIN_DB_POSITIVE[MAX_POSITIVE_SECTION];			/*!< Margin to the mask, normally should be a negative number, if pass. */
	double MARGIN_DB_NEGATIVE[MAX_NEGATIVE_SECTION];			/*!< Margin to the mask, normally should be a negative number, if pass */
	double FREQ_AT_MARGIN_POSITIVE[MAX_POSITIVE_SECTION];		/*!< Point out the frequency offset at margin to the mask */
	double FREQ_AT_MARGIN_NEGATIVE[MAX_NEGATIVE_SECTION];		/*!< Point out the frequency offset at margin to the mask */
	double VIOLATION_PERCENT;									/*!< Percentage which fail the mask */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];						/*!< The path loss of test system. */
	vector <double> *SPECTRUM_RAW_DATA_X_VSA[MAX_TESTER_NUM];	/*!< spectrum X raw data in each VSA.  */
	vector <double> *SPECTRUM_RAW_DATA_Y_VSA[MAX_TESTER_NUM];	/*!< spectrum Y raw data in each VSA.  */

	// OBW Test Result
	double OBW_FREQ_START_MHZ;
	double OBW_FREQ_STOP_MHZ;
	double OBW_MHZ;

	// SBW Test Result
	double SBW_FREQ_START_MHZ;
	double SBW_FREQ_STOP_MHZ;
	double SBW_MHZ;

	char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txVerifyMaskReturn;
#pragma endregion

void ClearTxMaskReturn(void)
{
	l_wifiMask_11b.clear();
	l_wifiMask_11ag.clear();
	l_wifiMask_11nHt20.clear();
	l_wifiMask_11nHt40.clear();

	l_wifiMask_11pDSRC10A.clear();
	l_wifiMask_11pDSRC10B.clear();
	l_wifiMask_11pDSRC10C.clear();
	l_wifiMask_11pDSRC10D.clear();

	l_wifiMask_11pQUAR5A.clear();
	l_wifiMask_11pQUAR5B.clear();
	l_wifiMask_11pQUAR5C.clear();
	l_wifiMask_11pQUAR5D.clear();

	for(int i=0;i<MAX_TESTER_NUM;i++)
	{
		if( NULL!=l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[i] )
		{
			delete l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[i];
			l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[i] = NULL;
		}

		if( NULL!=l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[i] )
		{
			delete l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[i];
			l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[i] = NULL;
		}
	}

	l_txVerifyMaskParamMap.clear();
	l_txVerifyMaskReturnMap.clear();
}

#ifndef WIN32
int initTXVerifyMaskContainers = InitializeTXVerifyMaskContainers();
#endif

// These global variables/functions only for WiFi_TX_Verify_Mask.cpp
int CheckTxMaskParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *samplingTimeUs, double *cableLossDb, double *peakToAvgRatio, char* errorMsg );
int WiFi_TX_Verify_Mask_2010ExtTest(void);

//! WiFi TX Verify Mask
/*!
 * Input Parameters
 *
 *  - Mandatory
 *      -# FREQ_MHZ (double): The center frequency (MHz)
 *      -# DATA_RATE (string): The data rate to verify Mask
 *      -# TX_POWER (double): The power (dBm) DUT is going to transmit at the antenna port
 *
 * Return Values
 *      -# A string that contains all DUT info, such as FW version, driver version, chip revision, etc.
 *      -# A string for error message
 *
 * \return 0 No error occurred
 * \return -1 Error(s) occurred.  Please see the returned error message for details
 */


WIFI_TEST_API int WiFi_TX_Verify_Mask(void)
{
	int    err = ERR_OK;

	bool   analysisOK = false, captureOK = false, enableMultiCapture = false;
	int    avgIteration = 0;
	int    channel = 0, HT40ModeOn = 0;
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
	ClearReturnParameters(l_txVerifyMaskReturnMap);

	/*------------------------*
	 * Respond to QUERY_INPUT *
	 *------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_txVerifyMaskParamMap);
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
		RespondToQueryReturn(l_txVerifyMaskReturnMap);
		return err;
	}
	else
	{
		// do nothing
	}

	/*----------------------------------------------------------------------------------------
	 * Declare vectors for storing test results: vector< double >
	 *-----------------------------------------------------------------------------------------*/
	vector< double >    powerAvEachBurst(g_WiFiGlobalSettingParam.MASK_FFT_AVERAGE);

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
		err = GetInputParameters(l_txVerifyMaskParamMap);
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
		* needed to split WiFi_TX_Verify_2010ExtTest into two to	* 
		* avoid a stack check issue.							*
												*
		*------------------------------------------------------*/
		if ( g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE )
		{
			return WiFi_TX_Verify_Mask_2010ExtTest();
		}

		// Error return of this function is irrelevant
		CheckDutTransmitStatus();

#pragma region Prepare input parameters

		err = CheckTxMaskParameters( &channel, &wifiMode, &wifiStreamNum, &samplingTimeUs, &cableLossDb, &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters CheckTxMaskParameters() return OK.\n");
		}

		// Modified /* -cfy@sunnyvale, 2012/3/13- */
		// Considering stream number while calculating chain gain for composite measurement so as to set the proper Rx amplitude in tester
		int antennaNum = l_txVerifyMaskParam.TX1 + l_txVerifyMaskParam.TX2 + l_txVerifyMaskParam.TX3 + l_txVerifyMaskParam.TX4;
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
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_txVerifyMaskParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_txVerifyMaskParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_txVerifyMaskParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",      l_txVerifyMaskParam.FREQ_MHZ);
		}

		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",		  l_txVerifyMaskParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_txVerifyMaskParam.PACKET_FORMAT_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N",l_txVerifyMaskParam.GUARD_INTERVAL_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",		  l_txVerifyMaskParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",		  l_txVerifyMaskParam.BANDWIDTH);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		  HT40ModeOn);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",				  l_txVerifyMaskParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",				  l_txVerifyMaskParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",				  l_txVerifyMaskParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",				  l_txVerifyMaskParam.TX4);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "SAMPLING_TIME_US",  samplingTimeUs); // -cfy@sunnyvale, 2012/3/13-
		// -cfy@sunnyvale, 2012/3/13-
		// Added GET_TX_TARGET_POWER feature for AR6004. When TX_POWER_DBM is set to -99, default target power will be applied.
		if ( (l_txVerifyMaskParam.TX_POWER_DBM == TX_TARGET_POWER_FLAG) )
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
				l_txVerifyMaskParam.TX_POWER_DBM = tx_target_power;
				g_last_TxPower_dBm_Record = l_txVerifyMaskParam.TX_POWER_DBM;
			}
			else
			{
				l_txVerifyMaskParam.TX_POWER_DBM = g_last_TxPower_dBm_Record;
			}
		}
		else
		{
			// do nothing
		}
		/* <><~~ */

		vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM",	  l_txVerifyMaskParam.TX_POWER_DBM);
		TM_SetDoubleParameter(g_WiFi_Test_ID, "TX_POWER_DBM", l_txVerifyMaskParam.TX_POWER_DBM); // -cfy@sunnyvale, 2012/3/13-

		// -cfy@sunnyvale, 2012/3/13-
		//-----------------------------------------------------
		// check mask template if specified
		//-----------------------------------------------------

		if (strlen(l_txVerifyMaskParam.MASK_TEMPLATE)>0)
		{
			char *ptr, buffer[MAX_BUFFER_SIZE];
			vector<DEFAULT_MASK_STRUCT> *maskTemplate;
			DEFAULT_MASK_STRUCT inputMask;
			int dataRate;

			TM_WiFiConvertDataRateNameToIndex(l_txVerifyMaskParam.DATA_RATE, &dataRate);

			strcpy_s(buffer, l_txVerifyMaskParam.MASK_TEMPLATE);

			// check data rate to use the right mask template
			if (dataRate<=CCK11)
			{
				maskTemplate = &l_wifiMask_11b;
			}
			else if (dataRate<=OFDM54)
			{
				maskTemplate = &l_wifiMask_11ag;
			}
			else if ((dataRate>=HALF3) && (dataRate<=HALF27))		// for 802.11p BW 10MHz
			{
				if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"A"))	// for Tx MAX output power<=0dBm, it defined Class A
				{
					maskTemplate = &l_wifiMask_11pDSRC10A;
				}
				else if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"B"))
				{
					maskTemplate = &l_wifiMask_11pDSRC10B;	// for Tx MAX output 0dBm<power<=10dBm, it defined Class B
				}
				else if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"C"))
				{
					maskTemplate = &l_wifiMask_11pDSRC10C;	// for Tx MAX output 10dBm<power<=20dBm, it defined Class C
				}
				else										// for Tx MAX output power<=28.8dBm, it defined Class D
				{
					maskTemplate = &l_wifiMask_11pDSRC10D;
				}
			}

			else if ((dataRate>=QUAR1_5) && (dataRate<=QUAR13_5))		// for 802.11p BW 5MHz
			{
				if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"A"))
				{
					maskTemplate = &l_wifiMask_11pQUAR5A;	// for Tx MAX output power<=0dBm, it defined Class A
				}
				else if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"B"))
				{
					maskTemplate = &l_wifiMask_11pQUAR5B;	// for Tx MAX output 0dBm<power<=10dBm, it defined Class B
				}
				else if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"C"))
				{
					maskTemplate = &l_wifiMask_11pQUAR5C;	// for Tx MAX output 10dBm<power<=20dBm, it defined Class C
				}
				else
				{
					maskTemplate = &l_wifiMask_11pQUAR5D;	// for Tx MAX output power<=26dBm, it defined Class D
				}
			}
			else
			{
				if (HT40ModeOn)
					maskTemplate = &l_wifiMask_11nHt40;
				else
					maskTemplate = &l_wifiMask_11nHt20;
			}

			// initialize mask template
			maskTemplate->clear();
			inputMask.deltaMaskPwr = 0;
			inputMask.freqOffset = 0;
			maskTemplate->push_back(inputMask);

			// add point to the template
			ptr = strtok(buffer, " \t");
			while(ptr)
			{
				inputMask.freqOffset = atof(ptr);
				ptr = strtok(NULL, " \t");
				if (ptr)
				{
					inputMask.deltaMaskPwr = atof(ptr);
					ptr = strtok(NULL, " \t");
					maskTemplate->push_back(inputMask);
				}
			}
		}
		/* <><~~ */

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

			err = ::LP_SetVsa(  l_txVerifyMaskParam.FREQ_MHZ*1e6,
					l_txVerifyMaskParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGainDb,
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

			/* Implement MASK_FFT_AVERAGE for IQapi #LPTW# cfy,-2010/05/18- */
			/*------------------------------*
			 * Start while loop for average *
			 *------------------------------*/
			avgIteration = 0;

			int    bufferSizeX = 0;
			int    bufferSizeY = 0;
			double dummyMax = 0, dummyMin = 0;
			double avgBufferRealX[MAX_BUFFER_SIZE];
			double avgBufferRealY[MAX_BUFFER_SIZE];

			while ( avgIteration<g_WiFiGlobalSettingParam.MASK_FFT_AVERAGE )
			{

				/*----------------------------*
				 * Perform Normal VSA capture *
				 *----------------------------*/
				double sampleFreqHz = 80e6;
				err = ::LP_VsaDataCapture( samplingTimeUs/1000000, g_WiFiGlobalSettingParam.VSA_TRIGGER_TYPE, sampleFreqHz, HT40ModeOn );
				if( ERR_OK!=err )	// capture is failed
				{
					double quickPower = NA_NUMBER;
					err = QuickCaptureRetry( l_txVerifyMaskParam.FREQ_MHZ, samplingTimeUs, g_WiFiGlobalSettingParam.VSA_TRIGGER_TYPE, HT40ModeOn, &quickPower, logMessage);
					if (ERR_OK!=err)	// QuickCaptureRetry() is failed
					{
						// Fail Capture
						if ( quickPower!=NA_NUMBER )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to capture WiFi signal at %d MHz.\nThe DUT power (without path loss) = %.1f dBm and QuickCaptureRetry() return error.\n", l_txVerifyMaskParam.FREQ_MHZ, quickPower);
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to capture WiFi signal at %d MHz, QuickCaptureRetry() return error.\n", l_txVerifyMaskParam.FREQ_MHZ);
						}
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] The QuickCaptureRetry() at %d MHz return OK.\n", l_txVerifyMaskParam.FREQ_MHZ);
					}
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_VsaDataCapture() at %d MHz return OK.\n", l_txVerifyMaskParam.FREQ_MHZ);
				}
#pragma endregion


				/*--------------*
				 *  Capture OK  *
				 *--------------*/
				captureOK = true;
				if (1==g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
				{
					// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Mask_SaveAlways", l_txVerifyMaskParam.FREQ_MHZ, l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.BANDWIDTH);
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
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_Analysis_Failed", l_txVerifyMaskParam.FREQ_MHZ, l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_AnalyzePower() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_AnalyzePower() return OK.\n");
				}

				/*-----------------------------------------*
				 *  Retrieve and Average analysis Results  *
				 *-----------------------------------------*/
				avgIteration++;
				analysisOK = true;

				// Since the limitation, we assume that all path loss value are very close.

				// P_av_no_gap_all_dBm
				powerAvEachBurst[avgIteration-1] = ::LP_GetScalarMeasurement("P_av_no_gap_all_dBm", 0);

				if ( -99.00 >= powerAvEachBurst[avgIteration-1] )
				{
					analysisOK = false;
					powerAvEachBurst[avgIteration-1] = NA_NUMBER;
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_Analysis_Failed", l_txVerifyMaskParam.FREQ_MHZ, l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(P_av_no_gap_all_dBm) return error.\n");
					throw logMessage;
				}
				else
				{
					//The current code will work correctly with 4x4 MIMO.
					int antenaOrder = 0;
					err = CheckAntennaOrderByStream(1, l_txVerifyMaskParam.TX1, l_txVerifyMaskParam.TX2, l_txVerifyMaskParam.TX3, l_txVerifyMaskParam.TX4, &antenaOrder);
					if ( ERR_OK!=err )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] CheckAntennaOrderByStream() return error.\n");
						throw logMessage;
					}

					powerAvEachBurst[avgIteration-1] = powerAvEachBurst[avgIteration-1] + l_txVerifyMaskParam.CABLE_LOSS_DB[antenaOrder-1];
				}

#pragma region Analysis
				/*----------------*
				 *  FFT Analysis  *
				 *----------------*/
				if (HT40ModeOn)		// HT40 mode
				{
					err = ::LP_AnalyzeHT40Mask();
				}
				else
				{
					// Perform FFT analysis
					err = ::LP_AnalyzeFFT();
				}
				if ( ERR_OK!=err )
				{
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Mask_Analysis_Failed", l_txVerifyMaskParam.FREQ_MHZ, l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Analyze FFT return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Analyze FFT return OK.\n");
				}
#pragma endregion



#pragma region Retrieve analysis Results
				/*-------------------------------*
				 *  Video Averaging the spectrum *
				 *-------------------------------*/
				/* Video Averaging is performed on a point-to-point basis over two or more sweeps,
				   which averaging the display trace point is each display pixel after the log amplifier in traditional SA.
				   Belowing is our implementation as a simulation of VSA for this function. With video averaging,
				   at each display point the new value is averaged in with the previously averaged data. #LPTW# cfy,-2010/05/20- */
				analysisOK = true;

				// Store the result
				double tmpBufferRealX[MAX_BUFFER_SIZE], tmpBufferImagX[MAX_BUFFER_SIZE];
				double tmpBufferRealY[MAX_BUFFER_SIZE], tmpBufferImagY[MAX_BUFFER_SIZE];
				int tmpBufferSizeX = ::LP_GetVectorMeasurement("x", tmpBufferRealX, tmpBufferImagX, MAX_BUFFER_SIZE);
				int tmpBufferSizeY = ::LP_GetVectorMeasurement("y", tmpBufferRealY, tmpBufferImagY, MAX_BUFFER_SIZE);

				if (avgIteration==1) // first capture
				{
					// Capture length is always the same, so just use the first result
					bufferSizeX = tmpBufferSizeX;
					bufferSizeY = tmpBufferSizeY;

					// Direct copy from the first result
					memcpy(avgBufferRealX, tmpBufferRealX, sizeof(tmpBufferRealX));
					memcpy(avgBufferRealY, tmpBufferRealY, sizeof(tmpBufferRealY));
				}
				else // average analysis results each time to save the calculation buffer
				{
					for (int i=0; i<bufferSizeY; i++)
					{
						// We only need to perform the averaging for Y real array because X are fixed and images are empty
						avgBufferRealY[i] = ((avgBufferRealY[i] * (avgIteration-1)) + tmpBufferRealY[i])/(avgIteration);
					}
				}
			}   // End - avgIteration

#pragma region Averaging and Saving Test Result
			/*----------------------------------*
			 * Averaging and Saving Test Result *
			 *----------------------------------*/
			if ( (ERR_OK==err) && captureOK && analysisOK )
			{
				// Average Power test result
				::AverageTestResult(&powerAvEachBurst[0], avgIteration, LOG_10, l_txVerifyMaskReturn.POWER_AVERAGE_DBM, dummyMax, dummyMin);

				// Added GET_TX_TARGET_POWER feature for AR6004. When TX_POWER_DBM is set to -99, default target power will be applied.
				// TBD: 2010 not support for this
				l_txVerifyMaskReturn.TX_POWER_DBM = l_txVerifyMaskParam.TX_POWER_DBM; // -cfy@sunnyvale, 2012/3/13-

			}
			else
			{
				// do nothing
			}
#pragma endregion

			if ( (0>=bufferSizeX)||(0>=bufferSizeY) )
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Can not retrieve FFT result, result length = 0.\n");
				throw logMessage;
			}
			else
			{
				l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[0]->clear();
				l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[0]->clear();

				//Return Mask Raw Data
				for(int i=0;i<bufferSizeX;i++)
				{
					l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[0]->push_back(avgBufferRealX[i]);
				}
				for(int i=0;i<bufferSizeY;i++)
				{
					l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[0]->push_back(avgBufferRealY[i]);
				}

				// Verify MASK
				int pass = VerifyPowerMask( avgBufferRealX, avgBufferRealY, bufferSizeY, wifiMode );

				// Verify OBW
				int startIndex = 0, stopIndex = 0;
				pass = VerifyOBW(avgBufferRealY, bufferSizeY, (l_txVerifyMaskParam.OBW_PERCENTAGE/100), startIndex, stopIndex);

				l_txVerifyMaskReturn.OBW_FREQ_START_MHZ = avgBufferRealX[startIndex] / 1000000;
				l_txVerifyMaskReturn.OBW_FREQ_STOP_MHZ  = avgBufferRealX[stopIndex]  / 1000000;
				l_txVerifyMaskReturn.OBW_MHZ  = l_txVerifyMaskReturn.OBW_FREQ_STOP_MHZ - l_txVerifyMaskReturn.OBW_FREQ_START_MHZ;

				//Verify SBW
				if(wifiMode==WIFI_11B)
				{
					int startIndexSBW = 0, stopIndexSBW = 0;
					double sbwPercentage;
					sbwPercentage = 90.0/100.0;
					pass = VerifyOBW(avgBufferRealY, bufferSizeY, sbwPercentage, startIndexSBW, stopIndexSBW);

					l_txVerifyMaskReturn.SBW_FREQ_START_MHZ = avgBufferRealX[startIndexSBW] / 1000000;
					l_txVerifyMaskReturn.SBW_FREQ_STOP_MHZ  = avgBufferRealX[stopIndexSBW]  / 1000000;
					l_txVerifyMaskReturn.SBW_MHZ  = l_txVerifyMaskReturn.SBW_FREQ_STOP_MHZ - l_txVerifyMaskReturn.SBW_FREQ_START_MHZ;
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
			if ( !g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE )
			{
#if 0// -cfy@sunnyvale, 2012/3/13-
				//Changed SISO spectrum measurement using the same method as in IQnxn
				// Transfer the absolute frequency results to relative frequency offset
				for (int i=0;i<MAX_POSITIVE_SECTION;i++)
				{
					if (l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i]!=NA_NUMBER)
					{
						l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i]-l_txVerifyMaskParam.FREQ_MHZ;
					}
					if (l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i]!=NA_NUMBER)
					{
						l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i]-l_txVerifyMaskParam.FREQ_MHZ;
					}
				}
#endif //0 /* <><~~ */
			}

			sprintf_s(l_txVerifyMaskReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txVerifyMaskReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_txVerifyMaskReturn.ERROR_MESSAGE, msg);

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
		ReturnErrorMessage(l_txVerifyMaskReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
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


	for(int i=0;i<MAX_TESTER_NUM;i++)
	{
		l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[i]->clear();
		l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[i]->clear();
	}


	return err;
}

int WiFi_TX_Verify_Mask_2010ExtTest(void)
{
    int    err = ERR_OK;

	bool   analysisOK = false, captureOK = false, enableMultiCapture = false;
	int    avgIteration = 0;
	int    channel = 0, HT40ModeOn = 0;
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

		err = CheckTxMaskParameters( &channel, &wifiMode, &wifiStreamNum, &samplingTimeUs, &cableLossDb, &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters CheckTxMaskParameters() return OK.\n");
		}

		// Modified /* -cfy@sunnyvale, 2012/3/13- */
		// Considering stream number while calculating chain gain for composite measurement so as to set the proper Rx amplitude in tester
		int antennaNum = l_txVerifyMaskParam.TX1 + l_txVerifyMaskParam.TX2 + l_txVerifyMaskParam.TX3 + l_txVerifyMaskParam.TX4;
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
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_txVerifyMaskParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_txVerifyMaskParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_txVerifyMaskParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",      l_txVerifyMaskParam.FREQ_MHZ);
		}

		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",		  l_txVerifyMaskParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_txVerifyMaskParam.PACKET_FORMAT_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N",l_txVerifyMaskParam.GUARD_INTERVAL_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",		  l_txVerifyMaskParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",		  l_txVerifyMaskParam.BANDWIDTH);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		  HT40ModeOn);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",				  l_txVerifyMaskParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",				  l_txVerifyMaskParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",				  l_txVerifyMaskParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",				  l_txVerifyMaskParam.TX4);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "SAMPLING_TIME_US",  samplingTimeUs); // -cfy@sunnyvale, 2012/3/13-
		// -cfy@sunnyvale, 2012/3/13-
		// Added GET_TX_TARGET_POWER feature for AR6004. When TX_POWER_DBM is set to -99, default target power will be applied.
		if ( (l_txVerifyMaskParam.TX_POWER_DBM == TX_TARGET_POWER_FLAG) )
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
				l_txVerifyMaskParam.TX_POWER_DBM = tx_target_power;
				g_last_TxPower_dBm_Record = l_txVerifyMaskParam.TX_POWER_DBM;
			}
			else
			{
				l_txVerifyMaskParam.TX_POWER_DBM = g_last_TxPower_dBm_Record;
			}
		}
		else
		{
			// do nothing
		}
		/* <><~~ */

		vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM",	  l_txVerifyMaskParam.TX_POWER_DBM);
		TM_SetDoubleParameter(g_WiFi_Test_ID, "TX_POWER_DBM", l_txVerifyMaskParam.TX_POWER_DBM); // -cfy@sunnyvale, 2012/3/13-

		// -cfy@sunnyvale, 2012/3/13-
		//-----------------------------------------------------
		// check mask template if specified
		//-----------------------------------------------------

		if (strlen(l_txVerifyMaskParam.MASK_TEMPLATE)>0)
		{
			char *ptr, buffer[MAX_BUFFER_SIZE];
			vector<DEFAULT_MASK_STRUCT> *maskTemplate;
			DEFAULT_MASK_STRUCT inputMask;
			int dataRate;

			TM_WiFiConvertDataRateNameToIndex(l_txVerifyMaskParam.DATA_RATE, &dataRate);

			strcpy_s(buffer, l_txVerifyMaskParam.MASK_TEMPLATE);

			// check data rate to use the right mask template
			if (dataRate<=CCK11)
			{
				maskTemplate = &l_wifiMask_11b;
			}
			else if (dataRate<=OFDM54)
			{
				maskTemplate = &l_wifiMask_11ag;
			}
			else if ((dataRate>=HALF3) && (dataRate<=HALF27))		// for 802.11p BW 10MHz
			{
				if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"A"))	// for Tx MAX output power<=0dBm, it defined Class A
				{
					maskTemplate = &l_wifiMask_11pDSRC10A;
				}
				else if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"B"))
				{
					maskTemplate = &l_wifiMask_11pDSRC10B;	// for Tx MAX output 0dBm<power<=10dBm, it defined Class B
				}
				else if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"C"))
				{
					maskTemplate = &l_wifiMask_11pDSRC10C;	// for Tx MAX output 10dBm<power<=20dBm, it defined Class C
				}
				else										// for Tx MAX output power<=28.8dBm, it defined Class D
				{
					maskTemplate = &l_wifiMask_11pDSRC10D;
				}
			}

			else if ((dataRate>=QUAR1_5) && (dataRate<=QUAR13_5))		// for 802.11p BW 5MHz
			{
				if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"A"))
				{
					maskTemplate = &l_wifiMask_11pQUAR5A;	// for Tx MAX output power<=0dBm, it defined Class A
				}
				else if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"B"))
				{
					maskTemplate = &l_wifiMask_11pQUAR5B;	// for Tx MAX output 0dBm<power<=10dBm, it defined Class B
				}
				else if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"C"))
				{
					maskTemplate = &l_wifiMask_11pQUAR5C;	// for Tx MAX output 10dBm<power<=20dBm, it defined Class C
				}
				else
				{
					maskTemplate = &l_wifiMask_11pQUAR5D;	// for Tx MAX output power<=26dBm, it defined Class D
				}
			}
			else
			{
				if (HT40ModeOn)
					maskTemplate = &l_wifiMask_11nHt40;
				else
					maskTemplate = &l_wifiMask_11nHt20;
			}

			// initialize mask template
			maskTemplate->clear();
			inputMask.deltaMaskPwr = 0;
			inputMask.freqOffset = 0;
			maskTemplate->push_back(inputMask);

			// add point to the template
			ptr = strtok(buffer, " \t");
			while(ptr)
			{
				inputMask.freqOffset = atof(ptr);
				ptr = strtok(NULL, " \t");
				if (ptr)
				{
					inputMask.deltaMaskPwr = atof(ptr);
					ptr = strtok(NULL, " \t");
					maskTemplate->push_back(inputMask);
				}
			}
		}
		/* <><~~ */

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

				// Implement WIFI TX multi-verification for IQ2010Ext /* #LPTW# cfy,-2010/05/20- */
				/*-------------------------------*
				 * Perform IQ2010Ext VSA capture *
				 *-------------------------------*/
				if ( HT40ModeOn==1 )	// HT40 mode on
				{
					err = ::LP_IQ2010EXT_InitiateMultiCaptureHT40(  l_txVerifyMaskParam.DATA_RATE,
							l_txVerifyMaskParam.FREQ_MHZ,
							l_txVerifyMaskParam.TX_POWER_DBM-cableLossDb+chainGainDb,
							(int)samplingTimeUs,
							g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
							g_WiFiGlobalSettingParam.MASK_FFT_AVERAGE,
							g_WiFiGlobalSettingParam.VSA_PORT,
							l_txVerifyMaskParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
							);
				}
				else
				{
					err = ::LP_IQ2010EXT_InitiateMultiCapture(  l_txVerifyMaskParam.DATA_RATE,
							l_txVerifyMaskParam.FREQ_MHZ,
							l_txVerifyMaskParam.TX_POWER_DBM-cableLossDb+chainGainDb,
							(int)samplingTimeUs,
							g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
							g_WiFiGlobalSettingParam.MASK_FFT_AVERAGE,
							g_WiFiGlobalSettingParam.VSA_PORT,
							l_txVerifyMaskParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
							);
				}
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to new initiate MultiCapture at %d MHz.\n", l_txVerifyMaskParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_InitiateMultiCapture() at %d MHz return OK.\n", l_txVerifyMaskParam.FREQ_MHZ);
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
		

// Implement WIFI TX multi-verification for IQ2010Ext /* #LPTW# cfy,-2010/05/20- */
		/*---------------------------*
		 * Start IQ2010_Ext Function *
		 *---------------------------*/
		
			if ( enableMultiCapture==false )
			{
				/*-------------------------------*
				 * Perform IQ2010Ext VSA capture *
				 *-------------------------------*/
				if ( HT40ModeOn==1 )	// HT40 mode on
				{
					err = ::LP_IQ2010EXT_InitiateMultiCaptureHT40(  l_txVerifyMaskParam.DATA_RATE,
							l_txVerifyMaskParam.FREQ_MHZ,
							l_txVerifyMaskParam.TX_POWER_DBM-cableLossDb+chainGainDb,
							(int)samplingTimeUs,
							g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
							g_WiFiGlobalSettingParam.MASK_FFT_AVERAGE,
							g_WiFiGlobalSettingParam.VSA_PORT,
							l_txVerifyMaskParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
							);
				}
				else
				{
					err = ::LP_IQ2010EXT_InitiateMultiCapture(  l_txVerifyMaskParam.DATA_RATE,
							l_txVerifyMaskParam.FREQ_MHZ,
							l_txVerifyMaskParam.TX_POWER_DBM-cableLossDb+chainGainDb,
							(int)samplingTimeUs,
							g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
							g_WiFiGlobalSettingParam.MASK_FFT_AVERAGE,
							g_WiFiGlobalSettingParam.VSA_PORT,
							l_txVerifyMaskParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
							);
				}
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to new initiate MultiCapture at %d MHz.\n", l_txVerifyMaskParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_InitiateMultiCapture() at %d MHz return OK.\n", l_txVerifyMaskParam.FREQ_MHZ);
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
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Mask_SaveAlways", l_txVerifyMaskParam.FREQ_MHZ, l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.BANDWIDTH);
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
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Verify_Mask_Failed", l_txVerifyMaskParam.FREQ_MHZ, l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.BANDWIDTH);
						WiFiSaveSigFile(sigFileNameBuffer);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_AnalyzePower(%d) return error.\n", l_txVerifyMaskParam.FREQ_MHZ);
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
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Verify_Mask_Failed", l_txVerifyMaskParam.FREQ_MHZ, l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.BANDWIDTH);
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
					if( (powerMeasured > (l_txVerifyMaskParam.TX_POWER_DBM + g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB - cableLossDb)) ||
							(powerMeasured < (l_txVerifyMaskParam.TX_POWER_DBM - g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB - cableLossDb))
					  )
					{


						//Skip 0 count and capture 1 packet
						err = ::LP_IQ2010EXT_InitiateMultiCapture(  l_txVerifyMaskParam.DATA_RATE,
								l_txVerifyMaskParam.FREQ_MHZ,
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
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to new initiate MultiCapture at %d MHz.\n", l_txVerifyMaskParam.FREQ_MHZ);
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_InitiateMultiCapture() at %d MHz return OK.\n", l_txVerifyMaskParam.FREQ_MHZ);
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
					dutTxPower =l_txVerifyMaskParam.TX_POWER_DBM;
				}

				if ( HT40ModeOn==1 )	// HT40 mode on
				{
					err = ::LP_IQ2010EXT_InitiateMultiCaptureHT40(  l_txVerifyMaskParam.DATA_RATE,
							l_txVerifyMaskParam.FREQ_MHZ,
							dutTxPower-cableLossDb+chainGainDb,
							(int)samplingTimeUs,
							g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
							1,
							g_WiFiGlobalSettingParam.VSA_PORT,
							dutTxPower-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
							);
				}
				else
				{
					err = ::LP_IQ2010EXT_InitiateMultiCapture(  l_txVerifyMaskParam.DATA_RATE,
							l_txVerifyMaskParam.FREQ_MHZ,
							dutTxPower-cableLossDb+chainGainDb,
							(int)samplingTimeUs,
							g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
							1,
							g_WiFiGlobalSettingParam.VSA_PORT,
							dutTxPower-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
							);
				}


				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to new initiate MultiCapture at %d MHz.\n", l_txVerifyMaskParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_InitiateMultiCapture() at %d MHz return OK.\n", l_txVerifyMaskParam.FREQ_MHZ);
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

			/*----------------------------*
			 *  IQ2010Ext Power Analysis  *
			 *----------------------------*/
			err = ::LP_IQ2010EXT_AnalyzePower();
			if ( ERR_OK!=err )
			{
				// Fail Analysis, thus save capture (Signal File) for debug
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Mask_Analysis_Failed", l_txVerifyMaskParam.FREQ_MHZ, l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_AnalyzePower(%d) return error.\n", l_txVerifyMaskParam.FREQ_MHZ);
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
			//	double dummyAvg = 0,
			double dummyMax = 0, dummyMin = 0;
			//	double bufferReal [MAX_BUFFER_SIZE] = {0};
			//	double bufferImage[MAX_BUFFER_SIZE] = {0};

			analysisOK = true;

			// P_av_no_gap_all_dBm
			err = ::LP_IQ2010EXT_GetDoubleMeasurements( "P_av_no_gap_all_dBm", &l_txVerifyMaskReturn.POWER_AVERAGE_DBM, &dummyMin, &dummyMax );
			if ( ERR_OK!=err )
			{
				analysisOK = false;
				l_txVerifyMaskReturn.POWER_AVERAGE_DBM = NA_NUMBER;
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_Analysis_Failed", l_txVerifyMaskParam.FREQ_MHZ, l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetDoubleMeasurements(P_av_no_gap_all_dBm) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(P_av_no_gap_all_dBm) return OK.\n");
				// Since the limitation, we assume that all path loss value are very close.
				int antenaOrder = 0;
				err = CheckAntennaOrderByStream(1, l_txVerifyMaskParam.TX1, l_txVerifyMaskParam.TX2, l_txVerifyMaskParam.TX3, l_txVerifyMaskParam.TX4, &antenaOrder);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] CheckAntennaOrderByStream() return error.\n");
					throw logMessage;
				}

				l_txVerifyMaskReturn.POWER_AVERAGE_DBM = l_txVerifyMaskReturn.POWER_AVERAGE_DBM + l_txVerifyMaskParam.CABLE_LOSS_DB[antenaOrder-1];
			}

#pragma region Analysis
			/*---------------------------*
			 *  IQ2010Ext MASK Analysis  *
			 *---------------------------*/
			// maskType can be (0):WIFI_11B, (1):WIFI_11AG, (2):WIFI_11N_HT20 or (3):WIFI_11N_HT40
			err = ::LP_IQ2010EXT_AnalyzeMaskMeasurement( wifiMode, l_txVerifyMaskParam.OBW_PERCENTAGE/100 );
			if ( ERR_OK!=err )
			{
				// Fail Analysis, thus save capture (Signal File) for debug
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Mask_Analysis_Failed", l_txVerifyMaskParam.FREQ_MHZ, l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_AnalyzeMaskMeasurement(%d) return error.\n", wifiMode);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_AnalyzeMaskMeasurement(%d) return OK.\n", wifiMode);
			}
#pragma endregion

#pragma region Retrieve analysis Results
			/*-----------------------------------*
			 *  Retrieve IQ2010EXT Mask Results  *
			 *-----------------------------------*/
			analysisOK = true;

			// Store the result
			double bufferRealX[MAX_BUFFER_SIZE];
			double bufferRealY[MAX_BUFFER_SIZE];
			int    numberOfResultSet = 0;
			int    dataSize = 0;

			err = ::LP_IQ2010EXT_GetNumberOfMeasurementElements( "x", 0, &numberOfResultSet );
			if ( numberOfResultSet<=0 )
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Can not retrieve FFT result, result length = 0.\n");
				throw logMessage;
			}
			else
			{
				::LP_IQ2010EXT_GetVectorMeasurement( "x", bufferRealX, MAX_BUFFER_SIZE, &dataSize, 0 );
				::LP_IQ2010EXT_GetVectorMeasurement( "y", bufferRealY, MAX_BUFFER_SIZE, &dataSize, 0 );

				l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[0]->clear();
				l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[0]->clear();

				//Return Mask Raw Data
				for(int i=0;i<numberOfResultSet;i++)
				{
					l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[0]->push_back(bufferRealX[i]);
				}
				for(int i=0;i<numberOfResultSet;i++)
				{
					l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[0]->push_back(bufferRealY[i]);
				}

#if defined(_DEBUG)
				// print out the Mask into file for debug
				FILE *fp1;
				fopen_s(&fp1, "Log_IQ2010Ext_Mask_Result.csv", "w");
				if (fp1)
				{
					fprintf(fp1, "Frequency,Signal,Size: %d\n", numberOfResultSet);    // print to log file
					for (int x=0;x<numberOfResultSet;x++)
					{
						fprintf(fp1, "%8.2f,%8.2f,%8.2f\n", bufferRealX[x], bufferRealY[x]);    // print to log file
					}
					fclose(fp1);
				}

				// Debug only
				// VerifyPowerMask( bufferRealX, bufferRealY, numberOfResultSet, wifiMode );
#endif
			}

			err = ::LP_IQ2010EXT_GetDoubleMeasurements( "VIOLATION_PERCENT_VSA1", &l_txVerifyMaskReturn.VIOLATION_PERCENT, &dummyMin, &dummyMax );
			if ( ERR_OK!=err )
			{
				analysisOK = false;
				l_txVerifyMaskReturn.VIOLATION_PERCENT = NA_NUMBER;
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Mask_Analysis_Failed", l_txVerifyMaskParam.FREQ_MHZ, l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetDoubleMeasurements(VIOLATION_PERCENT_VSA1) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(VIOLATION_PERCENT_VSA1) return OK.\n");
			}

			err = ::LP_IQ2010EXT_GetDoubleMeasurements( "OBW_MHZ_VSA1", &l_txVerifyMaskReturn.OBW_MHZ, &dummyMin, &dummyMax );
			if ( ERR_OK!=err )
			{
				analysisOK = false;
				l_txVerifyMaskReturn.OBW_MHZ = NA_NUMBER;
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Mask_Analysis_Failed", l_txVerifyMaskParam.FREQ_MHZ, l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetDoubleMeasurements(OBW_MHZ_VSA1) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(OBW_MHZ_VSA1) return OK.\n");
			}

			err  = ::LP_IQ2010EXT_GetDoubleMeasurements( "FREQ_MHZ_LO_A_VSA1", &l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[0], &dummyMin, &dummyMax );
			err += ::LP_IQ2010EXT_GetDoubleMeasurements( "FREQ_MHZ_LO_B_VSA1", &l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[1], &dummyMin, &dummyMax );
			err += ::LP_IQ2010EXT_GetDoubleMeasurements( "FREQ_MHZ_UP_A_VSA1", &l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[0], &dummyMin, &dummyMax );
			err += ::LP_IQ2010EXT_GetDoubleMeasurements( "FREQ_MHZ_UP_B_VSA1", &l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[1], &dummyMin, &dummyMax );
			if ( wifiMode!=WIFI_11B )
			{
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "FREQ_MHZ_LO_C_VSA1", &l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[2], &dummyMin, &dummyMax );
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "FREQ_MHZ_LO_D_VSA1", &l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[3], &dummyMin, &dummyMax );
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "FREQ_MHZ_UP_C_VSA1", &l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[2], &dummyMin, &dummyMax );
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "FREQ_MHZ_UP_D_VSA1", &l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[3], &dummyMin, &dummyMax );
			}
			if ( ERR_OK!=err )
			{
				analysisOK = false;
				l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[0] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[1] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[2] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[3] = NA_NUMBER;
				l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[0] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[1] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[2] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[3] = NA_NUMBER;
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Mask_Analysis_Failed", l_txVerifyMaskParam.FREQ_MHZ, l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetDoubleMeasurements(FREQ_MHZ_VSA) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(FREQ_MHZ_VSA) return OK.\n");
			}

			err  = ::LP_IQ2010EXT_GetDoubleMeasurements( "MASK_MARGIN_DB_LO_A_VSA1", &l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[0], &dummyMin, &dummyMax );
			err += ::LP_IQ2010EXT_GetDoubleMeasurements( "MASK_MARGIN_DB_LO_B_VSA1", &l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[1], &dummyMin, &dummyMax );
			err += ::LP_IQ2010EXT_GetDoubleMeasurements( "MASK_MARGIN_DB_UP_A_VSA1", &l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[0], &dummyMin, &dummyMax );
			err += ::LP_IQ2010EXT_GetDoubleMeasurements( "MASK_MARGIN_DB_UP_B_VSA1", &l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[1], &dummyMin, &dummyMax );
			if ( wifiMode!=WIFI_11B )
			{
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "MASK_MARGIN_DB_LO_C_VSA1", &l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[2], &dummyMin, &dummyMax );
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "MASK_MARGIN_DB_LO_D_VSA1", &l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[3], &dummyMin, &dummyMax );
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "MASK_MARGIN_DB_UP_C_VSA1", &l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[2], &dummyMin, &dummyMax );
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "MASK_MARGIN_DB_UP_D_VSA1", &l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[3], &dummyMin, &dummyMax );
			}
			if ( ERR_OK!=err )
			{
				analysisOK = false;
				l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[0] = l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[1] = l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[2] = l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[3] = NA_NUMBER;
				l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[0] = l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[1] = l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[2] = l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[3] = NA_NUMBER;
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Mask_Analysis_Failed", l_txVerifyMaskParam.FREQ_MHZ, l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetDoubleMeasurements(MARGIN_DB_VSA) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(MARGIN_DB_VSA) return OK.\n");
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
			if ( !g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE )
			{
#if 0// -cfy@sunnyvale, 2012/3/13-
				//Changed SISO spectrum measurement using the same method as in IQnxn
				// Transfer the absolute frequency results to relative frequency offset
				for (int i=0;i<MAX_POSITIVE_SECTION;i++)
				{
					if (l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i]!=NA_NUMBER)
					{
						l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i]-l_txVerifyMaskParam.FREQ_MHZ;
					}
					if (l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i]!=NA_NUMBER)
					{
						l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i]-l_txVerifyMaskParam.FREQ_MHZ;
					}
				}
#endif //0 /* <><~~ */
			}

			sprintf_s(l_txVerifyMaskReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txVerifyMaskReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_txVerifyMaskReturn.ERROR_MESSAGE, msg);

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
		ReturnErrorMessage(l_txVerifyMaskReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
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


	for(int i=0;i<MAX_TESTER_NUM;i++)
	{
		l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[i]->clear();
		l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[i]->clear();
	}


	return err;
}

int InitializeTXVerifyMaskContainers(void)
{
	DEFAULT_MASK_STRUCT dummyStruct;

	/*----------------------*
	 *  Construct the Mask  *
	 *----------------------*/
	// Default Mask of 802.11b
	dummyStruct.freqOffset   = 0;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11b.push_back(dummyStruct);
	dummyStruct.freqOffset   = 11;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11b.push_back(dummyStruct);
	dummyStruct.freqOffset   = 11;
	dummyStruct.deltaMaskPwr = -30;
	l_wifiMask_11b.push_back(dummyStruct);
	dummyStruct.freqOffset   = 22;
	dummyStruct.deltaMaskPwr = -30;
	l_wifiMask_11b.push_back(dummyStruct);
	dummyStruct.freqOffset   = 22;
	dummyStruct.deltaMaskPwr = -50;
	l_wifiMask_11b.push_back(dummyStruct);

	// Default Mask of 802.11ag
	dummyStruct.freqOffset   = 0;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11ag.push_back(dummyStruct);
	dummyStruct.freqOffset   = 9;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11ag.push_back(dummyStruct);
	dummyStruct.freqOffset   = 11;
	dummyStruct.deltaMaskPwr = -20;
	l_wifiMask_11ag.push_back(dummyStruct);
	dummyStruct.freqOffset   = 20;
	dummyStruct.deltaMaskPwr = -28;
	l_wifiMask_11ag.push_back(dummyStruct);
	dummyStruct.freqOffset   = 30;
	dummyStruct.deltaMaskPwr = -40;
	l_wifiMask_11ag.push_back(dummyStruct);

	// Default Mask of 802.11n HT20
	dummyStruct.freqOffset   = 0;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11nHt20.push_back(dummyStruct);
	dummyStruct.freqOffset   = 9;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11nHt20.push_back(dummyStruct);
	dummyStruct.freqOffset   = 11;
	dummyStruct.deltaMaskPwr = -20;
	l_wifiMask_11nHt20.push_back(dummyStruct);
	dummyStruct.freqOffset   = 20;
	dummyStruct.deltaMaskPwr = -28;
	l_wifiMask_11nHt20.push_back(dummyStruct);
	dummyStruct.freqOffset   = 30;
	dummyStruct.deltaMaskPwr = -45;
	l_wifiMask_11nHt20.push_back(dummyStruct);

	// Default Mask of 802.11n HT40
	dummyStruct.freqOffset   = 0;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11nHt40.push_back(dummyStruct);
	dummyStruct.freqOffset   = 19;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11nHt40.push_back(dummyStruct);
	dummyStruct.freqOffset   = 21;
	dummyStruct.deltaMaskPwr = -20;
	l_wifiMask_11nHt40.push_back(dummyStruct);
	dummyStruct.freqOffset   = 40;
	dummyStruct.deltaMaskPwr = -28;
	l_wifiMask_11nHt40.push_back(dummyStruct);
	dummyStruct.freqOffset   = 60;
	dummyStruct.deltaMaskPwr = -45;
	l_wifiMask_11nHt40.push_back(dummyStruct);

	// Default Mask of 802.11p DSRC Bandwidth  10MHz class A
	dummyStruct.freqOffset   = 0;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11pDSRC10A.push_back(dummyStruct);
	dummyStruct.freqOffset   = 4.5;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11pDSRC10A.push_back(dummyStruct);
	dummyStruct.freqOffset   = 5;
	dummyStruct.deltaMaskPwr = -10;
	l_wifiMask_11pDSRC10A.push_back(dummyStruct);
	dummyStruct.freqOffset   = 5.5;
	dummyStruct.deltaMaskPwr = -20;
	l_wifiMask_11pDSRC10A.push_back(dummyStruct);
	dummyStruct.freqOffset   = 10;
	dummyStruct.deltaMaskPwr = -28;
	l_wifiMask_11pDSRC10A.push_back(dummyStruct);
	dummyStruct.freqOffset   = 15;
	dummyStruct.deltaMaskPwr = -40;
	l_wifiMask_11pDSRC10A.push_back(dummyStruct);

	// Default Mask of 802.11p DSRC Bandwidth  10MHz class B
	dummyStruct.freqOffset   = 0;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11pDSRC10B.push_back(dummyStruct);
	dummyStruct.freqOffset   = 4.5;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11pDSRC10B.push_back(dummyStruct);
	dummyStruct.freqOffset   = 5;
	dummyStruct.deltaMaskPwr = -16;
	l_wifiMask_11pDSRC10B.push_back(dummyStruct);
	dummyStruct.freqOffset   = 5.5;
	dummyStruct.deltaMaskPwr = -20;
	l_wifiMask_11pDSRC10B.push_back(dummyStruct);
	dummyStruct.freqOffset   = 10;
	dummyStruct.deltaMaskPwr = -28;
	l_wifiMask_11pDSRC10B.push_back(dummyStruct);
	dummyStruct.freqOffset   = 15;
	dummyStruct.deltaMaskPwr = -40;
	l_wifiMask_11pDSRC10B.push_back(dummyStruct);

	// Default Mask of 802.11p DSRC Bandwidth  10MHz class C
	dummyStruct.freqOffset   = 0;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11pDSRC10C.push_back(dummyStruct);
	dummyStruct.freqOffset   = 4.5;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11pDSRC10C.push_back(dummyStruct);
	dummyStruct.freqOffset   = 5;
	dummyStruct.deltaMaskPwr = -26;
	l_wifiMask_11pDSRC10C.push_back(dummyStruct);
	dummyStruct.freqOffset   = 5.5;
	dummyStruct.deltaMaskPwr = -32;
	l_wifiMask_11pDSRC10C.push_back(dummyStruct);
	dummyStruct.freqOffset   = 10;
	dummyStruct.deltaMaskPwr = -40;
	l_wifiMask_11pDSRC10C.push_back(dummyStruct);
	dummyStruct.freqOffset   = 15;
	dummyStruct.deltaMaskPwr = -50;
	l_wifiMask_11pDSRC10C.push_back(dummyStruct);

	// Default Mask of 802.11p DSRC Bandwidth 10MHz class D
	dummyStruct.freqOffset   = 0;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11pDSRC10D.push_back(dummyStruct);
	dummyStruct.freqOffset   = 4.5;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11pDSRC10D.push_back(dummyStruct);
	dummyStruct.freqOffset   = 5;
	dummyStruct.deltaMaskPwr = -35;
	l_wifiMask_11pDSRC10D.push_back(dummyStruct);
	dummyStruct.freqOffset   = 5.5;
	dummyStruct.deltaMaskPwr = -45;
	l_wifiMask_11pDSRC10D.push_back(dummyStruct);
	dummyStruct.freqOffset   = 10;
	dummyStruct.deltaMaskPwr = -55;
	l_wifiMask_11pDSRC10D.push_back(dummyStruct);
	dummyStruct.freqOffset   = 15;
	dummyStruct.deltaMaskPwr = -65;
	l_wifiMask_11pDSRC10D.push_back(dummyStruct);

	// Default Mask of 802.11p Qualter data rate Bandwidth 5MHz class A
	dummyStruct.freqOffset   = 0;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11pQUAR5A.push_back(dummyStruct);
	dummyStruct.freqOffset   = 2.25;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11pQUAR5A.push_back(dummyStruct);
	dummyStruct.freqOffset   = 2.5;
	dummyStruct.deltaMaskPwr = -10;
	l_wifiMask_11pQUAR5A.push_back(dummyStruct);
	dummyStruct.freqOffset   = 2.75;
	dummyStruct.deltaMaskPwr = -20;
	l_wifiMask_11pQUAR5A.push_back(dummyStruct);
	dummyStruct.freqOffset   = 5;
	dummyStruct.deltaMaskPwr = -28;
	l_wifiMask_11pQUAR5A.push_back(dummyStruct);
	dummyStruct.freqOffset   = 7.5;
	dummyStruct.deltaMaskPwr = -40;
	l_wifiMask_11pQUAR5A.push_back(dummyStruct);

	// Default Mask of 802.11p Qualter data rate Bandwidth 5MHz class B
	dummyStruct.freqOffset   = 0;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11pQUAR5B.push_back(dummyStruct);
	dummyStruct.freqOffset   = 2.25;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11pQUAR5B.push_back(dummyStruct);
	dummyStruct.freqOffset   = 2.5;
	dummyStruct.deltaMaskPwr = -16;
	l_wifiMask_11pQUAR5B.push_back(dummyStruct);
	dummyStruct.freqOffset   = 2.75;
	dummyStruct.deltaMaskPwr = -20;
	l_wifiMask_11pQUAR5B.push_back(dummyStruct);
	dummyStruct.freqOffset   = 5;
	dummyStruct.deltaMaskPwr = -28;
	l_wifiMask_11pQUAR5B.push_back(dummyStruct);
	dummyStruct.freqOffset   = 7.5;
	dummyStruct.deltaMaskPwr = -40;
	l_wifiMask_11pQUAR5B.push_back(dummyStruct);


	// Default Mask of 802.11p Qualter data rate Bandwidth 5MHz class C
	dummyStruct.freqOffset   = 0;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11pQUAR5C.push_back(dummyStruct);
	dummyStruct.freqOffset   = 2.25;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11pQUAR5C.push_back(dummyStruct);
	dummyStruct.freqOffset   = 2.5;
	dummyStruct.deltaMaskPwr = -26;
	l_wifiMask_11pQUAR5C.push_back(dummyStruct);
	dummyStruct.freqOffset   = 2.75;
	dummyStruct.deltaMaskPwr = -32;
	l_wifiMask_11pQUAR5C.push_back(dummyStruct);
	dummyStruct.freqOffset   = 5;
	dummyStruct.deltaMaskPwr = -40;
	l_wifiMask_11pQUAR5C.push_back(dummyStruct);
	dummyStruct.freqOffset   = 7.5;
	dummyStruct.deltaMaskPwr = -50;
	l_wifiMask_11pQUAR5C.push_back(dummyStruct);

	// Default Mask of 802.11p Qualter data rate Bandwidth 5MHz class D
	dummyStruct.freqOffset   = 0;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11pQUAR5D.push_back(dummyStruct);
	dummyStruct.freqOffset   = 2.25;
	dummyStruct.deltaMaskPwr = 0;
	l_wifiMask_11pQUAR5D.push_back(dummyStruct);
	dummyStruct.freqOffset   = 2.5;
	dummyStruct.deltaMaskPwr = -35;
	l_wifiMask_11pQUAR5D.push_back(dummyStruct);
	dummyStruct.freqOffset   = 2.75;
	dummyStruct.deltaMaskPwr = -45;
	l_wifiMask_11pQUAR5D.push_back(dummyStruct);
	dummyStruct.freqOffset   = 5;
	dummyStruct.deltaMaskPwr = -55;
	l_wifiMask_11pQUAR5D.push_back(dummyStruct);
	dummyStruct.freqOffset   = 7.5;
	dummyStruct.deltaMaskPwr = -65;
	l_wifiMask_11pQUAR5D.push_back(dummyStruct);

	/*------------------*
	 * Input Parameters  *
	 *------------------*/
	l_txVerifyMaskParamMap.clear();

	WIFI_SETTING_STRUCT setting;

	sprintf_s(l_txVerifyMaskParam.BANDWIDTH, MAX_BUFFER_SIZE, "%s", "HT20");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.BANDWIDTH))    // Type_Checking
	{
		setting.value = (void*)l_txVerifyMaskParam.BANDWIDTH;
		setting.unit        = "";
		setting.helpText    = "Channel bandwidth. Valid options: QUAR, HALF, HT20 or HT40";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BANDWIDTH", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	sprintf_s(l_txVerifyMaskParam.DATA_RATE, MAX_BUFFER_SIZE, "%s", "OFDM-54");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.DATA_RATE))    // Type_Checking
	{
		setting.value = (void*)l_txVerifyMaskParam.DATA_RATE;
		setting.unit        = "";
		setting.helpText    = "Data rate names, such as DSSS-1, CCK-5_5, CCK-11, QUAR-13_5, DSRC-27, OFDM-54, MCS0, MCS15";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_txVerifyMaskParam.PREAMBLE, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.PREAMBLE))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyMaskParam.PREAMBLE;
		setting.unit        = "";
		setting.helpText    = "The preamble type of 11B(only), can be SHORT or LONG, Default=LONG.";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PREAMBLE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_txVerifyMaskParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, "MIXED");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.PACKET_FORMAT_11N))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyMaskParam.PACKET_FORMAT_11N;
		setting.unit        = "";
		setting.helpText    = "The packet format of 11N(only), can be MIXED or GREENFIELD, Default=MIXED.";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT_11N", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_txVerifyMaskParam.TX_POWER_CLASS, MAX_BUFFER_SIZE, "A");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.TX_POWER_CLASS))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyMaskParam.TX_POWER_CLASS;
		setting.unit        = "";
		setting.helpText    = "The power class of 11p(only), can be A, B, C or D, Default=A.";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_CLASS", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_txVerifyMaskParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.GUARD_INTERVAL_11N))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyMaskParam.GUARD_INTERVAL_11N;
		setting.unit        = "";
		setting.helpText    = "The guard interval format of 11N(only), can be LONG or SHORT, Default=LONG.";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL_11N", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyMaskParam.FREQ_MHZ = 2412;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyMaskParam.FREQ_MHZ))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskParam.FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "Channel center frequency in MHz";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_MHZ", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyMaskParam.SAMPLING_TIME_US = 0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyMaskParam.SAMPLING_TIME_US))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskParam.SAMPLING_TIME_US;
		setting.unit        = "us";
		setting.helpText    = "Capture time in micro-seconds";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("SAMPLING_TIME_US", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyMaskParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyMaskParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyMaskParam.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_txVerifyMaskParam.TX_POWER_DBM = 15.0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyMaskParam.TX_POWER_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskParam.TX_POWER_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected power level at DUT antenna port (For AR6004, set -99 for default target power.)";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyMaskParam.OBW_PERCENTAGE = 99.0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyMaskParam.OBW_PERCENTAGE))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskParam.OBW_PERCENTAGE;
		setting.unit        = "%";
		setting.helpText    = "The percentage of power for OBW measurement, default = 99%";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("OBW_PERCENTAGE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyMaskParam.TX1 = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyMaskParam.TX1))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskParam.TX1;
		setting.unit        = "";
		setting.helpText    = "DUT TX path 1 on/off. 1:on; 0:off";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX1", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyMaskParam.TX2 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyMaskParam.TX2))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskParam.TX2;
		setting.unit        = "";
		setting.helpText    = "DUT TX path 2 on/off. 1:on; 0:off";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX2", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyMaskParam.TX3 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyMaskParam.TX3))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskParam.TX3;
		setting.unit        = "";
		setting.helpText    = "DUT TX path 3 on/off. 1:on; 0:off";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX3", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyMaskParam.TX4 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyMaskParam.TX4))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskParam.TX4;
		setting.unit        = "";
		setting.helpText    = "DUT TX path 4 on/off. 1:on; 0:off";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX4", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyMaskParam.ARRAY_HANDLING_METHOD = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyMaskParam.ARRAY_HANDLING_METHOD))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskParam.ARRAY_HANDLING_METHOD;
		setting.unit        = "";
		setting.helpText    = "Use the flag to handle array result. Default: 0; 0: Do nothing, 1: Display result on screen, 2: Display result and log to file";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ARRAY_HANDLING_METHOD", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	// -cfy@sunnyvale, 2012/3/13-
	strcpy_s(l_txVerifyMaskParam.MASK_TEMPLATE, MAX_BUFFER_SIZE, "");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.MASK_TEMPLATE))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyMaskParam.MASK_TEMPLATE;
		setting.unit        = "";
		setting.helpText    = "Mask template, not use if blank";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("MASK_TEMPLATE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	/* <><~~ */

	/*----------------*
	 * Return Values: *
	 * ERROR_MESSAGE  *
	 *----------------*/
	l_txVerifyMaskReturnMap.clear();

	// -cfy@sunnyvale, 2012/3/13-
	l_txVerifyMaskReturn.TX_POWER_DBM = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyMaskReturn.TX_POWER_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskReturn.TX_POWER_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected power level at DUT antenna port.";
		l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	/* <><~~ */

	l_txVerifyMaskReturn.POWER_AVERAGE_DBM = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyMaskReturn.POWER_AVERAGE_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskReturn.POWER_AVERAGE_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Average power of each burst in dBm.";
		l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_AVERAGE_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_POSITIVE_SECTION;i++)
	{
		l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "MASK_MARGIN_DB_UPPER_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Margin to the mask, normally should be a negative number, if pass.";
			l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	for (int i=0;i<MAX_NEGATIVE_SECTION;i++)
	{
		l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "MASK_MARGIN_DB_LOWER_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Margin to the mask, normally should be a negative number, if pass.";
			l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	for (int i=0;i<MAX_POSITIVE_SECTION;i++)
	{
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "FREQ_AT_MARGIN_UPPER_%d", i+1);
			setting.unit        = "MHz";
			setting.helpText    = "Point out the frequency offset at margin to the mask.";
			l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	for (int i=0;i<MAX_NEGATIVE_SECTION;i++)
	{
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "FREQ_AT_MARGIN_LOWER_%d", i+1);
			setting.unit        = "MHz";
			setting.helpText    = "Point out the frequency offset at margin to the mask.";
			l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_txVerifyMaskReturn.VIOLATION_PERCENT = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyMaskReturn.VIOLATION_PERCENT))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskReturn.VIOLATION_PERCENT;
		setting.unit        = "%";
		setting.helpText    = "Percentage which fail the mask";
		l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("VIOLATION_PERCENT", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyMaskReturn.OBW_FREQ_START_MHZ = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyMaskReturn.OBW_FREQ_START_MHZ))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskReturn.OBW_FREQ_START_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "";
		l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("OBW_FREQ_START_MHZ", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyMaskReturn.OBW_FREQ_STOP_MHZ = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyMaskReturn.OBW_FREQ_STOP_MHZ))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskReturn.OBW_FREQ_STOP_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "";
		l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("OBW_FREQ_STOP_MHZ", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyMaskReturn.OBW_MHZ = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyMaskReturn.OBW_MHZ))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskReturn.OBW_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "OBW for the specified power percentage";
		l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("OBW_MHZ", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	//SBW (802.11b only)
	l_txVerifyMaskReturn.SBW_FREQ_START_MHZ = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyMaskReturn.SBW_FREQ_START_MHZ))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskReturn.SBW_FREQ_START_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "802.11b only";
		l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SBW_FREQ_START_MHZ", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyMaskReturn.SBW_FREQ_STOP_MHZ = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyMaskReturn.SBW_FREQ_STOP_MHZ))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskReturn.SBW_FREQ_STOP_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "802.11b only";
		l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SBW_FREQ_STOP_MHZ", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyMaskReturn.SBW_MHZ = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyMaskReturn.SBW_MHZ))    // Type_Checking
	{
		setting.value = (void*)&l_txVerifyMaskReturn.SBW_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "802.11b only";
		l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SBW_MHZ", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	//

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyMaskReturn.CABLE_LOSS_DB[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyMaskReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyMaskReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	for(int i=0;i<MAX_TESTER_NUM;i++)
	{
		l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[i] = new vector<double>();
		l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[i]->clear();
		setting.type = WIFI_SETTING_TYPE_ARRAY_DOUBLE;
		if (0 == l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[i]->size())    // Type_Checking
		{
			setting.value = (void*)l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[i];
			setting.unit        = "Hz";
			setting.helpText    = "Spectrum X raw data.";
			char tmpStr[MAX_BUFFER_SIZE];
			sprintf_s(tmpStr, MAX_BUFFER_SIZE, "RAW_DATA_FREQ_VSA%d", i+1);
			l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tmpStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	for(int i=0;i<MAX_TESTER_NUM;i++)
	{
		l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[i] = new vector<double>();
		l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[i]->clear();
		setting.type = WIFI_SETTING_TYPE_ARRAY_DOUBLE;
		if (0 == l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[i]->size())    // Type_Checking
		{
			setting.value = (void*)l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[i];
			setting.unit        = "dBm/100kHz";
			setting.helpText    = "Spectrum Y raw data.";
			char tmpStr[MAX_BUFFER_SIZE];
			sprintf_s(tmpStr, MAX_BUFFER_SIZE, "RAW_DATA_POWER_VSA%d", i+1);
			l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tmpStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_txVerifyMaskReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value = (void*)l_txVerifyMaskReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	return 0;
}

//=============================================================================
//	Description:
//		A generic function for interpolating. Find y_val when xVal is present.
//
//                             (yMax - yMin)
//    Returns y_val = yMin + (--------------- * (xVal - xMin))
//                             (xMax - xMin)
//=============================================================================
inline double IQInterpolate( double yMax, double yMin, double xMax, double xMin, double xVal )
{
	return (yMin + ((yMax - yMin) / (xMax - xMin) * (xVal - xMin)));
}

int VerifyPowerMask (double *freqBuf, double *maskBuf, int maskBufCnt, int wifiMode)
{
	int passTest = ERR_OK;

	//int    maskTopStart = 999999, maskTopStop = 0;
	double peakPower = NA_NUMBER;
	double *maskWiFi = new double[maskBufCnt];

	double firstRefFreq =  freqBuf[0]/1000000;				// to MHz
	double deltaFreq    = (freqBuf[1]-freqBuf[0])/1000000;	// to MHz


	vector<DEFAULT_MASK_STRUCT>::iterator maskBegin_Iter;
	vector<DEFAULT_MASK_STRUCT>::iterator maskEnd_Iter;
	vector<DEFAULT_MASK_STRUCT>::iterator dummy_Iter;

	/*--------------------------*
	 *  Looking for peak power  *
	 *--------------------------*/
	for (int i=0;i<maskBufCnt;i++)
	{
		if ( maskBuf[i]>peakPower )
		{   // store the current peak power and keep searching
			peakPower = maskBuf[i];
		}
		else
		{
			// keep searching...
		}
	}

	/*-----------------------*
	 *  Select default Mask  *
	 *-----------------------*/
	switch( wifiMode )
	{
		case WIFI_11B:
			maskBegin_Iter = l_wifiMask_11b.begin();
			maskEnd_Iter = l_wifiMask_11b.end();
			break;
		case WIFI_11AG:
			if (0==strcmp(l_txVerifyMaskParam.BANDWIDTH,"HALF")) {
				if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"A")) {
					maskBegin_Iter = l_wifiMask_11pDSRC10A.begin();
					maskEnd_Iter = l_wifiMask_11pDSRC10A.end();
				} else if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"B")) {
					maskBegin_Iter = l_wifiMask_11pDSRC10B.begin();
					maskEnd_Iter = l_wifiMask_11pDSRC10B.end();
				} else if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"C")) {
					maskBegin_Iter = l_wifiMask_11pDSRC10C.begin();
					maskEnd_Iter = l_wifiMask_11pDSRC10C.end();
				} else {
					maskBegin_Iter = l_wifiMask_11pDSRC10D.begin();
					maskEnd_Iter = l_wifiMask_11pDSRC10D.end();
				}
			} else if (0==strcmp(l_txVerifyMaskParam.BANDWIDTH,"QUAR")) {
				if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"A")) {
					maskBegin_Iter = l_wifiMask_11pQUAR5A.begin();
					maskEnd_Iter = l_wifiMask_11pQUAR5A.end();
				} else if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"B")) {
					maskBegin_Iter = l_wifiMask_11pQUAR5B.begin();
					maskEnd_Iter = l_wifiMask_11pQUAR5B.end();
				} else if (0==strcmp(l_txVerifyMaskParam.TX_POWER_CLASS,"C")) {
					maskBegin_Iter = l_wifiMask_11pQUAR5C.begin();
					maskEnd_Iter = l_wifiMask_11pQUAR5C.end();
				} else {
					maskBegin_Iter = l_wifiMask_11pQUAR5D.begin();
					maskEnd_Iter = l_wifiMask_11pQUAR5D.end();
				}
			} else {
				maskBegin_Iter = l_wifiMask_11ag.begin();
				maskEnd_Iter = l_wifiMask_11ag.end();
			}
			break;
		case WIFI_11N_HT20:
			maskBegin_Iter = l_wifiMask_11nHt20.begin();
			maskEnd_Iter = l_wifiMask_11nHt20.end();
			break;
		case WIFI_11N_HT40:
			maskBegin_Iter = l_wifiMask_11nHt40.begin();
			maskEnd_Iter = l_wifiMask_11nHt40.end();
			break;
		default:
			maskBegin_Iter = l_wifiMask_11ag.begin();
			maskEnd_Iter = l_wifiMask_11ag.end();
			break;
	}

	/*-------------------------------*
	 *  Construct the absolute Mask  *
	 *-------------------------------*/
	int    sectionIndex = 0;
	double currentFreq  = 0, xVal = 0, yDeltaPwrdB = 0;

	// Initial the return results
	l_txVerifyMaskReturn.VIOLATION_PERCENT  = 0;
	for (int x=0;x<MAX_POSITIVE_SECTION;x++)
	{
		l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[x]		= NA_NUMBER;
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[x]	= NA_NUMBER;
		l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[x]		= NA_NUMBER;
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[x] = NA_NUMBER;
	}

	currentFreq = firstRefFreq;	 // point to first one

	for (int i=0;i<maskBufCnt;i++)
	{
		xVal = fabs(currentFreq);
		dummy_Iter = maskBegin_Iter;
		sectionIndex = 0;

		while ( dummy_Iter!=maskEnd_Iter ) // not end of mask vector
		{
			if ( xVal >= dummy_Iter->freqOffset )
			{
				dummy_Iter++;
				sectionIndex++;

				if ( dummy_Iter!=maskEnd_Iter ) // not end of mask vector
				{
					if ( xVal < dummy_Iter->freqOffset )    // find out the range
					{
						double xMax = dummy_Iter->freqOffset;
						double yMax = dummy_Iter->deltaMaskPwr;
						dummy_Iter--;
						sectionIndex--;
						double xMin = dummy_Iter->freqOffset;
						double yMin = dummy_Iter->deltaMaskPwr;

						yDeltaPwrdB = IQInterpolate( yMax, yMin, xMax, xMin, xVal );

						//if (dummy_Iter->deltaMaskPwr==0) // This is the top of spectrum
						//{
						//    if (i<maskTopStart) maskTopStart = i;
						//    if (i>maskTopStop)  maskTopStop  = i;
						//}
						//else
						//{
						//    // do nothing, not top
						//}
						break;
					}
					else
					{
						// not in this Mask range, continue and keep searching
						continue;
					}
				}
				else    // already end of vector, thus direct assign mask_Iter->deltaMaskPwr to yDeltaPwrdB
				{
					dummy_Iter--;
					sectionIndex--;
					yDeltaPwrdB = dummy_Iter->deltaMaskPwr;
					break;
				}
			}
			else
			{
				// no this case, because fabs(currentFreq) should >= 0
			}
		}

		// Construct the absolute Mask and store into Array
		maskWiFi[i] = peakPower + yDeltaPwrdB;

		/*---------------------------------*
		 *  Report Margin by each section  *
		 *---------------------------------*/
		if ( sectionIndex==0 )		// This is the "TOP" of spectral, just skip this section.
		{
			// do nothing, moving to next frequency step
		}
		else
		{
			double deltaPower = maskBuf[i] - maskWiFi[i];	// check Mask

			if ( i>(maskBufCnt/2) )		// Upper section
			{
				if ( deltaPower>l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[sectionIndex-1] )	// need to save this as margin
				{
					l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[sectionIndex-1]      = deltaPower;		// save margin
					// -cfy@sunnyvale, 2012/3/13-
					//Changed SISO spectrum measurement using the same method as in IQnxn
					l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[sectionIndex-1] = (freqBuf[i]/1e6);	// save frequency
					/* <><~~ */

				}
				else
				{
					// do nothing
				}
			}
			else						// Lower section
			{
				if ( deltaPower>l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[sectionIndex-1] )	// need to save this as margin
				{
					l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[sectionIndex-1]      = deltaPower;		// save margin
					// -cfy@sunnyvale, 2012/3/13-
					//Changed SISO spectrum measurement using the same method as in IQnxn
					l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[sectionIndex-1] = (freqBuf[i]/1e6);	// save frequency
					/* <><~~ */
				}
				else
				{
					// do nothing
				}
			}

			if ( deltaPower>0 )  // Mask Failed
			{
				l_txVerifyMaskReturn.VIOLATION_PERCENT++;
			}
			else    // Mask Pass
			{
				// keep compare...
			}
		}

		// moving to next frequency step
		currentFreq = currentFreq + deltaFreq;
	}   // end - for (int i=0;i<maskBufCnt;i++)

	if ( wifiMode==WIFI_11B )	// 802.11b only have -2,-1,+1,+2 sections
	{
		l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[0]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[1]);		// save margin
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[0] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[1];	// save frequency
		l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[0]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[1]);		// save margin
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[0] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[1];	// save frequency

		l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[1]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[3]);		// save margin
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[1] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[3];	// save frequency
		l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[1]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[3]);		// save margin
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[1] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[3];	// save frequency

		l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[2]      = NA_NUMBER;
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[2] = NA_NUMBER;
		l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[2]      = NA_NUMBER;
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[2] = NA_NUMBER;

		l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[3]      = NA_NUMBER;
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[3] = NA_NUMBER;
		l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[3]      = NA_NUMBER;
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[3] = NA_NUMBER;

		// -cfy@sunnyvale, 2012/3/13-
		l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[4]      = NA_NUMBER;
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[4] = NA_NUMBER;
		l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[4]      = NA_NUMBER;
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[4] = NA_NUMBER;
		/* <><~~ */
	}
	else
	{
		if (( wifiMode==WIFI_11P_DSRC10 ) || ( wifiMode==WIFI_11P_QUAR5 )) {
			// Turn Values to Positive
			l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[0]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[0]);
			l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[0]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[0]);

			l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[1]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[1]);
			l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[1]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[1]);

			l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[2]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[2]);
			l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[2]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[2]);

			l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[3]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[3]);
			l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[3]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[3]);

			l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[4]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[4]);
			l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[4]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[4]);
		} else {
			// Turn Values to Positive
			l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[0]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[0]);
			l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[0]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[0]);

			l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[1]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[1]);
			l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[1]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[1]);

			l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[2]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[2]);
			l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[2]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[2]);

			l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[3]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[3]);
			l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[3]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[3]);

			l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[4]      = NA_NUMBER;
			l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[4] = NA_NUMBER;
			l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[4]      = NA_NUMBER;
			l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[4] = NA_NUMBER;
		}
	}


	// Percentage, which fail the mask
	l_txVerifyMaskReturn.VIOLATION_PERCENT = (l_txVerifyMaskReturn.VIOLATION_PERCENT/maskBufCnt) * 100.0;

#if defined(_DEBUG)
	// print out the Mask into file for debug
	FILE *fp;
	fopen_s(&fp, "Log_Mask_Result.csv", "w");
	if (fp)
	{
		fprintf(fp, "Frequency,WiFi Mask,Signal,Size: %d\n", maskBufCnt);    // print to log file
		for (int x=0;x<maskBufCnt;x++)
		{
			fprintf(fp, "%8.2f,%8.2f,%8.2f\n", freqBuf[x], maskWiFi[x], maskBuf[x]);    // print to log file
		}
		fclose(fp);
	}
#endif

	// free memory
	delete [] maskWiFi;

	if (l_txVerifyMaskReturn.VIOLATION_PERCENT!=0)
	{
		passTest = -1;
	}
	else
	{
		passTest = ERR_OK;
	}

	return passTest;
}

int VerifyOBW(double *maskBuf, int maskBufCnt, double obwPercentage, int &startIndex, int &stopIndex)
{

	int passTest = ERR_OK;

	double  totalPower = 0;
	//double  obwPowerLimit = 0;
	double  obwPowerExceed = 0.0;
	double  sumPower = 0.0;


	// Step 1, calculate the total power
	for (int i=0;i<maskBufCnt;i++)
	{
		totalPower = totalPower + pow( 10,(maskBuf[i]/10) );
	}

	// Step 2, calculate the exceed part( >99%) by OBW percantage
	obwPowerExceed = totalPower * (1.0 - obwPercentage);
	obwPowerExceed /= 2.0;

	//Step 3, Set flag from left sideband and right sideband
	startIndex = 0;
	stopIndex  = maskBufCnt - 1;

	//Find the point in 99%, start from left sideband, increase frequency
	sumPower = pow( 10,(maskBuf[startIndex]/10) );
	while((sumPower <= obwPowerExceed) && (startIndex < (maskBufCnt/2)))
	{
		startIndex++;
		sumPower += pow( 10,(maskBuf[startIndex]/10) );
	}

	//Find the point in 99%, start from right sideband, decrease frequency
	sumPower = pow( 10,(maskBuf[stopIndex]/10) );
	while((sumPower <= obwPowerExceed) && (stopIndex > (maskBufCnt/2)))
	{
		stopIndex --;
		sumPower += pow( 10,(maskBuf[stopIndex]/10) );
	}

	return passTest;
}


//-------------------------------------------------------------------------------------
// This is a function for checking the input parameters before the test.
//
//-------------------------------------------------------------------------------------
int CheckTxMaskParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *samplingTimeUs, double *cableLossDb, double *peakToAvgRatio, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		// Checking the Input Parameters
		err = TM_WiFiConvertFrequencyToChannel(l_txVerifyMaskParam.FREQ_MHZ, channel);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_txVerifyMaskParam.FREQ_MHZ);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}

		err = TM_WiFiConvertDataRateNameToIndex(l_txVerifyMaskParam.DATA_RATE, &dummyInt);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_txVerifyMaskParam.DATA_RATE);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}

		if ( 0!=strcmp(l_txVerifyMaskParam.BANDWIDTH, "HT20") && 0!=strcmp(l_txVerifyMaskParam.BANDWIDTH, "HT40")
				&& 0!=strcmp(l_txVerifyMaskParam.BANDWIDTH, "QUAR") && 0!=strcmp(l_txVerifyMaskParam.BANDWIDTH, "HALF") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown BANDWIDTH, WiFi bandwidth %s not supported.\n", l_txVerifyMaskParam.BANDWIDTH);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txVerifyMaskParam.PREAMBLE, "SHORT") && 0!=strcmp(l_txVerifyMaskParam.PREAMBLE, "LONG") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_txVerifyMaskParam.PREAMBLE);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txVerifyMaskParam.PACKET_FORMAT_11N, "MIXED") && 0!=strcmp(l_txVerifyMaskParam.PACKET_FORMAT_11N, "GREENFIELD") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PACKET_FORMAT_11N, WiFi 11n packet format %s not supported.\n", l_txVerifyMaskParam.PACKET_FORMAT_11N);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txVerifyMaskParam.TX_POWER_CLASS, "A") && 0!=strcmp(l_txVerifyMaskParam.TX_POWER_CLASS, "B") && 0!=strcmp(l_txVerifyMaskParam.TX_POWER_CLASS, "C") && 0!=strcmp(l_txVerifyMaskParam.TX_POWER_CLASS, "D") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown TX_POWER_CLASS, WiFi 11p power class %s not supported.\n", l_txVerifyMaskParam.TX_POWER_CLASS);
			throw logMessage;
		}

		if ( 0!=strcmp(l_txVerifyMaskParam.GUARD_INTERVAL_11N, "LONG") && 0!=strcmp(l_txVerifyMaskParam.GUARD_INTERVAL_11N, "SHORT") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown GUARD_INTERVAL_11N, WiFi 11n guard interval format %s not supported.\n", l_txVerifyMaskParam.GUARD_INTERVAL_11N);
			throw logMessage;
		}

		// -cfy@sunnyvale, 2012/3/13-
		double testerHWversion =0.0;
		err = ::GetTesterHwVersion (&testerHWversion);
		if(err != ERR_OK)
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Function GetTesterHwVersion() Failed.\n");
			throw logMessage;
		}
		else
		{
			// do nothing /*LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Function GetTesterHwVersion() OK.\n");*/
		}

		if( strcmp(LP_GetTesterName(), "IQView") == 0 )
		{
			if(testerHWversion<1.6)
			{
				if(0==strcmp(l_txVerifyMaskParam.BANDWIDTH, "HT40"))
				{
					if( (l_txVerifyMaskParam.FREQ_MHZ>2457 && l_txVerifyMaskParam.FREQ_MHZ<2900) || l_txVerifyMaskParam.FREQ_MHZ<2442) //based on Qiang Zhao's comment
					{
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] HT40 mask test is not supported at %dMHz with IQview/IQflex/IQnxn.\n", l_txVerifyMaskParam.FREQ_MHZ);
						throw logMessage;
					}
					else
					{
						//do nothing
					}
				}
				else
				{
					//do nothing
				}
			}
			else
			{
				//do nothing
			}
		}
		/* <><~~ */

		// Convert parameter
		err = WiFiTestMode(l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.BANDWIDTH, wifiMode, wifiStreamNum);
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
				l_txVerifyMaskParam.FREQ_MHZ,
				l_txVerifyMaskParam.TX1,
				l_txVerifyMaskParam.TX2,
				l_txVerifyMaskParam.TX3,
				l_txVerifyMaskParam.TX4,
				l_txVerifyMaskParam.CABLE_LOSS_DB,
				l_txVerifyMaskReturn.CABLE_LOSS_DB,
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
		if (0==l_txVerifyMaskParam.SAMPLING_TIME_US)
		{
			if ( *wifiMode==WIFI_11B )
			{
				*samplingTimeUs = g_WiFiGlobalSettingParam.MASK_DSSS_SAMPLE_INTERVAL_US;
			}
			else if ( *wifiMode==WIFI_11AG )
			{
				if (0==strcmp(l_txVerifyMaskParam.BANDWIDTH,"HALF")) {
					*samplingTimeUs = g_WiFiGlobalSettingParam.MASK_HALF_SAMPLE_INTERVAL_US;
				} else if (0==strcmp(l_txVerifyMaskParam.BANDWIDTH,"QUAR")) {
					*samplingTimeUs = g_WiFiGlobalSettingParam.MASK_QUAR_SAMPLE_INTERVAL_US;
				} else {
					*samplingTimeUs = g_WiFiGlobalSettingParam.MASK_OFDM_SAMPLE_INTERVAL_US;
				}
			}
			else	// 802.11n
			{
				// TODO
				*samplingTimeUs = g_WiFiGlobalSettingParam.MASK_OFDM_SAMPLE_INTERVAL_US;
			}
		}
		else	// SAMPLING_TIME_US != 0
		{
			*samplingTimeUs = l_txVerifyMaskParam.SAMPLING_TIME_US;
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
				0!=strcmp(l_txVerifyMaskParam.BANDWIDTH, g_RecordedParam.BANDWIDTH) ||
				0!=strcmp(l_txVerifyMaskParam.DATA_RATE, g_RecordedParam.DATA_RATE) ||
				0!=strcmp(l_txVerifyMaskParam.PREAMBLE,  g_RecordedParam.PREAMBLE) ||
				0!=strcmp(l_txVerifyMaskParam.PACKET_FORMAT_11N, g_RecordedParam.PACKET_FORMAT_11N) ||
				0!=strcmp(l_txVerifyMaskParam.GUARD_INTERVAL_11N, g_RecordedParam.GUARD_INTERVAL_11N) ||
				l_txVerifyMaskParam.CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
				l_txVerifyMaskParam.CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
				l_txVerifyMaskParam.CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
				l_txVerifyMaskParam.CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
				l_txVerifyMaskParam.FREQ_MHZ!=g_RecordedParam.FREQ_MHZ ||
				l_txVerifyMaskParam.TX_POWER_DBM!=g_RecordedParam.POWER_DBM ||
				l_txVerifyMaskParam.TX1!=g_RecordedParam.ANT1 ||
				l_txVerifyMaskParam.TX2!=g_RecordedParam.ANT2 ||
				l_txVerifyMaskParam.TX3!=g_RecordedParam.ANT3 ||
				l_txVerifyMaskParam.TX4!=g_RecordedParam.ANT4)
		{
			g_dutConfigChanged = true;
		}
		else
		{
			g_dutConfigChanged = false;
		}

		// Save the current setup
		g_RecordedParam.ANT1					= l_txVerifyMaskParam.TX1;
		g_RecordedParam.ANT2					= l_txVerifyMaskParam.TX2;
		g_RecordedParam.ANT3					= l_txVerifyMaskParam.TX3;
		g_RecordedParam.ANT4					= l_txVerifyMaskParam.TX4;
		g_RecordedParam.CABLE_LOSS_DB[0]		= l_txVerifyMaskParam.CABLE_LOSS_DB[0];
		g_RecordedParam.CABLE_LOSS_DB[1]		= l_txVerifyMaskParam.CABLE_LOSS_DB[1];
		g_RecordedParam.CABLE_LOSS_DB[2]		= l_txVerifyMaskParam.CABLE_LOSS_DB[2];
		g_RecordedParam.CABLE_LOSS_DB[3]		= l_txVerifyMaskParam.CABLE_LOSS_DB[3];
		g_RecordedParam.FREQ_MHZ				= l_txVerifyMaskParam.FREQ_MHZ;
		g_RecordedParam.POWER_DBM				= l_txVerifyMaskParam.TX_POWER_DBM;

		sprintf_s(g_RecordedParam.BANDWIDTH, MAX_BUFFER_SIZE, l_txVerifyMaskParam.BANDWIDTH);
		sprintf_s(g_RecordedParam.DATA_RATE, MAX_BUFFER_SIZE, l_txVerifyMaskParam.DATA_RATE);
		sprintf_s(g_RecordedParam.PREAMBLE,  MAX_BUFFER_SIZE, l_txVerifyMaskParam.PREAMBLE);
		sprintf_s(g_RecordedParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, l_txVerifyMaskParam.PACKET_FORMAT_11N);
		sprintf_s(g_RecordedParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, l_txVerifyMaskParam.GUARD_INTERVAL_11N);


		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] CheckTxMaskParameters() Confirmed.\n");
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
