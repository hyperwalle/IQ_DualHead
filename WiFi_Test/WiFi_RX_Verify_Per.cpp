#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_Test.h"
#include "WiFi_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

using namespace std;


#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_rxVerifyPerParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_rxVerifyPerReturnMap;

struct tagParam
{
	// Mandatory Parameters
	int    FREQ_MHZ;                                /*! The center frequency (MHz). */
	int    FRAME_COUNT;                             /*! Number of count to send data packet */
	char   BANDWIDTH[MAX_BUFFER_SIZE];              /*! The RF bandwidth to verify RX. */
	char   DATA_RATE[MAX_BUFFER_SIZE];              /*! The data rate to verify RX. */
	char   PREAMBLE[MAX_BUFFER_SIZE];               /*! The preamble type of 11B(only). */
	char   PACKET_FORMAT_11N[MAX_BUFFER_SIZE];      /*! The packet format of 11N(only). */
	char   GUARD_INTERVAL_11N[MAX_BUFFER_SIZE];     /*! The guard interval of 11N(only). */
	double RX_POWER_DBM;                            /*! The output power to verify PER. */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */

	// DUT Parameters
	int    RX1;                                     /*! DUT RX1 on/off. Default=1(on)  */
	int    RX2;                                     /*! DUT RX2 on/off. Default=0(off) */
	int    RX3;                                     /*! DUT RX3 on/off. Default=0(off) */
	int    RX4;                                     /*! DUT RX4 on/off. Default=0(off) */
} l_rxVerifyPerParam;

struct tagReturn
{
	// RX Power Level
	double RX_POWER_LEVEL;							/*!< POWER_LEVEL dBm for the PER test. Format: POWER LEVEL */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */

	// PER Test Result
	int	   GOOD_PACKETS;							/*!< GOOD_PACKETS is the number of good packets that reported from Dut */
	int    TOTAL_PACKETS;							/*!< TOTAL_PACKETS is the total number of packets. */
	double PER;										/*!< PER test result on specific data stream. Format: PER[SpecificStream] */

	// RSSI Test Result
	double RSSI;									/*!< (Average) RSSI test result on specific data stream. Format: RSSI[SpecificStream] */
	double SNR;                                     /*!< (Average) SNR test result on specific data stream. Format: SNR[SpecificStream] */

	char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_rxVerifyPerReturn;
#pragma endregion

void ClearRxPerReturn(void)
{
	l_rxVerifyPerParamMap.clear();
	l_rxVerifyPerReturnMap.clear();
}

#ifndef WIN32
int initRXVerifyPerContainers = InitializeRXVerifyPerContainers();
#endif


// These global variables/functions only for WiFi_TX_Verify_Power.cpp
int ConfirmRxPerParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *cableLossDb, char* errorMsg );
int WiFi_RX_Verify_Per_2010ExtTest(void);

//! WiFi RX Verify PER
/*!
 * Input Parameters
 *
 *  - Mandatory
 *      -# FREQ_MHZ (double): The center frequency (MHz)
 *      -# DATA_RATE (string): The data rate to verify EVM
 *      -# RX_POWER (double): The power (dBm) of DUT is going to receive at the antenna port
 *
 * Return Values
 *      -# A string that contains all DUT info, such as FW version, driver version, chip revision, etc.
 *      -# A string for error message
 *
 * \return 0 No error occurred
 * \return -1 Error(s) occurred.  Please see the returned error message for details
 */


