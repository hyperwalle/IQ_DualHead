#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_Test.h"
#include "WiFi_Test_Internal.h"
#include "IQmeasure.h"
#include "math.h"
#include "vDUT.h"

using namespace std;


#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_rxSweepPerParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_rxSweepPerReturnMap;

struct tagParam
{
	// Mandatory Parameters
	int    FREQ_MHZ;                                /*! The center frequency (MHz). */
	int    FRAME_COUNT;                             /*! Number of count to send data packet */
	char   BANDWIDTH[MAX_BUFFER_SIZE];              /*! The RF bandwidth to verify EVM. */
	char   DATA_RATE[MAX_BUFFER_SIZE];              /*! The data rate to verify EVM. */
	char   PREAMBLE[MAX_BUFFER_SIZE];               /*! The preamble type of 11B(only). */
	char   PACKET_FORMAT_11N[MAX_BUFFER_SIZE];      /*! The packet format of 11N(only). */
	char   GUARD_INTERVAL_11N[MAX_BUFFER_SIZE];     /*! The guard interval of 11N(only). */
	double CABLE_LOSS_DB[MAX_BUFFER_SIZE];          /*! The path loss of test system. */
	double START_POWER_LEVEL_DBM;                   /*! The PER sweep start power. Default = -65 dBm */
	double STOP_POWER_LEVEL_DBM;                    /*! The PER sweep stop power. Default = -75 dBm */
	double SWEEP_STEP_DB;                           /*! The PER sweep step in dB. Default = -1 dB */
	double SENS_PER_PERCENTAGE;                     /*! The PER fail percentage */
	int    STOP_AT_PER_PERCENTAGE;                  /*! The flag to stop PER sweep test when sensitivity point has been detected. Default = 0 (ignore sensitivity point, and sweep the whole range) */
	int    ARRAY_HANDLING_METHOD;                   /*! The flag to handle array result. Default: 0; 0: Do nothing, 1: Display result on screen, 2: Display result and log to file */
	int    SPIKE_REMOVAL;                           /*! The flag to remove PER spike during sweep. Default: 1 */

	// DUT Parameters
	int    RX1;                                     /*! DUT TX1 on/off. Default=1(on) */
	int    RX2;                                     /*! DUT TX2 on/off. Default=0(off) */
	int    RX3;                                     /*! DUT TX3 on/off. Default=0(off) */
	int    RX4;                                     /*! DUT TX4 on/off. Default=0(off) */
} l_rxSweepPerParam;

struct tagReturn
{
	// PER Test Result
	int    NUMBER_OF_POWER_LEVELS;          /*!< Number of power levels in the sweep test */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];  /*! The path loss of test system. */

	vector <double> *POWER_LEVELS;          /*!< Array to store sweep power levels */
	vector <double> *PER_VALUES;            /*!< Array to store sweep PER values */

	double SENS_POWER_LEVEL_DBM;			/*!< POWER_LEVEL dBm for the PER sweep test. Format: POWER LEVEL */
	char   reserved[MAX_BUFFER_SIZE];
	char   reserved_1[MAX_BUFFER_SIZE];

	char   ERROR_MESSAGE[MAX_BUFFER_SIZE];

} l_rxSweepPerReturn;
#pragma endregion

void ClearRxSweepPerReturn(void)
{

	l_rxSweepPerReturn.POWER_LEVELS->clear();
	l_rxSweepPerReturn.PER_VALUES->clear();

	if( NULL!=l_rxSweepPerReturn.POWER_LEVELS )
	{
		delete l_rxSweepPerReturn.POWER_LEVELS;
		l_rxSweepPerReturn.POWER_LEVELS = NULL;
	}

	if( NULL!=l_rxSweepPerReturn.PER_VALUES )
	{
		delete l_rxSweepPerReturn.PER_VALUES;
		l_rxSweepPerReturn.PER_VALUES = NULL;
	}

	l_rxSweepPerParamMap.clear();
	l_rxSweepPerReturnMap.clear();
}

// These global variables/functions only for WiFi_TX_Verify_Power.cpp
int ConfirmRxSweepPerParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *cableLossDb, char* errorMsg );
int WiFi_RX_Sweep_Per_2010ExtTest(void);

//! WiFi_RX_Sweep_PER
/*!
 * Input Parameters
 *
 *  - None
 *
 * Return Values
 *      -# A string for possible error message
 *
 * \return 0 No error occurred
 * \return -1 DUT failed to insert.  Please see the returned error message for details
 */
WIFI_TEST_API int WiFi_RX_Sweep_Per(void)
{
	int    err = ERR_OK;

	int	   totalPackets = 0;
	int	   goodPackets  = 0;
	int	   badPackets   = 0;

	int	   sensPointIndex = 0;
	bool   bSensFound = false;
	int    channel = 0, HT40ModeOn = 0;
	int    dummyValue = 0;
	int    wifiMode = 0, wifiStreamNum = 0;
	int    packetNumber = 0;
	double cableLossDb  = 0;
	double MaxVSGPowerLimit = 0;
	bool   vDutRxActived = false;
	char   vErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};


	/*---------------------------------------*
	 * Clear Return Parameters and Container *
	 *---------------------------------------*/
	ClearReturnParameters(l_rxSweepPerReturnMap);

	/*------------------------*
	 * Respond to QUERY_INPUT *
	 *------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_rxSweepPerParamMap);
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
		RespondToQueryReturn(l_rxSweepPerReturnMap);
		return err;
	}
	else
	{
		// do nothing
	}

	vector< double >     perValuesVector(MAX_BUFFER_SIZE);
	vector< double >     pwrLevelsVector(MAX_BUFFER_SIZE);

	try
	{
		/*---------------------------------------------------------*
		 * Both g_WiFi_Test_ID and g_WiFi_Dut need to be valid (>=0)   *
		 *---------------------------------------------------------*/
		if( g_WiFi_Test_ID<0 || g_WiFi_Dut<0 )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] WiFi_Test_ID or WiFi_Dut not valid. WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_Test_ID, g_WiFi_Dut);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] g_WiFi_Test_ID = %d and g_WiFi_Dut = %d.\n", g_WiFi_Test_ID, g_WiFi_Dut);
		}

		TM_ClearReturns(g_WiFi_Test_ID);

		/*--------------------------*
		 * Get mandatory parameters *
		 *--------------------------*/
		err = GetInputParameters(l_rxSweepPerParamMap);
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
		* needed to split WiFi_RX_Sweep_Per_2010ExtTest into two to	* 
		* avoid a stack check issue.							*
		* 										*
		*------------------------------------------------------*/
		if ( g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE )
		{
			return WiFi_RX_Sweep_Per_2010ExtTest();
		}

		// Error return of this function is irrelevant
		CheckDutTransmitStatus();

#pragma region Prepare input parameters
		err = ConfirmRxSweepPerParameters( &channel, &wifiMode, &wifiStreamNum, &cableLossDb, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters ConfirmRxSweepPerParameters() return OK.\n");
		}