WIFI_TEST_API int WiFi_RX_Verify_Per(void)
{
	int    err = ERR_OK;

	int	   totalPackets = 0;
	int	   goodPackets  = 0;
	int	   badPackets   = 0;
	int    channel = 0, HT40ModeOn = 0;
	int    dummyValue = 0;
	int    wifiMode = 0, wifiStreamNum = 0;
	int    packetNumber = 0;
	double cableLossDb = 0;
	double MaxVSGPowerlimit = 0;
	char   vErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	/*---------------------------------------*
	 * Clear Return Parameters and Container *
	 *---------------------------------------*/
	ClearReturnParameters(l_rxVerifyPerReturnMap);

	/*------------------------*
	 * Respond to QUERY_INPUT *
	 *------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_rxVerifyPerParamMap);
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
		RespondToQueryReturn(l_rxVerifyPerReturnMap);
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
		err = GetInputParameters(l_rxVerifyPerParamMap);
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
		* needed to split WiFi_RX_Verify_Per_2010ExtTest into two to	* 
		* avoid a stack check issue.							*
		* 										*
		*------------------------------------------------------*/
		if ( g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE )
		{
			return WiFi_RX_Verify_Per_2010ExtTest();
		}
		// Error return of this function is irrelevant
		CheckDutTransmitStatus();

#pragma region Prepare input parameters
		err = ConfirmRxPerParameters( &channel, &wifiMode, &wifiStreamNum, &cableLossDb, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters ConfirmRxPerParameters() return OK.\n");
		}
#pragma endregion

#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to Receive  *
		 *---------------------------*/
		// Set DUT RF frquency, Rx power, antenna, data rate
		vDUT_ClearParameters(g_WiFi_Dut);

		if( wifiMode==WIFI_11N_HT40 )
		{
			HT40ModeOn = 1;   // 1: HT40 mode;
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_rxVerifyPerParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_rxVerifyPerParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_rxVerifyPerParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",		l_rxVerifyPerParam.FREQ_MHZ);
		}
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",			HT40ModeOn);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",			l_rxVerifyPerParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",			l_rxVerifyPerParam.BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",			l_rxVerifyPerParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N",	l_rxVerifyPerParam.PACKET_FORMAT_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N",	l_rxVerifyPerParam.GUARD_INTERVAL_11N);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX1",					l_rxVerifyPerParam.RX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX2",					l_rxVerifyPerParam.RX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX3",					l_rxVerifyPerParam.RX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX4",					l_rxVerifyPerParam.RX4);

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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_SET_ANTENNA) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_SET_ANTENNA) return OK.\n");
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_SET_BW) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_SET_BW) return OK.\n");
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_SET_DATA_RATE) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_SET_DATA_RATE) return OK.\n");
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
			MaxVSGPowerlimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11B;
		}
		else if( wifiMode==WIFI_11AG )
		{
			MaxVSGPowerlimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11G;
		}
		else
		{
			MaxVSGPowerlimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11N;
		}

		double VSG_POWER = l_rxVerifyPerParam.RX_POWER_DBM+cableLossDb;
		if ( VSG_POWER>MaxVSGPowerlimit )
		{
			//ERR_VSG_POWER_EXCEED_LIMIT
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", VSG_POWER, MaxVSGPowerlimit);
			throw logMessage;
		}
		else
		{
			// do nothing
		}


		
			// NOT "g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE"
		
			/*---------------------*
			 * Load waveform Files *
			 *---------------------*/
			char  modFile[MAX_BUFFER_SIZE] = "\0";
			err = GetWaveformFileName(  "PER",
					"WAVEFORM_NAME",
					wifiMode,
					l_rxVerifyPerParam.BANDWIDTH,
					l_rxVerifyPerParam.DATA_RATE,
					l_rxVerifyPerParam.PREAMBLE,
					l_rxVerifyPerParam.PACKET_FORMAT_11N,
					l_rxVerifyPerParam.GUARD_INTERVAL_11N,
					modFile,
					MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get waveform file name, GetWaveformFileName(%s) return error.\n", modFile);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] GetWaveformFileName() return OK.\n");
			}

			// Load the whole MOD file for continuous transmit
			err = ::LP_SetVsgModulation( modFile , 0);
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
			err = ::LP_SetVsg(l_rxVerifyPerParam.FREQ_MHZ*1e6, VSG_POWER, g_WiFiGlobalSettingParam.VSG_PORT);
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] VSG send out packet LP_SetFrameCnt(1) failed.\n");
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

			// Delay for DUT settle
			if (0!=g_WiFiGlobalSettingParam.DUT_RX_SETTLE_TIME_MS)
			{
				Sleep(g_WiFiGlobalSettingParam.DUT_RX_SETTLE_TIME_MS);
			}
			else
			{
				// do nothing
			}

			/*--------------------------*
			 * Send packet for PER Test *
			 *--------------------------*/
			if (0==l_rxVerifyPerParam.FRAME_COUNT)
			{
				err = GetPacketNumber(	wifiMode,
						l_rxVerifyPerParam.BANDWIDTH,
						l_rxVerifyPerParam.DATA_RATE,
						l_rxVerifyPerParam.PACKET_FORMAT_11N,
						l_rxVerifyPerParam.GUARD_INTERVAL_11N,
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
				packetNumber = l_rxVerifyPerParam.FRAME_COUNT;
			}

			err = ::LP_SetFrameCnt(packetNumber);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] VSG send out packet LP_SetFrameCnt(%d) return error.\n", packetNumber);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] VSG send out packet LP_SetFrameCnt(%d) return OK.\n", packetNumber);
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

			l_rxVerifyPerReturn.RX_POWER_LEVEL = l_rxVerifyPerParam.RX_POWER_DBM;

			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "GOOD_PACKETS", &goodPackets);
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
				l_rxVerifyPerReturn.GOOD_PACKETS = goodPackets;
			}

			// Using packet sent as the total packets
			totalPackets = packetNumber;
			l_rxVerifyPerReturn.TOTAL_PACKETS = totalPackets;

			// Sometime Dut get packets from Air
			// workaround for some dut which returns more ack than packet sent
			if ( goodPackets>totalPackets )
			{
				goodPackets = totalPackets;
				l_rxVerifyPerReturn.GOOD_PACKETS  = goodPackets;
				l_rxVerifyPerReturn.TOTAL_PACKETS = totalPackets;
			}
			else
			{
				// do nothing
			}

			badPackets = totalPackets - goodPackets;

			if( totalPackets!=0 )
			{
				l_rxVerifyPerReturn.PER = ((double)(badPackets)/(double)(totalPackets)) * 100.0;
			}
			else	// In this case, totalPackets = 0
			{
				l_rxVerifyPerReturn.PER = 100.0;
			}
		

		int    rssiValue_int = (int)NA_NUMBER;
		double rssiValue     =      NA_NUMBER;
		err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "RSSI", &rssiValue_int);		// Get Integer (RSSI) is for backward compatible
		if ( ERR_OK!=err )
		{
			err = ::vDUT_GetDoubleReturn(g_WiFi_Dut, "RSSI", &rssiValue);
			if ( ERR_OK!=err )
			{
				err = ERR_OK;	// TODO: Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] WARNING, This DUT does not support RSSI measurement.\n");
				//throw logMessage;
			}
			else
			{
				l_rxVerifyPerReturn.RSSI = rssiValue;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetDoubleReturn(RSSI) return OK.");
			}
		}
		else
		{
			l_rxVerifyPerReturn.RSSI = (double)rssiValue_int;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(RSSI) return OK.");
		}

		int    snrValue_int = (int)NA_NUMBER;
		double snrValue     =      NA_NUMBER;
		err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "SNR", &snrValue_int);		// Get Integer (SNR) is for backward compatible
		if ( ERR_OK!=err )
		{
			err = ::vDUT_GetDoubleReturn(g_WiFi_Dut, "SNR", &snrValue);
			if ( ERR_OK!=err )
			{
				err = ERR_OK;	// TODO: Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] WARNING, This DUT does not support SNR measurement.\n");
				//throw logMessage;
			}
			else
			{
				l_rxVerifyPerReturn.SNR = snrValue;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetDoubleReturn(SNR) return OK.");
			}
		}
		else
		{
			l_rxVerifyPerReturn.SNR = (double)snrValue_int;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(SNR) return OK.");
		}