#pragma endregion

#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to Receive  *
		 *---------------------------*/
		// Set DUT RF frequency, tx power, antenna, data rate
		vDUT_ClearParameters(g_WiFi_Dut);

		if( wifiMode==WIFI_11N_HT40 )
		{
			HT40ModeOn = 1;   // 1: HT40 mode;
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_rxSweepPerParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_rxSweepPerParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_rxSweepPerParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",	  l_rxSweepPerParam.FREQ_MHZ);
		}
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		  HT40ModeOn);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",		  l_rxSweepPerParam.FREQ_MHZ);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",		  l_rxSweepPerParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",		  l_rxSweepPerParam.BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",		  l_rxSweepPerParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_rxSweepPerParam.PACKET_FORMAT_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N", l_rxSweepPerParam.GUARD_INTERVAL_11N);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX1",			      l_rxSweepPerParam.RX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX2",				  l_rxSweepPerParam.RX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX3",				  l_rxSweepPerParam.RX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX4",				  l_rxSweepPerParam.RX4);

		vDUT_AddStringParameter (g_WiFi_Dut, "PER_WAVEFORM_DESTINATION_MAC", g_WiFiGlobalSettingParam.PER_WAVEFORM_DESTINATION_MAC);

		if ( (g_dutRxConfigChanged==true)||(g_vDutRxActived==false) )
		{
			if ( g_vDutRxActived==true )
			{
				/*-----------*
				 *  Rx Stop  *
				 *-----------*/
				err = ::vDUT_Run(g_WiFi_Dut, "RX_STOP");
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
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_STOP) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					g_vDutRxActived = false;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_STOP) return OK.\n");
				}
			}
			else
			{
				// continue Dut configuration
			}

			// Modified /* -cfy@sunnyvale, 2012/3/13- */
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
			/* <><~~ */

			err = vDUT_Run(g_WiFi_Dut, "RX_SET_ANTENNA");
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

			err = vDUT_Run(g_WiFi_Dut, "RX_SET_BW");
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

			err = vDUT_Run(g_WiFi_Dut, "RX_SET_DATA_RATE");
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

			err = vDUT_Run(g_WiFi_Dut, "RX_SET_FILTER");
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_SET_FILTER) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_SET_FILTER) return OK.\n");
			}
		}
		else
		{
			// g_dutRxConfigChanged==false, do nothing
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Since g_dutRxConfigChanged = false, skip Dut control section.\n");
		}
#pragma endregion

#pragma region Setup LP Tester
		/*------------------------------------*
		 * check VSG output power limit first *
		 *------------------------------------*/
		if( wifiMode==WIFI_11B )
		{
			MaxVSGPowerLimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11B;
		}
		else if( wifiMode==WIFI_11AG )
		{
			MaxVSGPowerLimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11G;
		}
		else
		{
			MaxVSGPowerLimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11N;
		}

		double VSG_POWER = l_rxSweepPerParam.START_POWER_LEVEL_DBM+cableLossDb;
		if ( VSG_POWER>MaxVSGPowerLimit )
		{
			//ERR_VSG_POWER_EXCEED_LIMIT
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", VSG_POWER, MaxVSGPowerLimit);
			throw logMessage;
		}
		else
		{
			// do nothing
		}

		/*---------------------------------------------*
		 * Ccalculate how many sweep step for the test *
		 *---------------------------------------------*/
		int totalSweepSteps = abs((int)((l_rxSweepPerParam.START_POWER_LEVEL_DBM - l_rxSweepPerParam.STOP_POWER_LEVEL_DBM)/l_rxSweepPerParam.SWEEP_STEP_DB));

		if (l_rxSweepPerParam.START_POWER_LEVEL_DBM < l_rxSweepPerParam.STOP_POWER_LEVEL_DBM)
		{
			l_rxSweepPerParam.SWEEP_STEP_DB = fabs(l_rxSweepPerParam.SWEEP_STEP_DB);

		}
		else
		{

			if (l_rxSweepPerParam.SWEEP_STEP_DB > 0)
			{
				l_rxSweepPerParam.SWEEP_STEP_DB = -1.0 * l_rxSweepPerParam.SWEEP_STEP_DB;
			}
			else
			{
				// do nothing
			}
		}

		bool bFind = false;

		
			// NOT "g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE"
		

			/*---------------------*
			 * Load waveform Files *
			 *---------------------*/
			char   modFile[MAX_BUFFER_SIZE] = {'\0'};
			err = GetWaveformFileName("PER",
					"WAVEFORM_NAME",
					wifiMode,
					l_rxSweepPerParam.BANDWIDTH,
					l_rxSweepPerParam.DATA_RATE,
					l_rxSweepPerParam.PREAMBLE,
					l_rxSweepPerParam.PACKET_FORMAT_11N,
					l_rxSweepPerParam.GUARD_INTERVAL_11N,
					modFile,
					MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get waveform file name, GetWaveformFileName() return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] GetWaveformFileName() return OK.\n");
			}

			// Load the whole MOD file for continuous transmit
			err = ::LP_SetVsgModulation( modFile );
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_SetVsgModulation( %s ) return error.\n", modFile);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsgModulation( %s ) return OK.\n", modFile);
			}

			/*--------------------*
			 * Setup IQTester VSG *
			 *--------------------*/
			// Since the limitation, we assume that all path loss value are very close.
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Set VSG port by g_WiFiGlobalSettingParam.VSG_PORT=[%d].\n", g_WiFiGlobalSettingParam.VSG_PORT);
			err = ::LP_SetVsg(l_rxSweepPerParam.FREQ_MHZ*1e6, VSG_POWER, g_WiFiGlobalSettingParam.VSG_PORT);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to setup VSG, LP_SetVsg() return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsg() return OK.\n");
			}

			/*---------------*
			 *  Turn on VSG  *
			 *---------------*/
			err = ::LP_EnableVsgRF(1);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to turn on VSG, LP_EnableVsgRF(1) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_EnableVsgRF(1) return OK.\n");
			}

			if ( !g_vDutControlDisabled )
			{
				/*-----------------------------*
				 * Send one packet for warm up *
				 *-----------------------------*/
				err = ::LP_SetFrameCnt(1);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] VSG send out packet failed, LP_SetFrameCnt(1) return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetFrameCnt(1) return OK.\n");
				}
				// After warm up, no signal coming out from VSG
			}
			else
			{
				// do nothing
			}
#pragma endregion

#pragma region Start RX_TEST

			/*---------------------*
			 *  Do RX pre_RX frst  *
			 *---------------------*/
			err = vDUT_Run(g_WiFi_Dut, "RX_PRE_RX");
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_PRE_RX) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_PRE_RX) return OK.\n");
			}

			/*-------------------------*
			 * clear RX packet counter *
			 *-------------------------*/
			err = vDUT_Run(g_WiFi_Dut, "RX_CLEAR_STATS");
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_CLEAR_STATS) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_CLEAR_STATS) return OK.\n");
			}

			/*----------*
			 * RX Start *
			 *----------*/
			err = vDUT_Run(g_WiFi_Dut, "RX_START");
			if ( ERR_OK!=err )
			{
				g_vDutRxActived = false;
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_START) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutRxActived = true;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_START) return OK.\n");
			}

			l_rxSweepPerReturn.POWER_LEVELS->clear();
			l_rxSweepPerReturn.PER_VALUES->clear();

			double	per = NA_NUMBER;
			int iFirstFailedIndex = totalSweepSteps + 999999;
			bSensFound = false;
			for (int i=0; i<=totalSweepSteps; i++)
			{
				per = NA_NUMBER;

				if (i >= MAX_BUFFER_SIZE)
				{
					break;
				}
				else
				{
					//do nothing
				}

				/*-------------------------*
				 * clear RX packet counter *
				 *-------------------------*/
				err = vDUT_Run(g_WiFi_Dut, "RX_CLEAR_STATS");
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
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_CLEAR_STATS) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_CLEAR_STATS) return OK.\n");
				}

				VSG_POWER = l_rxSweepPerParam.START_POWER_LEVEL_DBM + cableLossDb + l_rxSweepPerParam.SWEEP_STEP_DB * i;

				//check VSG output power limit first
				if( wifiMode==WIFI_11B )
				{
					MaxVSGPowerLimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11B;
				}
				else if( wifiMode==WIFI_11AG )
				{
					MaxVSGPowerLimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11G;
				}
				else
				{
					MaxVSGPowerLimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11N;
				}

				if( (VSG_POWER)>MaxVSGPowerLimit )
				{
					err = ERR_VSG_POWER_EXCEED_LIMIT;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", VSG_POWER, MaxVSGPowerLimit);
					throw logMessage;
				}

				err = ::LP_SetVsg(l_rxSweepPerParam.FREQ_MHZ*1e6, VSG_POWER, g_WiFiGlobalSettingParam.VSG_PORT);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to setup VSG, LP_SetVsg() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsg() return OK.\n");
				}

				/*---------------*
				 *  Turn on VSG  *
				 *---------------*/
				err = ::LP_EnableVsgRF(1);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to turn on VSG, LP_EnableVsgRF(1) return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_EnableVsgRF(1) return OK.\n");
				}
				/*--------------------------*
				 * Send packet for PER Test *
				 *--------------------------*/
				if (0==l_rxSweepPerParam.FRAME_COUNT)
				{
					err = GetPacketNumber(  wifiMode,
							l_rxSweepPerParam.BANDWIDTH,
							l_rxSweepPerParam.DATA_RATE,
							l_rxSweepPerParam.PACKET_FORMAT_11N,
							l_rxSweepPerParam.GUARD_INTERVAL_11N,
							&packetNumber);
					if ( ERR_OK!=err )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get the number of packet for PER test.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Get the number of packet GetPacketNumber() return OK.\n");
					}
				}
				else
				{
					packetNumber = l_rxSweepPerParam.FRAME_COUNT;
				}

				err = ::LP_SetFrameCnt(packetNumber);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] VSG send out packet failed, LP_SetFrameCnt(%d) return error.\n", packetNumber);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetFrameCnt(%d) return OK.\n", packetNumber);
				}
#pragma endregion

				/*-----------------*
				 * Wait for TxDone *
				 *-----------------*/
				int timeOutInMs = 0, sleepTime = 50;
				while ( timeOutInMs<=(1000*(g_WiFiGlobalSettingParam.PER_VSG_TIMEOUT_SEC)) )
				{
					err = ::LP_TxDone();
					if (ERR_OK!=err)	// Tx not finish yet...
					{
						Sleep(sleepTime);
						timeOutInMs = timeOutInMs + sleepTime;
					}
					else
					{	// Tx Done, then break the while loop.
						break;
					}
				}
				if ( timeOutInMs>(1000*(g_WiFiGlobalSettingParam.PER_VSG_TIMEOUT_SEC)) )	// timeout
				{
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQTester VSG timeout.\n");
					throw logMessage;
				}
				else
				{
					// no error, do noting.
				}

				if ( g_vDutControlDisabled )
				{
					/*----------------------------*
					 * Disable VSG output signal  *
					 *----------------------------*/
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
				else
				{
					// do nothing
				}

#pragma region Retrieve analysis Results
				/*--------------------*
				 * Get RX PER Result  *
				 *--------------------*/
				totalPackets = 0;
				goodPackets  = 0;
				badPackets   = 0;

				err = ::vDUT_Run(g_WiFi_Dut, "RX_GET_STATS");
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
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_GET_STATS) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_GET_STATS) return OK.\n");
				}

				err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "GOOD_PACKETS",  &goodPackets);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_GetIntegerReturn(GOOD_PACKETS) return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(GOOD_PACKETS) return OK.\n");
				}

				if ( goodPackets<0 )
				{
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Dut report GOOD_BITS less than 0.\n");
					throw logMessage;
				}
				else
				{
					// l_rxSweepPerReturn.GOOD_PACKETS = goodPackets;
				}

				// Using packet sent as the total packets
				totalPackets = packetNumber;

				// Sometime Dut get packets from Air
				// workaround for some dut which returns more ack than packet sent
				if ( goodPackets>totalPackets )
				{
					goodPackets = totalPackets;
				}
				else
				{
					// do nothing
				}

				badPackets = totalPackets - goodPackets;

				l_rxSweepPerReturn.POWER_LEVELS->push_back(l_rxSweepPerParam.START_POWER_LEVEL_DBM + l_rxSweepPerParam.SWEEP_STEP_DB * i);
				pwrLevelsVector[i] = l_rxSweepPerParam.START_POWER_LEVEL_DBM + l_rxSweepPerParam.SWEEP_STEP_DB * i;

				if( totalPackets!=0 )
				{
					per = ((double)(badPackets)/(double)(totalPackets)) * 100.0;
				}
				else	// In this case, totalPackets = 0
				{
					per = 100.0;
				}

				l_rxSweepPerReturn.PER_VALUES->push_back(per);
				perValuesVector[i] = per;

				// l_rxSweepPerReturn.NUMBER_OF_POWER_LEVELS = i + 1;

				if ( perValuesVector[i] <= l_rxSweepPerParam.SENS_PER_PERCENTAGE )	// PER pass
				{
					continue;
				}
				else    //		PER fail
				{
					if (0 != l_rxSweepPerParam.STOP_AT_PER_PERCENTAGE) //stop at sens per percentage
					{
						if (0 == l_rxSweepPerParam.SPIKE_REMOVAL) //spike removal is off
						{
							// Jump out of "for" loop. Stop PER sweep
							sensPointIndex = i;
							bSensFound = true;
							break;
						}
						else  //spike removal is on.  check if already have 3 continuous fail. If yes, stop test. If not, continue
						{
							if( i==0 || i==1)
							{
								continue;
							}
							else if ( i == totalSweepSteps)
							{
								if (perValuesVector[i-1] > l_rxSweepPerParam.SENS_PER_PERCENTAGE &&
										perValuesVector[i-2] > l_rxSweepPerParam.SENS_PER_PERCENTAGE)
								{
									sensPointIndex = i-2;
									bSensFound = true;
									break;
								}
								else if ( perValuesVector[i-1] > l_rxSweepPerParam.SENS_PER_PERCENTAGE &&
										perValuesVector[i-2] <= l_rxSweepPerParam.SENS_PER_PERCENTAGE)
								{
									sensPointIndex = i-1;
									bSensFound = true;
									break;
								}
								else
								{
									sensPointIndex = i;
									bSensFound = true;
									break;
								}
							}
							else // i>=2
							{
								if(  perValuesVector[i] > l_rxSweepPerParam.SENS_PER_PERCENTAGE &&
										perValuesVector[i-1] > l_rxSweepPerParam.SENS_PER_PERCENTAGE &&
										perValuesVector[i-2] > l_rxSweepPerParam.SENS_PER_PERCENTAGE )
								{
									sensPointIndex = i-2;
									bSensFound = true;
									break;
								}
							}
						}
					}
					else   // Don't stop test. Continue sweeping PER
					{
						if ( !bSensFound)
						{
							sensPointIndex = i;
							bSensFound = true;
						}
						else
						{
							// do nothing
						}
						continue;
					}
				}

			}//end of for loop


			if ( !bSensFound)  // all pass
			{
				l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM = l_rxSweepPerParam.STOP_POWER_LEVEL_DBM;
			}
			else
			{
				if (sensPointIndex >0)
				{
					double dRate = perValuesVector[sensPointIndex] - perValuesVector[sensPointIndex-1];
					dRate = l_rxSweepPerParam.SWEEP_STEP_DB / dRate;

					l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM = pwrLevelsVector[sensPointIndex] - dRate*(perValuesVector[sensPointIndex] - l_rxSweepPerParam.SENS_PER_PERCENTAGE);
				}
				else
				{
					l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM = l_rxSweepPerParam.START_POWER_LEVEL_DBM;
				}
			}



			/*-----------*
			 *  Rx Stop  *
			 *-----------*/
			err = vDUT_Run(g_WiFi_Dut, "RX_STOP");
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_STOP) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutRxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_STOP) return OK.\n");
			}
		