#pragma endregion

		if ( (g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE)&&(g_vDutRxActived) )
		{
			// do nothing
		}
		else
		{
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
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err )
		{
			sprintf_s(l_rxVerifyPerReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_rxVerifyPerReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_rxVerifyPerReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		ReturnErrorMessage(l_rxVerifyPerReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
		err = -1;
	}

	// This is a special case, only when some error occur before the RX_STOP.
	// This case will take care by the error handling, but must do RX_STOP manually.
	//if ( g_vDutRxActived )
	//{
	//	vDUT_Run(g_WiFi_Dut, "RX_STOP");
	//}
	//else
	//{
	//	// do nothing
	//}

	return err;
}

int WiFi_RX_Verify_Per_2010ExtTest(void)
{
	int    err = ERR_OK;

	int	   totalPackets = 0;
	int	   goodPackets  = 0;
	int	   badPackets   = 0;
	int    channel = 0, HT40ModeOn = 0;
	int    dummyValue = 0;
	int    wifiMode = 0, wifiStreamNum = 0;
	int    packetNumber = 0;
	double cableLossDb = 0;
	double MaxVSGPowerlimit = 0;
	char   vErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	try
	{
    // Error return of this function is irrelevant
		CheckDutTransmitStatus();

#pragma region Prepare input parameters
		err = ConfirmRxPerParameters( &channel, &wifiMode, &wifiStreamNum, &cableLossDb, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters ConfirmRxPerParameters() return OK.\n");
		}
#pragma endregion

#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to Receive  *
		 *---------------------------*/
		// Set DUT RF frquency, Rx power, antenna, data rate
		vDUT_ClearParameters(g_WiFi_Dut);

		if( wifiMode==WIFI_11N_HT40 )
		{
			HT40ModeOn = 1;   // 1: HT40 mode;
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_rxVerifyPerParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_rxVerifyPerParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_rxVerifyPerParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",		l_rxVerifyPerParam.FREQ_MHZ);
		}
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",			HT40ModeOn);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",			l_rxVerifyPerParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",			l_rxVerifyPerParam.BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",			l_rxVerifyPerParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N",	l_rxVerifyPerParam.PACKET_FORMAT_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N",	l_rxVerifyPerParam.GUARD_INTERVAL_11N);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX1",					l_rxVerifyPerParam.RX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX2",					l_rxVerifyPerParam.RX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX3",					l_rxVerifyPerParam.RX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX4",					l_rxVerifyPerParam.RX4);

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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_SET_ANTENNA) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_SET_ANTENNA) return OK.\n");
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_SET_BW) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_SET_BW) return OK.\n");
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_SET_DATA_RATE) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_SET_DATA_RATE) return OK.\n");
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
			MaxVSGPowerlimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11B;
		}
		else if( wifiMode==WIFI_11AG )
		{
			MaxVSGPowerlimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11G;
		}
		else
		{
			MaxVSGPowerlimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11N;
		}

		double VSG_POWER = l_rxVerifyPerParam.RX_POWER_DBM+cableLossDb;
		if ( VSG_POWER>MaxVSGPowerlimit )
		{
			//ERR_VSG_POWER_EXCEED_LIMIT
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", VSG_POWER, MaxVSGPowerlimit);
			throw logMessage;
		}
		else
		{
			// do nothing
		}
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
					l_rxVerifyPerParam.BANDWIDTH,
					l_rxVerifyPerParam.DATA_RATE,
					l_rxVerifyPerParam.PREAMBLE,
					l_rxVerifyPerParam.PACKET_FORMAT_11N,
					l_rxVerifyPerParam.GUARD_INTERVAL_11N,
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

			if (0==l_rxVerifyPerParam.FRAME_COUNT)
			{
				err = GetPacketNumber(  wifiMode,
						l_rxVerifyPerParam.BANDWIDTH,
						l_rxVerifyPerParam.DATA_RATE,
						l_rxVerifyPerParam.PACKET_FORMAT_11N,
						l_rxVerifyPerParam.GUARD_INTERVAL_11N,
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
				packetNumber = l_rxVerifyPerParam.FRAME_COUNT;
			}

			/*-------------------------------------*
			 * Start IQ2010Ext PER Test (with ACK) *
			 *-------------------------------------*/
			err = ::LP_IQ2010EXT_RxPer( waveformIndex,
					l_rxVerifyPerParam.FREQ_MHZ,
					l_rxVerifyPerParam.RX_POWER_DBM+cableLossDb,
					l_rxVerifyPerParam.RX_POWER_DBM+cableLossDb,
					1,
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
			double ackRespMax = NA_NUMBER, ackRespMin = NA_NUMBER, per = NA_NUMBER;
			int  dataSize = 0;

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
				//LP_IQ2010EXT_GetVectorMeasurement( "powerLeveldBm", numberOfResultSet, 0);
				//LP_IQ2010EXT_GetVectorMeasurement( "ackRespMaxUs",  numberOfResultSet, 0);
				//LP_IQ2010EXT_GetVectorMeasurement( "ackRespMinUs",  numberOfResultSet, 0);

				::LP_IQ2010EXT_GetVectorMeasurement( "perPercent", &per, 1, &dataSize, 0 );
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
			l_rxVerifyPerReturn.RX_POWER_LEVEL = l_rxVerifyPerParam.RX_POWER_DBM;
			totalPackets = packetNumber;
			l_rxVerifyPerReturn.TOTAL_PACKETS = totalPackets;
			l_rxVerifyPerReturn.GOOD_PACKETS = (int)(totalPackets*((100-per)/100));
			l_rxVerifyPerReturn.PER = per;
		int    rssiValue_int = (int)NA_NUMBER;
		double rssiValue     =      NA_NUMBER;
		err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "RSSI", &rssiValue_int);		// Get Integer (RSSI) is for backward compatible
		if ( ERR_OK!=err )
		{
			err = ::vDUT_GetDoubleReturn(g_WiFi_Dut, "RSSI", &rssiValue);
			if ( ERR_OK!=err )
			{
				err = ERR_OK;	// TODO: Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] WARNING, This DUT does not support RSSI measurement.\n");
				//throw logMessage;
			}
			else
			{
				l_rxVerifyPerReturn.RSSI = rssiValue;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetDoubleReturn(RSSI) return OK.");
			}
		}
		else
		{
			l_rxVerifyPerReturn.RSSI = (double)rssiValue_int;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(RSSI) return OK.");
		}

		int    snrValue_int = (int)NA_NUMBER;
		double snrValue     =      NA_NUMBER;
		err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "SNR", &snrValue_int);		// Get Integer (SNR) is for backward compatible
		if ( ERR_OK!=err )
		{
			err = ::vDUT_GetDoubleReturn(g_WiFi_Dut, "SNR", &snrValue);
			if ( ERR_OK!=err )
			{
				err = ERR_OK;	// TODO: Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] WARNING, This DUT does not support SNR measurement.\n");
				//throw logMessage;
			}
			else
			{
				l_rxVerifyPerReturn.SNR = snrValue;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetDoubleReturn(SNR) return OK.");
			}
		}
		else
		{
			l_rxVerifyPerReturn.SNR = (double)snrValue_int;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(SNR) return OK.");
		}