#pragma endregion

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err )
		{
			l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM = (int)(l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM*100);
			l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM = l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM/100;

			sprintf_s(l_rxSweepPerReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_rxSweepPerReturnMap);
		}
		else
		{
			// do nothing
		}
	}

	catch(char *msg)
	{
		ReturnErrorMessage(l_rxSweepPerReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		ReturnErrorMessage(l_rxSweepPerReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
		err = -1;
	}

	// This is a special case, only when some error occur before the RX_STOP.
	// This case will take care by the error handling, but must do RX_STOP manually.
	//if ( vDutRxActived )
	//{
	//    vDUT_Run(g_WiFi_Dut, "RX_STOP");
	//}
	//else
	//{
	//    // do nothing
	//}

	perValuesVector.clear();
	pwrLevelsVector.clear();

	return err;
}

int WiFi_RX_Sweep_Per_2010ExtTest()
{
	int    err = ERR_OK;

	int	   totalPackets = 0;
	int	   goodPackets  = 0;
	int	   badPackets   = 0;

	int	   sensPointIndex = 0;
	bool   bSensFound = false;
	int    channel = 0, HT40ModeOn = 0;
	int    dummyValue = 0;
	int    wifiMode = 0, wifiStreamNum = 0;
	int    packetNumber = 0;
	double cableLossDb  = 0;
	double MaxVSGPowerLimit = 0;
	bool   vDutRxActived = false;
	char   vErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	try
	{
		// Error return of this function is irrelevant
		CheckDutTransmitStatus();

#pragma region Prepare input parameters
		err = ConfirmRxSweepPerParameters( &channel, &wifiMode, &wifiStreamNum, &cableLossDb, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters ConfirmRxSweepPerParameters() return OK.\n");
		}
#pragma endregion

#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to Receive  *
		 *---------------------------*/
		// Set DUT RF frequency, tx power, antenna, data rate
		vDUT_ClearParameters(g_WiFi_Dut);

		if( wifiMode==WIFI_11N_HT40 )
		{
			HT40ModeOn = 1;   // 1: HT40 mode;
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_rxSweepPerParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_rxSweepPerParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_rxSweepPerParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",	  l_rxSweepPerParam.FREQ_MHZ);
		}
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		  HT40ModeOn);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",		  l_rxSweepPerParam.FREQ_MHZ);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",		  l_rxSweepPerParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",		  l_rxSweepPerParam.BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",		  l_rxSweepPerParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_rxSweepPerParam.PACKET_FORMAT_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N", l_rxSweepPerParam.GUARD_INTERVAL_11N);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX1",			      l_rxSweepPerParam.RX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX2",				  l_rxSweepPerParam.RX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX3",				  l_rxSweepPerParam.RX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX4",				  l_rxSweepPerParam.RX4);

		vDUT_AddStringParameter (g_WiFi_Dut, "PER_WAVEFORM_DESTINATION_MAC", g_WiFiGlobalSettingParam.PER_WAVEFORM_DESTINATION_MAC);

		if ( (g_dutRxConfigChanged==true)||(g_vDutRxActived==false) )
		{
			if ( g_vDutRxActived==true )
			{
				/*-----------*
				 *  Rx Stop  *
				 *-----------*/
				err = ::vDUT_Run(g_WiFi_Dut, "RX_STOP");
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
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_STOP) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					g_vDutRxActived = false;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_STOP) return OK.\n");
				}
			}
			else
			{
				// continue Dut configuration
			}

			// Modified /* -cfy@sunnyvale, 2012/3/13- */
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
			/* <><~~ */

			err = vDUT_Run(g_WiFi_Dut, "RX_SET_ANTENNA");
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

			err = vDUT_Run(g_WiFi_Dut, "RX_SET_BW");
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

			err = vDUT_Run(g_WiFi_Dut, "RX_SET_DATA_RATE");
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

			err = vDUT_Run(g_WiFi_Dut, "RX_SET_FILTER");
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_SET_FILTER) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_SET_FILTER) return OK.\n");
			}
		}
		else
		{
			// g_dutRxConfigChanged==false, do nothing
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Since g_dutRxConfigChanged = false, skip Dut control section.\n");
		}
#pragma endregion

#pragma region Setup LP Tester
		/*------------------------------------*
		 * check VSG output power limit first *
		 *------------------------------------*/
		if( wifiMode==WIFI_11B )
		{
			MaxVSGPowerLimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11B;
		}
		else if( wifiMode==WIFI_11AG )
		{
			MaxVSGPowerLimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11G;
		}
		else
		{
			MaxVSGPowerLimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11N;
		}

		double VSG_POWER = l_rxSweepPerParam.START_POWER_LEVEL_DBM+cableLossDb;
		if ( VSG_POWER>MaxVSGPowerLimit )
		{
			//ERR_VSG_POWER_EXCEED_LIMIT
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", VSG_POWER, MaxVSGPowerLimit);
			throw logMessage;
		}
		else
		{
			// do nothing
		}

		/*---------------------------------------------*
		 * Ccalculate how many sweep step for the test *
		 *---------------------------------------------*/
		int totalSweepSteps = abs((int)((l_rxSweepPerParam.START_POWER_LEVEL_DBM - l_rxSweepPerParam.STOP_POWER_LEVEL_DBM)/l_rxSweepPerParam.SWEEP_STEP_DB));

		if (l_rxSweepPerParam.START_POWER_LEVEL_DBM < l_rxSweepPerParam.STOP_POWER_LEVEL_DBM)
		{
			l_rxSweepPerParam.SWEEP_STEP_DB = fabs(l_rxSweepPerParam.SWEEP_STEP_DB);

		}
		else
		{

			if (l_rxSweepPerParam.SWEEP_STEP_DB > 0)
			{
				l_rxSweepPerParam.SWEEP_STEP_DB = -1.0 * l_rxSweepPerParam.SWEEP_STEP_DB;
			}
			else
			{
				// do nothing
			}
		}

		bool bFind = false;

/*---------------------------*
		 * Start IQ2010_Ext Function *
		 *---------------------------*/
		
			int waveformIndex = -1;

#pragma region Start RX_TEST

			if ( (g_vDutRxActived==false) )
			{
				/*---------------------*
				 *  Do RX pre_RX frst  *
				 *---------------------*/
				err = vDUT_Run(g_WiFi_Dut, "RX_PRE_RX");
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
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_PRE_RX) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_PRE_RX) return OK.\n");
				}

				/*-------------------------*
				 * clear RX packet counter *
				 *-------------------------*/
				err = vDUT_Run(g_WiFi_Dut, "RX_CLEAR_STATS");
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
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_CLEAR_STATS) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_CLEAR_STATS) return OK.\n");
				}

				/*----------*
				 * RX Start *
				 *----------*/
				err = vDUT_Run(g_WiFi_Dut, "RX_START");
				if ( ERR_OK!=err )
				{
					g_vDutRxActived = false;
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
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_START) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					g_vDutRxActived = true;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_START) return OK.\n");
				}

				// Delay for DUT settle
				if (0!=g_WiFiGlobalSettingParam.DUT_RX_SETTLE_TIME_MS)
				{
					Sleep(g_WiFiGlobalSettingParam.DUT_RX_SETTLE_TIME_MS);
				}
				else
				{
					// do nothing
				}
			}