#pragma endregion

		if ( (g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE)&&(g_vDutRxActived) )
		{
			// do nothing
		}
		else
		{
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
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err )
		{
			sprintf_s(l_rxVerifyPerReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_rxVerifyPerReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_rxVerifyPerReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		ReturnErrorMessage(l_rxVerifyPerReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
		err = -1;
	}

	// This is a special case, only when some error occur before the RX_STOP.
	// This case will take care by the error handling, but must do RX_STOP manually.
	//if ( g_vDutRxActived )
	//{
	//	vDUT_Run(g_WiFi_Dut, "RX_STOP");
	//}
	//else
	//{
	//	// do nothing
	//}

	return err;

}

int InitializeRXVerifyPerContainers(void)
{
	/*-----------------*
	 * Input Parameters *
	 *-----------------*/
	l_rxVerifyPerParamMap.clear();

	WIFI_SETTING_STRUCT setting;
	setting.unit = "";
	setting.helpText = "";

	strcpy_s(l_rxVerifyPerParam.BANDWIDTH, MAX_BUFFER_SIZE, "HT20");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.BANDWIDTH))    // Type_Checking
	{
		setting.value = (void*)l_rxVerifyPerParam.BANDWIDTH;
		setting.unit        = "";
		setting.helpText    = "Channel bandwidth. Valid options: QUAR, HALF, HT20 or HT40";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BANDWIDTH", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_rxVerifyPerParam.DATA_RATE, MAX_BUFFER_SIZE, "OFDM-54");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.DATA_RATE))    // Type_Checking
	{
		setting.value = (void*)l_rxVerifyPerParam.DATA_RATE;
		setting.unit        = "";
		setting.helpText    = "Data rate names, such as DSSS-1, CCK-5_5, CCK-11, QUAR1_5, Half-27, OFDM-54, MCS0, MCS15";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_rxVerifyPerParam.PREAMBLE, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.PREAMBLE))    // Type_Checking
	{
		setting.value       = (void*)l_rxVerifyPerParam.PREAMBLE;
		setting.unit        = "";
		setting.helpText    = "The preamble type of 11B(only), can be SHORT or LONG, Default=LONG.";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PREAMBLE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_rxVerifyPerParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, "MIXED");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.PACKET_FORMAT_11N))    // Type_Checking
	{
		setting.value       = (void*)l_rxVerifyPerParam.PACKET_FORMAT_11N;
		setting.unit        = "";
		setting.helpText    = "The packet format of 11N(only), can be MIXED or GREENFIELD, Default=MIXED.";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT_11N", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_rxVerifyPerParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.GUARD_INTERVAL_11N))    // Type_Checking
	{
		setting.value       = (void*)l_rxVerifyPerParam.GUARD_INTERVAL_11N;
		setting.unit        = "";
		setting.helpText    = "The guard interval format of 11N(only), can be LONG or SHORT, Default=LONG.";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL_11N", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.FREQ_MHZ = 2412;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.FREQ_MHZ))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "Channel center frequency in MHz";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_MHZ", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.FRAME_COUNT = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.FRAME_COUNT))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.FRAME_COUNT;
		setting.unit        = "";
		setting.helpText    = "Number of frame count, default=0, means using default global setting value.";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("FRAME_COUNT", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_rxVerifyPerParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_rxVerifyPerParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_rxVerifyPerParam.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_rxVerifyPerParam.RX_POWER_DBM = -65.0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyPerParam.RX_POWER_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.RX_POWER_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected power level at DUT antenna port";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX_POWER_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.RX1 = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.RX1))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.RX1;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 1 on/off. 1:on; 0:off";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX1", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.RX2 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.RX2))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.RX2;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 2 on/off. 1:on; 0:off";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX2", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.RX3 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.RX3))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.RX3;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 3 on/off. 1:on; 0:off";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX3", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.RX4 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.RX4))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.RX4;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 4 on/off. 1:on; 0:off";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX4", setting) );
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
	l_rxVerifyPerReturnMap.clear();

	l_rxVerifyPerReturn.TOTAL_PACKETS = (int)NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerReturn.TOTAL_PACKETS))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.TOTAL_PACKETS;
		setting.unit        = "";
		setting.helpText    = " TOTAL_PACKETS is the number of total packets.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("TOTAL_PACKETS", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerReturn.GOOD_PACKETS = (int)NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerReturn.GOOD_PACKETS))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.GOOD_PACKETS;
		setting.unit        = "";
		setting.helpText    = " GOOD_PACKETS is the number of good packets that reported from Dut.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("GOOD_PACKETS", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerReturn.PER = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyPerReturn.PER))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.PER;
		setting.unit        = "%";
		setting.helpText    = "Average PER over received packets.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("PER", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerReturn.RSSI = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyPerReturn.RSSI))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.RSSI;
		setting.unit        = "";
		setting.helpText    = "Average RSSI over received packets.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("RSSI", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerReturn.SNR = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyPerReturn.SNR))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.SNR;
		setting.unit        = "dB";
		setting.helpText    = "Average SNR for the received frames.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SNR", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerReturn.RX_POWER_LEVEL = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyPerReturn.RX_POWER_LEVEL))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.RX_POWER_LEVEL;
		setting.unit        = "dBm";
		setting.helpText    = "RX power level for PER";
		l_rxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX_POWER_LEVEL", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_rxVerifyPerReturn.CABLE_LOSS_DB[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_rxVerifyPerReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_rxVerifyPerReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_rxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_rxVerifyPerReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_rxVerifyPerReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_rxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
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
int ConfirmRxPerParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *cableLossDb, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		// Checking the Input Parameters
		err = TM_WiFiConvertFrequencyToChannel(l_rxVerifyPerParam.FREQ_MHZ, channel);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_rxVerifyPerParam.FREQ_MHZ);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}
		err = TM_WiFiConvertDataRateNameToIndex(l_rxVerifyPerParam.DATA_RATE, &dummyInt);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_rxVerifyPerParam.DATA_RATE);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertDataRateNameToIndex() return OK.\n");
		}
		if ( 0!=strcmp(l_rxVerifyPerParam.BANDWIDTH, "HT20") && 0!=strcmp(l_rxVerifyPerParam.BANDWIDTH, "HT40")
				&& 0!=strcmp(l_rxVerifyPerParam.BANDWIDTH, "QUAR") && 0!=strcmp(l_rxVerifyPerParam.BANDWIDTH, "HALF") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown BANDWIDTH, WiFi bandwidth %s not supported.\n", l_rxVerifyPerParam.BANDWIDTH);
			throw logMessage;
		}
		if ( 0!=strcmp(l_rxVerifyPerParam.PREAMBLE, "SHORT") && 0!=strcmp(l_rxVerifyPerParam.PREAMBLE, "LONG") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_rxVerifyPerParam.PREAMBLE);
			throw logMessage;
		}
		if ( 0!=strcmp(l_rxVerifyPerParam.PACKET_FORMAT_11N, "MIXED") && 0!=strcmp(l_rxVerifyPerParam.PACKET_FORMAT_11N, "GREENFIELD") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PACKET_FORMAT_11N, WiFi 11n packet format %s not supported.\n", l_rxVerifyPerParam.PACKET_FORMAT_11N);
			throw logMessage;
		}
		if ( 0!=strcmp(l_rxVerifyPerParam.GUARD_INTERVAL_11N, "LONG") && 0!=strcmp(l_rxVerifyPerParam.GUARD_INTERVAL_11N, "SHORT") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown GUARD_INTERVAL_11N, WiFi 11n guard interval format %s not supported.\n", l_rxVerifyPerParam.GUARD_INTERVAL_11N);
			throw logMessage;
		}


		// Convert parameter
		err = WiFiTestMode(l_rxVerifyPerParam.DATA_RATE, l_rxVerifyPerParam.BANDWIDTH, wifiMode, wifiStreamNum);
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
				l_rxVerifyPerParam.FREQ_MHZ,
				l_rxVerifyPerParam.RX1,
				l_rxVerifyPerParam.RX2,
				l_rxVerifyPerParam.RX3,
				l_rxVerifyPerParam.RX4,
				l_rxVerifyPerParam.CABLE_LOSS_DB,
				l_rxVerifyPerReturn.CABLE_LOSS_DB,
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
				0!=strcmp(l_rxVerifyPerParam.BANDWIDTH, g_RecordedRxParam.BANDWIDTH) ||
				0!=strcmp(l_rxVerifyPerParam.PREAMBLE,  g_RecordedRxParam.PREAMBLE) ||
				0!=strcmp(l_rxVerifyPerParam.PACKET_FORMAT_11N, g_RecordedRxParam.PACKET_FORMAT_11N) ||
				0!=strcmp(l_rxVerifyPerParam.GUARD_INTERVAL_11N, g_RecordedRxParam.GUARD_INTERVAL_11N) ||
				l_rxVerifyPerParam.CABLE_LOSS_DB[0]!=g_RecordedRxParam.CABLE_LOSS_DB[0] ||
				l_rxVerifyPerParam.CABLE_LOSS_DB[1]!=g_RecordedRxParam.CABLE_LOSS_DB[1] ||
				l_rxVerifyPerParam.CABLE_LOSS_DB[2]!=g_RecordedRxParam.CABLE_LOSS_DB[2] ||
				l_rxVerifyPerParam.CABLE_LOSS_DB[3]!=g_RecordedRxParam.CABLE_LOSS_DB[3] ||
				l_rxVerifyPerParam.FREQ_MHZ!=g_RecordedRxParam.FREQ_MHZ ||
				l_rxVerifyPerParam.RX1!=g_RecordedRxParam.ANT1 ||
				l_rxVerifyPerParam.RX2!=g_RecordedRxParam.ANT2 ||
				l_rxVerifyPerParam.RX3!=g_RecordedRxParam.ANT3 ||
				l_rxVerifyPerParam.RX4!=g_RecordedRxParam.ANT4)
		{
			g_dutRxConfigChanged = true;
		}
		else
		{
			g_dutRxConfigChanged = false;
		}

		// Save the current setup
		g_RecordedRxParam.ANT1					= l_rxVerifyPerParam.RX1;
		g_RecordedRxParam.ANT2					= l_rxVerifyPerParam.RX2;
		g_RecordedRxParam.ANT3					= l_rxVerifyPerParam.RX3;
		g_RecordedRxParam.ANT4					= l_rxVerifyPerParam.RX4;
		g_RecordedRxParam.CABLE_LOSS_DB[0]		= l_rxVerifyPerParam.CABLE_LOSS_DB[0];
		g_RecordedRxParam.CABLE_LOSS_DB[1]		= l_rxVerifyPerParam.CABLE_LOSS_DB[1];
		g_RecordedRxParam.CABLE_LOSS_DB[2]		= l_rxVerifyPerParam.CABLE_LOSS_DB[2];
		g_RecordedRxParam.CABLE_LOSS_DB[3]		= l_rxVerifyPerParam.CABLE_LOSS_DB[3];
		g_RecordedRxParam.FREQ_MHZ				= l_rxVerifyPerParam.FREQ_MHZ;

		sprintf_s(g_RecordedRxParam.BANDWIDTH, MAX_BUFFER_SIZE, l_rxVerifyPerParam.BANDWIDTH);
		sprintf_s(g_RecordedRxParam.PREAMBLE,  MAX_BUFFER_SIZE, l_rxVerifyPerParam.PREAMBLE);
		sprintf_s(g_RecordedRxParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, l_rxVerifyPerParam.PACKET_FORMAT_11N);
		sprintf_s(g_RecordedRxParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, l_rxVerifyPerParam.GUARD_INTERVAL_11N);



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