#pragma endregion
			/*-----------------------------------*
			 * Get Multi-Waveform Index from Map *
			 *-----------------------------------*/
			char  modFile[MAX_BUFFER_SIZE] = "\0";
			err = GetMultiWaveformFileName( "PER",
					"WAVEFORM_NAME",
					wifiMode,
					l_rxSweepPerParam.BANDWIDTH,
					l_rxSweepPerParam.DATA_RATE,
					l_rxSweepPerParam.PREAMBLE,
					l_rxSweepPerParam.PACKET_FORMAT_11N,
					l_rxSweepPerParam.GUARD_INTERVAL_11N,
					modFile,
					MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get Multi-Waveform file name, GetMultiWaveformFileName() return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] GetMultiWaveformFileName() return OK.\n");
			}

			err = TM_GetMultiWaveformIndex( modFile, &waveformIndex );
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get Multi-Waveform Index from Map, TM_GetMultiWaveformIndex() return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_GetMultiWaveformIndex() return OK.\n");
			}

			if (0==l_rxSweepPerParam.FRAME_COUNT)
			{
				err = GetPacketNumber(  wifiMode,
						l_rxSweepPerParam.BANDWIDTH,
						l_rxSweepPerParam.DATA_RATE,
						l_rxSweepPerParam.PACKET_FORMAT_11N,
						l_rxSweepPerParam.GUARD_INTERVAL_11N,
						&packetNumber);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get the number of packet for PER test.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Get the number of packet GetPacketNumber() return OK.\n");
				}
			}
			else
			{
				packetNumber = l_rxSweepPerParam.FRAME_COUNT;
			}

			/*-------------------------------------*
			 * Start IQ2010Ext PER Test (with ACK) *
			 *-------------------------------------*/
			err = ::LP_IQ2010EXT_RxPer( waveformIndex,
					l_rxSweepPerParam.FREQ_MHZ,
					l_rxSweepPerParam.START_POWER_LEVEL_DBM+cableLossDb,
					l_rxSweepPerParam.STOP_POWER_LEVEL_DBM+cableLossDb,
					abs(l_rxSweepPerParam.SWEEP_STEP_DB),
					packetNumber,
					g_WiFiGlobalSettingParam.VSG_PORT,
					g_WiFiGlobalSettingParam.VSA_ACK_POWER_RMS_DBM-cableLossDb,
					g_WiFiGlobalSettingParam.VSA_ACK_TRIGGER_LEVEL_DBM-cableLossDb
					);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_RxPer() return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_RxPer() return OK.\n");
			}

			/*---------------------------*
			 * Get IQ2010Ext PER Results *
			 *---------------------------*/
			double per[MAX_BUFFER_SIZE] = {NA_NUMBER}, powerLeve[MAX_BUFFER_SIZE];
			int    dataSize = 0;

			int numberOfResultSet = 0;
			err = ::LP_IQ2010EXT_GetNumberOfPerPowerLevels( "perPercent", &numberOfResultSet );
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetNumberOfPerPowerLevels(perPercent) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetNumberOfPerPowerLevels(perPercent) return OK.\n");
			}

			if ( numberOfResultSet>0 )
			{
				l_rxSweepPerReturn.NUMBER_OF_POWER_LEVELS = numberOfResultSet;

				::LP_IQ2010EXT_GetVectorMeasurement( "powerLeveldBm", powerLeve, MAX_BUFFER_SIZE, &dataSize, 0 );
				::LP_IQ2010EXT_GetVectorMeasurement( "perPercent", per, MAX_BUFFER_SIZE, &dataSize, 0 );
			}
			else
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] No result is found for Per test.\n");
				throw logMessage;
			}

			/*-------------------*
			 * Return RX Results *
			 *-------------------*/
			l_rxSweepPerReturn.POWER_LEVELS->clear();
			l_rxSweepPerReturn.PER_VALUES->clear();
			l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM = l_rxSweepPerParam.STOP_POWER_LEVEL_DBM;
			bSensFound = false;

			for (int i=0; i<numberOfResultSet; i++)
			{
				l_rxSweepPerReturn.POWER_LEVELS->push_back( l_rxSweepPerParam.START_POWER_LEVEL_DBM + l_rxSweepPerParam.SWEEP_STEP_DB * i );
				l_rxSweepPerReturn.PER_VALUES->push_back( per[i] );
			}

			for (int j=0; j<numberOfResultSet; j++)
			{

				if ( per[j] <= l_rxSweepPerParam.SENS_PER_PERCENTAGE )	// PER pass
				{
					continue;
				}
				else    //		PER fail
				{
					if (bSensFound)  // Store the first fail level
					{
						continue;

					}
					else
					{
						sensPointIndex = j;
						bSensFound = true;
						break;
					}

				}
			}


			if ( !bSensFound)  // all pass
			{
				l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM = l_rxSweepPerParam.STOP_POWER_LEVEL_DBM;
			}
			else
			{
				if (sensPointIndex >0)
				{
					double dRate = per[sensPointIndex] - per[sensPointIndex-1];
					dRate = l_rxSweepPerParam.SWEEP_STEP_DB / dRate;

					l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM = (l_rxSweepPerParam.START_POWER_LEVEL_DBM + l_rxSweepPerParam.SWEEP_STEP_DB*sensPointIndex)
						- dRate*(per[sensPointIndex] - l_rxSweepPerParam.SENS_PER_PERCENTAGE);
				}
				else
				{
					l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM = l_rxSweepPerParam.START_POWER_LEVEL_DBM;
				}
			}

			/*//Find the index of maximum input level: scan from low power to high power
			//G: good per, B: bad per
			//if the PER array is            B, B, B, G, G, G, G,...., obtain the first index with good per. i.e. index = 3
			//Let's see special PER array is B, B, B, G, B, G, G,...., obtain the first index with good per. Algo. is ok.
			//Then, the index 2 are what we want
			if( l_rxSweepPerParam.SWEEP_STEP_DB > 0 )
			{
			if ( per[i] < l_rxSweepPerParam.SENS_PER_PERCENTAGE && bFind == false)
			{
			//Find the index of the power level with first good PER. i.e. perValuesVector[iSensPointIndex-1] is failed.
			sensPointIndex = i;
			bFind = true;
			}
			else{}//do nothing
			}

			//Find the index of minimum inpur level: scan from high power to low power
			//G: good per, B: bad per
			//if the PER array is            G, G, G, B, B, B, B,...., obtain the first index with bad per. i.e. index = 3
			//Let's see special PER array is G, G, G, B, G, B, B,...., obtain the first index with bad per. Algo. is ok.
			if( l_rxSweepPerParam.SWEEP_STEP_DB < 0 )
			{
			if ( per[i] > l_rxSweepPerParam.SENS_PER_PERCENTAGE && bFind == false)
			{
			//Find the index of the power level with first bad PER. i.e. perValuesVector[iSensPointIndex] is failed.
			sensPointIndex = i;
			bFind = true;
			}
			else{}//do nothing
			}*/
			//end of for loop


			/*	//scan from low power to high power
				if( l_rxSweepPerParam.SWEEP_STEP_DB > 0 )
				{
				if( sensPointIndex == 0 )
				{//all per values are passed.
				l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM = l_rxSweepPerParam.START_POWER_LEVEL_DBM;
				}
				else if(sensPointIndex >0 && per[sensPointIndex] <= l_rxSweepPerParam.SENS_PER_PERCENTAGE &&
				per[sensPointIndex-1] > l_rxSweepPerParam.SENS_PER_PERCENTAGE )
				{
				sensPointIndex--;
				bSensFound = true;
				}

				if( !bFind )
				{//all per values are failed.
				l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM = l_rxSweepPerParam.STOP_POWER_LEVEL_DBM;
				}

				}

			//scan from high power to low power
			if( l_rxSweepPerParam.SWEEP_STEP_DB < 0 )
			{
			if( sensPointIndex == 0  )
			{//all per values are failed
			l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM = l_rxSweepPerParam.START_POWER_LEVEL_DBM;
			}
			else if( sensPointIndex >0 && per[sensPointIndex] > l_rxSweepPerParam.SENS_PER_PERCENTAGE &&
			per[sensPointIndex-1] <= l_rxSweepPerParam.SENS_PER_PERCENTAGE )
			{
			bSensFound = true;
			}

			if( !bFind )
			{//all per values are passed.
			l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM = l_rxSweepPerParam.STOP_POWER_LEVEL_DBM;
			}

			}

			if (true == bSensFound)
			{
			int iGoddPerStep = 0;
			if( l_rxSweepPerParam.SWEEP_STEP_DB > 0 )
			{
			iGoddPerStep = 1;
			}
			if( l_rxSweepPerParam.SWEEP_STEP_DB < 0 )
			{
			iGoddPerStep = -1;
			}
			double dRate = fabs(l_rxSweepPerParam.SWEEP_STEP_DB)/(per[sensPointIndex] - per[sensPointIndex + iGoddPerStep]);
			l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM = (l_rxSweepPerParam.START_POWER_LEVEL_DBM + l_rxSweepPerParam.SWEEP_STEP_DB*sensPointIndex) +
			dRate*(per[sensPointIndex] - l_rxSweepPerParam.SENS_PER_PERCENTAGE);

			}*/

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err )
		{
			l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM = (int)(l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM*100);
			l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM = l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM/100;

			sprintf_s(l_rxSweepPerReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_rxSweepPerReturnMap);
		}
		else
		{
			// do nothing
		}
	}

	catch(char *msg)
	{
		ReturnErrorMessage(l_rxSweepPerReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		ReturnErrorMessage(l_rxSweepPerReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
		err = -1;
	}

	// This is a special case, only when some error occur before the RX_STOP.
	// This case will take care by the error handling, but must do RX_STOP manually.
	//if ( vDutRxActived )
	//{
	//    vDUT_Run(g_WiFi_Dut, "RX_STOP");
	//}
	//else
	//{
	//    // do nothing
	//}

	

	return err;
}

int InitializeSweepPerContainers(void)
{
	/*-----------------*
	 * Input Parameters *
	 *-----------------*/
	l_rxSweepPerParamMap.clear();

	WIFI_SETTING_STRUCT setting;
	setting.unit = "";
	setting.helpText = "";

	strcpy_s(l_rxSweepPerParam.BANDWIDTH, MAX_BUFFER_SIZE, "HT20");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxSweepPerParam.BANDWIDTH))    // Type_Checking
	{
		setting.value = (void*)l_rxSweepPerParam.BANDWIDTH;
		setting.unit        = "";
		setting.helpText    = "Channel bandwidth. Valid options: HT20 or HT40";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BANDWIDTH", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_rxSweepPerParam.DATA_RATE, MAX_BUFFER_SIZE, "OFDM-54");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxSweepPerParam.DATA_RATE))    // Type_Checking
	{
		setting.value = (void*)l_rxSweepPerParam.DATA_RATE;
		setting.unit        = "";
		setting.helpText    = "Data rate names, such as DSSS-1, CCK-5_5, CCK-11, OFDM-54, MCS0, MCS15";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_rxSweepPerParam.PREAMBLE, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxSweepPerParam.PREAMBLE))    // Type_Checking
	{
		setting.value       = (void*)l_rxSweepPerParam.PREAMBLE;
		setting.unit        = "";
		setting.helpText    = "The preamble type of 11B(only), can be SHORT or LONG, Default=LONG.";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PREAMBLE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_rxSweepPerParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, "MIXED");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxSweepPerParam.PACKET_FORMAT_11N))    // Type_Checking
	{
		setting.value       = (void*)l_rxSweepPerParam.PACKET_FORMAT_11N;
		setting.unit        = "";
		setting.helpText    = "The packet format of 11N(only), can be MIXED or GREENFIELD, Default=MIXED.";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT_11N", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_rxSweepPerParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxSweepPerParam.GUARD_INTERVAL_11N))    // Type_Checking
	{
		setting.value       = (void*)l_rxSweepPerParam.GUARD_INTERVAL_11N;
		setting.unit        = "";
		setting.helpText    = "The guard interval format of 11N(only), can be LONG or SHORT, Default=LONG.";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL_11N", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxSweepPerParam.FREQ_MHZ = 2412;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxSweepPerParam.FREQ_MHZ))    // Type_Checking
	{
		setting.value = (void*)&l_rxSweepPerParam.FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "Channel center frequency in MHz";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_MHZ", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxSweepPerParam.FRAME_COUNT = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxSweepPerParam.FRAME_COUNT))    // Type_Checking
	{
		setting.value = (void*)&l_rxSweepPerParam.FRAME_COUNT;
		setting.unit        = "";
		setting.helpText    = "Number of frame count, default=0, means using default global setting value.";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("FRAME_COUNT", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_rxSweepPerParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_rxSweepPerParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_rxSweepPerParam.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_rxSweepPerParam.START_POWER_LEVEL_DBM = -65.0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxSweepPerParam.START_POWER_LEVEL_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_rxSweepPerParam.START_POWER_LEVEL_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected power level start at DUT antenna port";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("START_POWER_LEVEL_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxSweepPerParam.STOP_POWER_LEVEL_DBM= -75.0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxSweepPerParam.STOP_POWER_LEVEL_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_rxSweepPerParam.STOP_POWER_LEVEL_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected power level end at DUT antenna port";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("STOP_POWER_LEVEL_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_rxSweepPerParam.SWEEP_STEP_DB= -1.0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxSweepPerParam.SWEEP_STEP_DB))    // Type_Checking
	{
		setting.value = (void*)&l_rxSweepPerParam.SWEEP_STEP_DB;
		setting.unit        = "dB";
		setting.helpText    = "Expected power step for the PER sweep";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("SWEEP_STEP_DB", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxSweepPerParam.STOP_AT_PER_PERCENTAGE = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxSweepPerParam.STOP_AT_PER_PERCENTAGE))    // Type_Checking
	{
		setting.value = (void*)&l_rxSweepPerParam.STOP_AT_PER_PERCENTAGE;
		setting.unit        = "";
		setting.helpText    = "[IQ2010Ext only support:(0)] The flag to stop PER sweep test when sensitivity point has been detected. 1: stop at sens point; 0: ignore sens point and sweep the whole range";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("STOP_AT_PER_PERCENTAGE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxSweepPerParam.SENS_PER_PERCENTAGE = 10;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxSweepPerParam.SENS_PER_PERCENTAGE))    // Type_Checking
	{
		setting.value = (void*)&l_rxSweepPerParam.SENS_PER_PERCENTAGE;
		setting.unit        = "%";
		setting.helpText    = "The PER fail percentage";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("SENS_PER_PERCENTAGE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxSweepPerParam.ARRAY_HANDLING_METHOD = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxSweepPerParam.ARRAY_HANDLING_METHOD))    // Type_Checking
	{
		setting.value = (void*)&l_rxSweepPerParam.ARRAY_HANDLING_METHOD;
		setting.unit        = "";
		setting.helpText    = "Use the flag to handle array result. Default: 0; 0: Do nothing, 1: Display result on screen, 2: Display result and log to file";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ARRAY_HANDLING_METHOD", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_rxSweepPerParam.SPIKE_REMOVAL = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxSweepPerParam.SPIKE_REMOVAL))    // Type_Checking
	{
		setting.value = (void*)&l_rxSweepPerParam.SPIKE_REMOVAL;
		setting.unit        = "";
		setting.helpText    = "The flag to remove PER spike during sweep. Default: 1";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("SPIKE_REMOVAL", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_rxSweepPerParam.RX1 = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxSweepPerParam.RX1))    // Type_Checking
	{
		setting.value = (void*)&l_rxSweepPerParam.RX1;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 1 on/off. 1:on; 0:off";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX1", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxSweepPerParam.RX2 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxSweepPerParam.RX2))    // Type_Checking
	{
		setting.value = (void*)&l_rxSweepPerParam.RX2;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 2 on/off. 1:on; 0:off";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX2", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxSweepPerParam.RX3 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxSweepPerParam.RX3))    // Type_Checking
	{
		setting.value = (void*)&l_rxSweepPerParam.RX3;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 3 on/off. 1:on; 0:off";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX3", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxSweepPerParam.RX4 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxSweepPerParam.RX4))    // Type_Checking
	{
		setting.value = (void*)&l_rxSweepPerParam.RX4;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 4 on/off. 1:on; 0:off";
		l_rxSweepPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX4", setting) );
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
	l_rxSweepPerReturnMap.clear();

	l_rxSweepPerReturn.NUMBER_OF_POWER_LEVELS = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxSweepPerReturn.NUMBER_OF_POWER_LEVELS))    // Type_Checking
	{
		setting.value = (void*)&l_rxSweepPerReturn.NUMBER_OF_POWER_LEVELS;
		setting.unit        = "";
		setting.helpText    = "Number of power levels in the sweep test.";
		l_rxSweepPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("NUMBER_OF_POWER_LEVELS", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM = NA_DOUBLE;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_rxSweepPerReturn.SENS_POWER_LEVEL_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "PER sensitivity level of the sweep test.";
		l_rxSweepPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SENS_POWER_LEVEL_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxSweepPerReturn.POWER_LEVELS = new vector<double>();
	l_rxSweepPerReturn.POWER_LEVELS->clear();
	setting.type = WIFI_SETTING_TYPE_ARRAY_DOUBLE;
	if (0 == l_rxSweepPerReturn.POWER_LEVELS->size())    // Type_Checking
	{
		setting.value = (void*)l_rxSweepPerReturn.POWER_LEVELS;
		setting.unit        = "dBm";
		setting.helpText    = "Array to store sweep power levels.";
		l_rxSweepPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_LEVELS", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxSweepPerReturn.PER_VALUES = new vector<double>();
	l_rxSweepPerReturn.PER_VALUES->clear();
	setting.type = WIFI_SETTING_TYPE_ARRAY_DOUBLE;
	if (0 == l_rxSweepPerReturn.PER_VALUES->size())    // Type_Checking
	{
		setting.value = (void*)l_rxSweepPerReturn.PER_VALUES;
		setting.unit        = "%";
		setting.helpText    = "Array to store sweep PER values.";
		l_rxSweepPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("PER_VALUES", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_rxSweepPerReturn.CABLE_LOSS_DB[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_rxSweepPerReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_rxSweepPerReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_rxSweepPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_rxSweepPerReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxSweepPerReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_rxSweepPerReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_rxSweepPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
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
int ConfirmRxSweepPerParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *cableLossDb, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		// Checking the Input Parameters
		err = TM_WiFiConvertFrequencyToChannel(l_rxSweepPerParam.FREQ_MHZ, channel);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown FREQ_MHZ, convert WiFi frequncy %d to channel failed.\n", l_rxSweepPerParam.FREQ_MHZ);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}
		err = TM_WiFiConvertDataRateNameToIndex(l_rxSweepPerParam.DATA_RATE, &dummyInt);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_rxSweepPerParam.DATA_RATE);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertDataRateNameToIndex() return OK.\n");
		}
		if ( 0!=strcmp(l_rxSweepPerParam.BANDWIDTH, "HT20") && 0!=strcmp(l_rxSweepPerParam.BANDWIDTH, "HT40") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown BANDWIDTH, WiFi bandwidth %s not supported.\n", l_rxSweepPerParam.BANDWIDTH);
			throw logMessage;
		}
		if ( 0!=strcmp(l_rxSweepPerParam.PREAMBLE, "SHORT") && 0!=strcmp(l_rxSweepPerParam.PREAMBLE, "LONG") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_rxSweepPerParam.PREAMBLE);
			throw logMessage;
		}
		if ( 0!=strcmp(l_rxSweepPerParam.PACKET_FORMAT_11N, "MIXED") && 0!=strcmp(l_rxSweepPerParam.PACKET_FORMAT_11N, "GREENFIELD") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PACKET_FORMAT_11N, WiFi 11n packet format %s not supported.\n", l_rxSweepPerParam.PACKET_FORMAT_11N);
			throw logMessage;
		}
		if ( 0!=strcmp(l_rxSweepPerParam.GUARD_INTERVAL_11N, "LONG") && 0!=strcmp(l_rxSweepPerParam.GUARD_INTERVAL_11N, "SHORT") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown GUARD_INTERVAL_11N, WiFi 11n guard interval format %s not supported.\n", l_rxSweepPerParam.GUARD_INTERVAL_11N);
			throw logMessage;
		}


		// Convert parameter
		err = WiFiTestMode(l_rxSweepPerParam.DATA_RATE, l_rxSweepPerParam.BANDWIDTH, wifiMode, wifiStreamNum);
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
				l_rxSweepPerParam.FREQ_MHZ,
				l_rxSweepPerParam.RX1,
				l_rxSweepPerParam.RX2,
				l_rxSweepPerParam.RX3,
				l_rxSweepPerParam.RX4,
				l_rxSweepPerParam.CABLE_LOSS_DB,
				l_rxSweepPerReturn.CABLE_LOSS_DB,
				cableLossDb,
				RX_TABLE
				);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get RX_CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] CheckPathLossTableExt(RX_TABLE) return OK.\n");
		}

		// Check Dut configuration changed or not
		if (  g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE==0	||		// If "IQ2010_EXT_ENABLE=0" (means Normal Mode), then must configure DUT always.
				0!=strcmp(l_rxSweepPerParam.BANDWIDTH, g_RecordedRxParam.BANDWIDTH) ||
				0!=strcmp(l_rxSweepPerParam.PREAMBLE,  g_RecordedRxParam.PREAMBLE) ||
				0!=strcmp(l_rxSweepPerParam.PACKET_FORMAT_11N, g_RecordedRxParam.PACKET_FORMAT_11N) ||
				0!=strcmp(l_rxSweepPerParam.GUARD_INTERVAL_11N, g_RecordedRxParam.GUARD_INTERVAL_11N) ||
				l_rxSweepPerParam.CABLE_LOSS_DB[0]!=g_RecordedRxParam.CABLE_LOSS_DB[0] ||
				l_rxSweepPerParam.CABLE_LOSS_DB[1]!=g_RecordedRxParam.CABLE_LOSS_DB[1] ||
				l_rxSweepPerParam.CABLE_LOSS_DB[2]!=g_RecordedRxParam.CABLE_LOSS_DB[2] ||
				l_rxSweepPerParam.CABLE_LOSS_DB[3]!=g_RecordedRxParam.CABLE_LOSS_DB[3] ||
				l_rxSweepPerParam.FREQ_MHZ!=g_RecordedRxParam.FREQ_MHZ ||
				l_rxSweepPerParam.RX1!=g_RecordedRxParam.ANT1 ||
				l_rxSweepPerParam.RX2!=g_RecordedRxParam.ANT2 ||
				l_rxSweepPerParam.RX3!=g_RecordedRxParam.ANT3 ||
				l_rxSweepPerParam.RX4!=g_RecordedRxParam.ANT4)
		{
			g_dutRxConfigChanged = true;
		}
		else
		{
			g_dutRxConfigChanged = false;
		}

		// Save the current setup
		g_RecordedRxParam.ANT1					= l_rxSweepPerParam.RX1;
		g_RecordedRxParam.ANT2					= l_rxSweepPerParam.RX2;
		g_RecordedRxParam.ANT3					= l_rxSweepPerParam.RX3;
		g_RecordedRxParam.ANT4					= l_rxSweepPerParam.RX4;
		g_RecordedRxParam.CABLE_LOSS_DB[0]		= l_rxSweepPerParam.CABLE_LOSS_DB[0];
		g_RecordedRxParam.CABLE_LOSS_DB[1]		= l_rxSweepPerParam.CABLE_LOSS_DB[1];
		g_RecordedRxParam.CABLE_LOSS_DB[2]		= l_rxSweepPerParam.CABLE_LOSS_DB[2];
		g_RecordedRxParam.CABLE_LOSS_DB[3]		= l_rxSweepPerParam.CABLE_LOSS_DB[3];
		g_RecordedRxParam.FREQ_MHZ				= l_rxSweepPerParam.FREQ_MHZ;

		sprintf_s(g_RecordedRxParam.BANDWIDTH, MAX_BUFFER_SIZE, l_rxSweepPerParam.BANDWIDTH);
		sprintf_s(g_RecordedRxParam.PREAMBLE,  MAX_BUFFER_SIZE, l_rxSweepPerParam.PREAMBLE);
		sprintf_s(g_RecordedRxParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, l_rxSweepPerParam.PACKET_FORMAT_11N);
		sprintf_s(g_RecordedRxParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, l_rxSweepPerParam.GUARD_INTERVAL_11N);


		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] ConfirmRxPerParameters() Confirmed.\n");
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

